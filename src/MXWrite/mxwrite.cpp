#include "mxwrite.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#ifdef MXWRITE_HAS_CUDA_COPY
#include <cuda_runtime.h>
#endif
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/mastering_display_metadata.h>
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
}

namespace mx {

  namespace {

  // --- HDR helpers -----------------------------------------------------------
  // SMPTE ST.2084 (PQ) constants.
  constexpr float kPqM1 = 2610.0f / 16384.0f;
  constexpr float kPqM2 = (2523.0f / 4096.0f) * 128.0f;
  constexpr float kPqC1 = 3424.0f / 4096.0f;
  constexpr float kPqC2 = (2413.0f / 4096.0f) * 32.0f;
  constexpr float kPqC3 = (2392.0f / 4096.0f) * 32.0f;
  // SDR reference white as a fraction of PQ peak (100 nits / 10000 nits).
  constexpr float kSdrRefFraction = 100.0f / 10000.0f;

  inline float srgbEotf(float v) {
    // sRGB non-linear -> linear light.
    return (v <= 0.04045f) ? (v / 12.92f)
                           : std::pow((v + 0.055f) / 1.055f, 2.4f);
  }

  inline float pqOetf(float L) {
    // L in [0,1] where 1.0 == 10000 nits; returns PQ code value in [0,1].
    const float Lm = std::pow(std::max(0.0f, L), kPqM1);
    const float num = kPqC1 + kPqC2 * Lm;
    const float den = 1.0f + kPqC3 * Lm;
    return std::pow(num / den, kPqM2);
  }

  inline uint16_t clamp10(float v) {
    if (v < 0.0f)
      v = 0.0f;
    if (v > 1023.0f)
      v = 1023.0f;
    return static_cast<uint16_t>(v + 0.5f);
  }

  // Convert one RGBA8 row pair + 2 UV rows into BT.2020 PQ YUV420P10LE.
  // Assumes RGBA input is sRGB-gamma-encoded BT.709 SDR (which is what the
  // shader pipeline produces for HDR inputs after the 8-bit swscale path).
  // Output is limited-range 10-bit. Y: [64..940], UV: [64..960] centered at
  // 512.
  void convertRgbaToBt2020PqYuv420p10(const uint8_t *rgba, int src_stride_bytes,
                                      uint16_t *y_plane, int y_stride_shorts,
                                      uint16_t *u_plane, int u_stride_shorts,
                                      uint16_t *v_plane, int v_stride_shorts,
                                      int width, int height) {
    // BT.2020 non-constant luminance RGB->YUV (limited range).
    // E'Y = 0.2627*R + 0.6780*G + 0.0593*B
    // E'Pb = (B - Y) / 1.8814
    // E'Pr = (R - Y) / 1.4746
    // Limited 10-bit: Y: 0..1 -> 64..940 (range 876), UV: -0.5..0.5 -> 64..960
    // (range 896, center 512).
    constexpr float kKr = 0.2627f;
    constexpr float kKg = 0.6780f;
    constexpr float kKb = 0.0593f;
    constexpr float kPbDiv = 1.0f / 1.8814f;
    constexpr float kPrDiv = 1.0f / 1.4746f;

    for (int y = 0; y < height; y += 2) {
      const int y1 = std::min(y + 1, height - 1);
      const uint8_t *row0 = rgba + y * src_stride_bytes;
      const uint8_t *row1 = rgba + y1 * src_stride_bytes;
      uint16_t *yr0 = y_plane + y * y_stride_shorts;
      uint16_t *yr1 = y_plane + y1 * y_stride_shorts;
      uint16_t *ur = u_plane + (y / 2) * u_stride_shorts;
      uint16_t *vr = v_plane + (y / 2) * v_stride_shorts;

      for (int x = 0; x < width; x += 2) {
        const int x1 = std::min(x + 1, width - 1);

        // Load 2x2 block of sRGB 8-bit pixels.
        auto loadPq = [](const uint8_t *px, float &Y, float &U, float &V) {
          // sRGB 8-bit -> linear [0,1]
          const float r = srgbEotf(px[0] * (1.0f / 255.0f));
          const float g = srgbEotf(px[1] * (1.0f / 255.0f));
          const float b = srgbEotf(px[2] * (1.0f / 255.0f));
          // Scale SDR linear [0,1] (reference 100 nits) to PQ fractional.
          const float rL = r * kSdrRefFraction;
          const float gL = g * kSdrRefFraction;
          const float bL = b * kSdrRefFraction;
          // PQ encode per channel (RGB PQ).
          const float rp = pqOetf(rL);
          const float gp = pqOetf(gL);
          const float bp = pqOetf(bL);
          // BT.2020 RGB' -> YUV'.
          Y = kKr * rp + kKg * gp + kKb * bp;
          U = (bp - Y) * kPbDiv;
          V = (rp - Y) * kPrDiv;
        };

        float Y00, U00, V00;
        float Y01, U01, V01;
        float Y10, U10, V10;
        float Y11, U11, V11;
        loadPq(row0 + x * 4, Y00, U00, V00);
        loadPq(row0 + x1 * 4, Y01, U01, V01);
        loadPq(row1 + x * 4, Y10, U10, V10);
        loadPq(row1 + x1 * 4, Y11, U11, V11);

        // Y: per-pixel, limited-range 10-bit.
        yr0[x] = clamp10(Y00 * 876.0f + 64.0f);
        yr0[x1] = clamp10(Y01 * 876.0f + 64.0f);
        yr1[x] = clamp10(Y10 * 876.0f + 64.0f);
        yr1[x1] = clamp10(Y11 * 876.0f + 64.0f);

        // UV: 4:2:0 average of 2x2 block.
        const float Uavg = 0.25f * (U00 + U01 + U10 + U11);
        const float Vavg = 0.25f * (V00 + V01 + V10 + V11);
        ur[x / 2] = clamp10(Uavg * 896.0f + 512.0f);
        vr[x / 2] = clamp10(Vavg * 896.0f + 512.0f);
      }
    }
  }

  // Convert 16-bit RGBA (already BT.2020-primaries, PQ- or HLG-encoded) into
  // BT.2020 YUV420P10LE limited-range. No transfer conversion is applied here
  // because the GPU HDR encode pass already produced the non-linear signal.
  // @c rgba is tightly-packed 16-bit (8 bytes/pixel), @c src_stride_shorts is
  // the row stride in 16-bit samples (i.e. bytes/2).
  void convertBt2020Rgba16EncodedToYuv420p10(
      const uint16_t *rgba, int src_stride_shorts, uint16_t *y_plane,
      int y_stride_shorts, uint16_t *u_plane, int u_stride_shorts,
      uint16_t *v_plane, int v_stride_shorts, int width, int height) {
    constexpr float kKr = 0.2627f;
    constexpr float kKg = 0.6780f;
    constexpr float kKb = 0.0593f;
    constexpr float kPbDiv = 1.0f / 1.8814f;
    constexpr float kPrDiv = 1.0f / 1.4746f;
    constexpr float kInv65535 = 1.0f / 65535.0f;

    for (int y = 0; y < height; y += 2) {
      const int y1 = std::min(y + 1, height - 1);
      const uint16_t *row0 = rgba + y * src_stride_shorts;
      const uint16_t *row1 = rgba + y1 * src_stride_shorts;
      uint16_t *yr0 = y_plane + y * y_stride_shorts;
      uint16_t *yr1 = y_plane + y1 * y_stride_shorts;
      uint16_t *ur = u_plane + (y / 2) * u_stride_shorts;
      uint16_t *vr = v_plane + (y / 2) * v_stride_shorts;

      for (int x = 0; x < width; x += 2) {
        const int x1 = std::min(x + 1, width - 1);

        auto load = [&](const uint16_t *px, float &Y, float &U, float &V) {
          const float rp = px[0] * kInv65535;
          const float gp = px[1] * kInv65535;
          const float bp = px[2] * kInv65535;
          Y = kKr * rp + kKg * gp + kKb * bp;
          U = (bp - Y) * kPbDiv;
          V = (rp - Y) * kPrDiv;
        };

        float Y00, U00, V00;
        float Y01, U01, V01;
        float Y10, U10, V10;
        float Y11, U11, V11;
        load(row0 + x * 4, Y00, U00, V00);
        load(row0 + x1 * 4, Y01, U01, V01);
        load(row1 + x * 4, Y10, U10, V10);
        load(row1 + x1 * 4, Y11, U11, V11);

        yr0[x] = clamp10(Y00 * 876.0f + 64.0f);
        yr0[x1] = clamp10(Y01 * 876.0f + 64.0f);
        yr1[x] = clamp10(Y10 * 876.0f + 64.0f);
        yr1[x1] = clamp10(Y11 * 876.0f + 64.0f);

        const float Uavg = 0.25f * (U00 + U01 + U10 + U11);
        const float Vavg = 0.25f * (V00 + V01 + V10 + V11);
        ur[x / 2] = clamp10(Uavg * 896.0f + 512.0f);
        vr[x / 2] = clamp10(Vavg * 896.0f + 512.0f);
      }
    }
  }

  } // namespace

  std::mutex transfer_audio_mutex;

  bool is_format_supported(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext)
      return false;
    // Lowercase the extension for case-insensitive comparison.
    std::string lower_ext(ext);
    std::transform(lower_ext.begin(), lower_ext.end(), lower_ext.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    static const char *kSupported[] = {".mp4", ".mkv", ".mov",  ".avi", ".m4v",
                                       ".ts",  ".mts", ".m2ts", ".mpg", ".mpeg",
                                       ".flv", ".f4v", ".3gp",  ".3g2", ".wmv",
                                       ".asf", ".vob"};
    for (const char *s : kSupported) {
      if (lower_ext == s)
        return true;
    }
    return false;
  }

  void cleanup_contexts(AVFormatContext * source_ctx,
                        AVFormatContext * dest_ctx,
                        AVFormatContext * output_ctx) {
    if (source_ctx)
      avformat_close_input(&source_ctx);
    if (dest_ctx)
      avformat_close_input(&dest_ctx);
    if (output_ctx) {
      if (!(output_ctx->oformat->flags & AVFMT_NOFILE))
        avio_closep(&output_ctx->pb);
      avformat_free_context(output_ctx);
    }
  }

  bool transfer_audio(std::string_view sourceAudioFile,
                      std::string_view destVideoFile) {
    std::lock_guard<std::mutex> lock(transfer_audio_mutex);
    if (!is_format_supported(destVideoFile.data())) {
      std::cerr << "Unsupported output format. Supported formats: .mp4, .mkv, "
                   ".avi, .mov\n";
      return false;
    }

    AVFormatContext *source_ctx = nullptr, *dest_ctx = nullptr,
                    *output_ctx = nullptr;
    int source_audio_idx = -1, dest_video_idx = -1, dest_audio_idx = -1;
    std::string temp_output = std::string(destVideoFile) + ".tmp";

    if (avformat_open_input(&source_ctx, sourceAudioFile.data(), nullptr,
                            nullptr) != 0 ||
        avformat_open_input(&dest_ctx, destVideoFile.data(), nullptr,
                            nullptr) != 0) {
      std::cerr << "Failed to open input files\n";
      cleanup_contexts(source_ctx, dest_ctx, output_ctx);
      return false;
    }

    if (avformat_find_stream_info(source_ctx, nullptr) < 0 ||
        avformat_find_stream_info(dest_ctx, nullptr) < 0) {
      std::cerr << "Failed to find stream info\n";
      cleanup_contexts(source_ctx, dest_ctx, output_ctx);
      return false;
    }

    for (unsigned i = 0; i < source_ctx->nb_streams; ++i) {
      if (source_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
        source_audio_idx = i;
        break;
      }
    }

    for (unsigned i = 0; i < dest_ctx->nb_streams; ++i) {
      AVMediaType type = dest_ctx->streams[i]->codecpar->codec_type;
      if (type == AVMEDIA_TYPE_VIDEO)
        dest_video_idx = i;
      else if (type == AVMEDIA_TYPE_AUDIO)
        dest_audio_idx = i;
    }

    if (source_audio_idx == -1 || dest_video_idx == -1) {
      std::cerr << "Required streams not found\n";
      cleanup_contexts(source_ctx, dest_ctx, output_ctx);
      return false;
    }

    const AVOutputFormat *output_fmt =
        av_guess_format(nullptr, destVideoFile.data(), nullptr);
    if (!output_fmt) {
      output_fmt = av_guess_format("mp4", nullptr, nullptr);
      if (!output_fmt) {
        std::cerr << "Failed to determine output format\n";
        cleanup_contexts(source_ctx, dest_ctx, output_ctx);
        return false;
      }
    }

    if (avformat_alloc_output_context2(&output_ctx, output_fmt, nullptr,
                                       temp_output.c_str()) < 0) {
      std::cerr << "Failed to create output context\n";
      cleanup_contexts(source_ctx, dest_ctx, output_ctx);
      return false;
    }

    const AVCodec *audio_codec = avcodec_find_decoder(
        source_ctx->streams[source_audio_idx]->codecpar->codec_id);
    if (!audio_codec) {
      std::cerr << "Failed to find audio decoder\n";
      cleanup_contexts(source_ctx, dest_ctx, output_ctx);
      return false;
    }

    for (unsigned i = 0; i < dest_ctx->nb_streams; ++i) {
      if (dest_ctx->streams[i]->codecpar->codec_type != AVMEDIA_TYPE_VIDEO) {
        continue;
      }

      AVStream *dest_stream = dest_ctx->streams[i];
      AVStream *out_stream = avformat_new_stream(output_ctx, nullptr);
      if (!out_stream) {
        std::cerr << "Failed to create output stream\n";
        cleanup_contexts(source_ctx, dest_ctx, output_ctx);
        return false;
      }

      if (avcodec_parameters_copy(out_stream->codecpar, dest_stream->codecpar) <
          0) {
        std::cerr << "Failed to copy video parameters\n";
        cleanup_contexts(source_ctx, dest_ctx, output_ctx);
        return false;
      }

      out_stream->time_base = dest_stream->time_base;
      out_stream->codecpar->codec_tag = 0;
    }

    AVStream *out_stream = avformat_new_stream(output_ctx, audio_codec);
    if (!out_stream) {
      std::cerr << "Failed to create audio stream\n";
      cleanup_contexts(source_ctx, dest_ctx, output_ctx);
      return false;
    }

    AVCodecParameters *source_params =
        source_ctx->streams[source_audio_idx]->codecpar;
    if (avcodec_parameters_copy(out_stream->codecpar, source_params) < 0) {
      std::cerr << "Failed to copy audio parameters\n";
      cleanup_contexts(source_ctx, dest_ctx, output_ctx);
      return false;
    }

    if (source_params->frame_size == 0) {
      out_stream->codecpar->frame_size = 1024;
    } else {
      out_stream->codecpar->frame_size = source_params->frame_size;
    }

    out_stream->time_base = source_ctx->streams[source_audio_idx]->time_base;
    out_stream->codecpar->codec_tag = 0;
    dest_audio_idx = out_stream->index;

    if (!(output_ctx->oformat->flags & AVFMT_NOFILE)) {
      if (avio_open(&output_ctx->pb, temp_output.c_str(), AVIO_FLAG_WRITE) <
          0) {
        std::cerr << "Failed to open output file\n";
        cleanup_contexts(source_ctx, dest_ctx, output_ctx);
        return false;
      }
    }
    if (avformat_write_header(output_ctx, nullptr) < 0) {
      std::cerr << "Failed to write header\n";
      cleanup_contexts(source_ctx, dest_ctx, output_ctx);
      return false;
    }
    AVPacket packet;
    while (av_read_frame(dest_ctx, &packet) >= 0) {
      if (packet.stream_index == dest_audio_idx) {
        av_packet_unref(&packet);
        continue;
      }

      AVStream *in_stream = dest_ctx->streams[packet.stream_index];
      AVStream *out_stream = output_ctx->streams[packet.stream_index];
      av_packet_rescale_ts(&packet, in_stream->time_base,
                           out_stream->time_base);

      if (av_interleaved_write_frame(output_ctx, &packet) < 0) {
        std::cerr << "Failed to write packet\n";
        av_packet_unref(&packet);
        cleanup_contexts(source_ctx, dest_ctx, output_ctx);
        return false;
      }
      av_packet_unref(&packet);
    }

    int64_t video_duration_ts = 0;
    {
      AVStream *vid_stream = dest_ctx->streams[0];
      if (vid_stream->duration > 0) {
        video_duration_ts =
            av_rescale_q(vid_stream->duration, vid_stream->time_base,
                         source_ctx->streams[source_audio_idx]->time_base);
      } else if (dest_ctx->duration > 0) {
        AVRational av_tb = {1, AV_TIME_BASE};
        video_duration_ts =
            av_rescale_q(dest_ctx->duration, av_tb,
                         source_ctx->streams[source_audio_idx]->time_base);
      }
    }

    av_seek_frame(source_ctx, source_audio_idx, 0, AVSEEK_FLAG_BACKWARD);
    while (av_read_frame(source_ctx, &packet) >= 0) {
      if (packet.stream_index == source_audio_idx) {
        if (video_duration_ts > 0 && packet.pts != AV_NOPTS_VALUE &&
            packet.pts > video_duration_ts) {
          av_packet_unref(&packet);
          break;
        }
        AVStream *in_stream = source_ctx->streams[packet.stream_index];
        AVStream *out_stream = output_ctx->streams[dest_audio_idx];
        av_packet_rescale_ts(&packet, in_stream->time_base,
                             out_stream->time_base);
        packet.stream_index = dest_audio_idx;

        if (av_interleaved_write_frame(output_ctx, &packet) < 0) {
          std::cerr << "Failed to write audio packet\n";
          av_packet_unref(&packet);
          cleanup_contexts(source_ctx, dest_ctx, output_ctx);
          return false;
        }
      }
      av_packet_unref(&packet);
    }
    av_write_trailer(output_ctx);
    cleanup_contexts(source_ctx, dest_ctx, output_ctx);
    if(std::remove(destVideoFile.data()) != 0 ||
       std::rename(temp_output.c_str(), destVideoFile.data()) != 0) {
      std::cerr << "Failed to replace destination with muxed output\n";
      return false;
    }
    return true;
  }

  void Writer::calculateFPSFraction(float fps, int &fps_num, int &fps_den) {
    const float epsilon = 0.001f;
    fps_den = 1001;
    if (std::fabs(fps - 29.97f) < epsilon) {
      fps_num = 30000;
      fps_den = 1001;
    } else if (std::fabs(fps - 59.94f) < epsilon) {
      fps_num = 60000;
      fps_den = 1001;
    } else {
      float precision = 1000.0f;
      fps_num = static_cast<int>(std::round(fps * precision));
      fps_den = static_cast<int>(precision);
      int gcd = std::gcd(fps_num, fps_den);
      fps_num /= gcd;
      fps_den /= gcd;
    }
  }

  namespace {

  // Map an x264-style preset name to an NVENC preset (p1..p7).
  // p1 = fastest/lowest quality, p7 = slowest/highest quality.
  const char *x264_preset_to_nvenc(const std::string &p) {
    if (p == "ultrafast")
      return "p1";
    if (p == "superfast")
      return "p2";
    if (p == "veryfast" || p == "faster")
      return "p3";
    if (p == "fast")
      return "p4";
    if (p == "medium" || p.empty())
      return "p5";
    if (p == "slow" || p == "slower")
      return "p6";
    if (p == "veryslow")
      return "p7";
    // Allow passing NVENC preset names through directly.
    return p.c_str();
  }

  bool is_valid_x264_preset(const std::string &p) {
    static const char *presets[] = {
        "ultrafast", "superfast", "veryfast", "faster",   "fast",
        "medium",    "slow",      "slower",   "veryslow", "placebo"};
    for (const char *n : presets)
      if (p == n)
        return true;
    return false;
  }

  std::string lowercase_ascii(std::string text) {
    std::transform(
        text.begin(), text.end(), text.begin(),
        [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return text;
  }

  struct FfmpegOption {
    std::string name;
    std::string value;
  };

  int option_base_type(AVOptionType type) {
#if LIBAVUTIL_VERSION_MAJOR >= 60
    return static_cast<int>(type) & ~AV_OPT_TYPE_FLAG_ARRAY;
#else
    return static_cast<int>(type);
#endif
  }

  bool looks_like_option(const std::string &token) {
    if (token.size() < 2 || token.front() != '-') {
      return false;
    }
    const unsigned char next = static_cast<unsigned char>(token[1]);
    return !std::isdigit(next) && token[1] != '.';
  }

  bool tokenize_ffmpeg_options(const std::string &text,
                               std::vector<std::string> &tokens,
                               std::string &error) {
    std::string token;
    bool token_started = false;
    bool escaped = false;
    char quote = '\0';

    for (char ch : text) {
      if (escaped) {
        token.push_back(ch);
        token_started = true;
        escaped = false;
        continue;
      }
      if (ch == '\\' && quote != '\'') {
        escaped = true;
        token_started = true;
        continue;
      }
      if (quote != '\0') {
        if (ch == quote) {
          quote = '\0';
        } else {
          token.push_back(ch);
        }
        token_started = true;
        continue;
      }
      if (ch == '\'' || ch == '"') {
        quote = ch;
        token_started = true;
      } else if (std::isspace(static_cast<unsigned char>(ch))) {
        if (token_started) {
          tokens.push_back(token);
          token.clear();
          token_started = false;
        }
      } else {
        token.push_back(ch);
        token_started = true;
      }
    }

    if (escaped) {
      error = "trailing escape character";
      return false;
    }
    if (quote != '\0') {
      error = "unterminated quote";
      return false;
    }
    if (token_started) {
      tokens.push_back(token);
    }
    return true;
  }

  std::string normalize_ffmpeg_option_name(std::string name) {
    while (!name.empty() && name.front() == '-') {
      name.erase(name.begin());
    }

    const size_t stream_specifier = name.find(':');
    if (stream_specifier != std::string::npos) {
      const std::string suffix = name.substr(stream_specifier + 1);
      if (suffix == "v" || suffix.starts_with("v:")) {
        name.erase(stream_specifier);
      }
    }
    return name;
  }

  bool is_codec_name(const std::string &value) {
    const std::string codec = lowercase_ascii(value);
    return codec == "h265_nvenc" || codec == "h264" || codec == "h265" ||
           codec == "hevc" || codec == "nvenc" || codec == "software" ||
           codec == "auto" ||
           avcodec_find_encoder_by_name(codec.c_str()) != nullptr;
  }

  bool parse_ffmpeg_options(const std::string &text,
                            std::vector<FfmpegOption> &options) {
    std::vector<std::string> tokens;
    std::string error;
    if (!tokenize_ffmpeg_options(text, tokens, error)) {
      std::cerr << "MXWrite: invalid extra FFmpeg parameters: " << error
                << ".\n";
      return false;
    }

    for (size_t index = 0; index < tokens.size(); ++index) {
      std::string token = tokens[index];
      if (!looks_like_option(token)) {
        if (index == 0 && is_codec_name(token)) {
          options.push_back({"codec", token});
        } else {
          std::cerr << "MXWrite: ignoring non-option extra parameter '" << token
                    << "' (filenames are selected by Writer::open).\n";
        }
        continue;
      }

      while (!token.empty() && token.front() == '-') {
        token.erase(token.begin());
      }
      const size_t equals = token.find('=');
      std::string name = normalize_ffmpeg_option_name(token.substr(0, equals));
      std::string value;
      if (equals != std::string::npos) {
        value = token.substr(equals + 1);
      } else if (index + 1 < tokens.size() &&
                 !looks_like_option(tokens[index + 1])) {
        value = tokens[++index];
      } else {
        value = "1";
      }

      if (name.empty()) {
        std::cerr << "MXWrite: invalid empty FFmpeg option name.\n";
        return false;
      }
      options.push_back({std::move(name), std::move(value)});
    }
    return true;
  }

  const std::string *
  find_ffmpeg_option(const std::vector<FfmpegOption> &options,
                     const std::string &name) {
    for (auto option = options.rbegin(); option != options.rend(); ++option) {
      if (option->name == name) {
        return &option->value;
      }
    }
    return nullptr;
  }

  bool is_reserved_ffmpeg_option(const std::string &name) {
    return name == "c" || name == "codec" || name == "vcodec" ||
           name == "pix_fmt" || name == "pixel_format";
  }

  bool apply_ffmpeg_options(const std::vector<FfmpegOption> &options,
                            AVCodecContext *context,
                            AVFormatContext *output_context) {
    for (const FfmpegOption &option : options) {
      if (is_reserved_ffmpeg_option(option.name)) {
        continue;
      }

      int result = AVERROR_OPTION_NOT_FOUND;
      if (context->priv_data) {
        result = av_opt_set(context->priv_data, option.name.c_str(),
                            option.value.c_str(), 0);
      }
      if (result == AVERROR_OPTION_NOT_FOUND) {
        result =
            av_opt_set(context, option.name.c_str(), option.value.c_str(), 0);
      }
      if (result == AVERROR_OPTION_NOT_FOUND && output_context->priv_data) {
        result = av_opt_set(output_context->priv_data, option.name.c_str(),
                            option.value.c_str(), 0);
      }
      if (result < 0) {
        char error_text[AV_ERROR_MAX_STRING_SIZE] = {};
        av_strerror(result, error_text, sizeof(error_text));
        std::cerr << "MXWrite: FFmpeg option '-" << option.name << " "
                  << option.value << "' was rejected: " << error_text << ".\n";
        return false;
      }
    }
    return true;
  }

  bool set_named_encoder_option(void *object, const char *name,
                                const std::string &value) {
    if (!object || value.empty()) {
      return false;
    }
    const AVOption *option = av_opt_find(object, name, nullptr, 0, 0);
    if (!option) {
      return false;
    }
    const int base_type = option_base_type(option->type);
    const bool textual = base_type == AV_OPT_TYPE_STRING ||
                         base_type == AV_OPT_TYPE_DICT ||
                         base_type == AV_OPT_TYPE_BINARY;
    const bool numeric_text =
        std::isdigit(static_cast<unsigned char>(value.front())) ||
        value.front() == '-' || value.front() == '+' || value.front() == '.';
    if (!textual && !numeric_text) {
      const AVOption *named_value =
          av_opt_find(object, value.c_str(), option->unit, 0, 0);
      if (!named_value || named_value->type != AV_OPT_TYPE_CONST) {
        return false;
      }
    }
    return av_opt_set(object, name, value.c_str(), 0) >= 0;
  }

  bool is_nvenc_tune(const std::string &tune) {
    return tune == "hq" || tune == "uhq" || tune == "ll" || tune == "ull" ||
           tune == "lossless";
  }

  std::string nvenc_preset_to_software(const std::string &preset) {
    if (preset == "p1")
      return "ultrafast";
    if (preset == "p2")
      return "superfast";
    if (preset == "p3")
      return "veryfast";
    if (preset == "p4")
      return "fast";
    if (preset == "p5")
      return "medium";
    if (preset == "p6")
      return "slow";
    if (preset == "p7")
      return "veryslow";
    return preset;
  }

  std::vector<FfmpegOption>
  software_fallback_options(const std::vector<FfmpegOption> &options,
                            bool use_hevc_codec) {
    std::vector<FfmpegOption> translated;
    bool enable_lossless = false;

    for (const FfmpegOption &option : options) {
      FfmpegOption value = option;
      if (value.name == "preset") {
        value.value = nvenc_preset_to_software(lowercase_ascii(value.value));
      } else if (value.name == "tune") {
        const std::string tune = lowercase_ascii(value.value);
        if (tune == "lossless") {
          enable_lossless = true;
          continue;
        }
        if (tune == "hq" || tune == "uhq") {
          continue;
        }
        if (tune == "ll" || tune == "ull") {
          value.value = "zerolatency";
        }
      } else if (value.name == "profile" &&
                 lowercase_ascii(value.value) == "rext") {
        value.value = use_hevc_codec ? "main444-8" : "high444";
      } else if (value.name == "rgb_mode" || value.name == "rc") {
        continue;
      } else if (value.name == "cq") {
        value.name = "crf";
      }
      translated.push_back(std::move(value));
    }

    if (enable_lossless) {
      if (use_hevc_codec) {
        bool appended = false;
        for (FfmpegOption &option : translated) {
          if (option.name == "x265-params") {
            if (!option.value.empty())
              option.value += ':';
            option.value += "lossless=1";
            appended = true;
          }
        }
        if (!appended) {
          translated.push_back({"x265-params", "lossless=1"});
        }
      } else {
        translated.push_back({"qp", "0"});
      }
    }
    return translated;
  }

  std::vector<AVPixelFormat> supported_pixel_formats(const AVCodec *codec) {
    std::vector<AVPixelFormat> formats;
    if (!codec) {
      return formats;
    }

#if LIBAVCODEC_VERSION_MAJOR >= 61
    const void *configurations = nullptr;
    int configuration_count = 0;
    if (avcodec_get_supported_config(nullptr, codec, AV_CODEC_CONFIG_PIX_FORMAT,
                                     0, &configurations,
                                     &configuration_count) >= 0 &&
        configurations) {
      const auto *pixel_formats =
          static_cast<const AVPixelFormat *>(configurations);
      formats.assign(pixel_formats, pixel_formats + configuration_count);
    }
#else
    if (codec->pix_fmts) {
      for (const AVPixelFormat *format = codec->pix_fmts;
           *format != AV_PIX_FMT_NONE; ++format) {
        formats.push_back(*format);
      }
    }
#endif
    return formats;
  }

  bool is_hardware_pixel_format(AVPixelFormat format) {
    const AVPixFmtDescriptor *descriptor = av_pix_fmt_desc_get(format);
    return descriptor && (descriptor->flags & AV_PIX_FMT_FLAG_HWACCEL) != 0;
  }

  AVPixelFormat choose_software_pixel_format(const AVCodec *codec,
                                             AVPixelFormat requested_format) {
    const std::vector<AVPixelFormat> formats = supported_pixel_formats(codec);
    auto is_usable = [](AVPixelFormat format) {
      return !is_hardware_pixel_format(format) && sws_isSupportedOutput(format);
    };

    if (requested_format != AV_PIX_FMT_NONE) {
      if (!is_usable(requested_format)) {
        return AV_PIX_FMT_NONE;
      }
      if (formats.empty() || std::find(formats.begin(), formats.end(),
                                       requested_format) != formats.end()) {
        return requested_format;
      }
      return AV_PIX_FMT_NONE;
    }

    static constexpr AVPixelFormat preferred_formats[] = {
        AV_PIX_FMT_YUV420P,     AV_PIX_FMT_NV12,        AV_PIX_FMT_YUV422P,
        AV_PIX_FMT_YUV444P,     AV_PIX_FMT_YUV420P10LE, AV_PIX_FMT_YUV422P10LE,
        AV_PIX_FMT_YUV444P10LE, AV_PIX_FMT_BGRA,        AV_PIX_FMT_RGBA};
    if (formats.empty()) {
      return AV_PIX_FMT_YUV420P;
    }
    for (AVPixelFormat preferred : preferred_formats) {
      if (std::find(formats.begin(), formats.end(), preferred) !=
              formats.end() &&
          is_usable(preferred)) {
        return preferred;
      }
    }
    for (AVPixelFormat format : formats) {
      if (is_usable(format)) {
        return format;
      }
    }
    return AV_PIX_FMT_NONE;
  }

  bool codec_uses_hardware(const AVCodec *codec) {
    if (!codec) {
      return false;
    }
    if ((codec->capabilities & (AV_CODEC_CAP_HARDWARE | AV_CODEC_CAP_HYBRID)) !=
        0) {
      return true;
    }
    for (int index = 0; avcodec_get_hw_config(codec, index); ++index) {
      return true;
    }
    const std::string name =
        codec->name ? lowercase_ascii(codec->name) : std::string{};
    static constexpr std::string_view hardware_markers[] = {
        "_nvenc",   "_qsv",          "_vaapi",           "_amf",
        "_v4l2m2m", "_videotoolbox", "_mediafoundation", "_vulkan"};
    return std::any_of(std::begin(hardware_markers), std::end(hardware_markers),
                       [&name](std::string_view marker) {
                         return name.find(marker) != std::string::npos;
                       });
  }

  std::string option_type_name(AVOptionType type) {
    const int base_type = option_base_type(type);
    switch (base_type) {
    case AV_OPT_TYPE_FLAGS:
      return "flags";
    case AV_OPT_TYPE_INT:
      return "integer";
    case AV_OPT_TYPE_INT64:
      return "integer64";
    case AV_OPT_TYPE_DOUBLE:
      return "double";
    case AV_OPT_TYPE_FLOAT:
      return "float";
    case AV_OPT_TYPE_STRING:
      return "string";
    case AV_OPT_TYPE_RATIONAL:
      return "rational";
    case AV_OPT_TYPE_BINARY:
      return "binary";
    case AV_OPT_TYPE_DICT:
      return "dictionary";
    case AV_OPT_TYPE_UINT64:
      return "unsigned64";
    case AV_OPT_TYPE_IMAGE_SIZE:
      return "size";
    case AV_OPT_TYPE_PIXEL_FMT:
      return "pixel-format";
    case AV_OPT_TYPE_SAMPLE_FMT:
      return "sample-format";
    case AV_OPT_TYPE_VIDEO_RATE:
      return "frame-rate";
    case AV_OPT_TYPE_DURATION:
      return "duration";
    case AV_OPT_TYPE_COLOR:
      return "color";
    case AV_OPT_TYPE_BOOL:
      return "boolean";
#if LIBAVUTIL_VERSION_MAJOR >= 57
    case AV_OPT_TYPE_CHLAYOUT:
#else
    case AV_OPT_TYPE_CHANNEL_LAYOUT:
#endif
      return "channel-layout";
#if LIBAVUTIL_VERSION_MAJOR >= 60
    case AV_OPT_TYPE_UINT:
      return "unsigned";
#endif
    default:
      return "value";
    }
  }

  bool option_has_numeric_range(AVOptionType type) {
    const int base_type = option_base_type(type);
    return base_type == AV_OPT_TYPE_FLAGS || base_type == AV_OPT_TYPE_INT ||
           base_type == AV_OPT_TYPE_INT64 || base_type == AV_OPT_TYPE_DOUBLE ||
           base_type == AV_OPT_TYPE_FLOAT || base_type == AV_OPT_TYPE_UINT64 ||
           base_type == AV_OPT_TYPE_BOOL
#if LIBAVUTIL_VERSION_MAJOR >= 60
           || base_type == AV_OPT_TYPE_UINT
#endif
        ;
  }

  std::string number_text(double value) {
    std::ostringstream stream;
    stream << std::setprecision(12) << value;
    return stream.str();
  }

  void append_encoder_options(void *object,
                              std::vector<EncoderOptionInfo> &result,
                              std::set<std::string> &seen) {
    if (!object) {
      return;
    }
    for (const AVOption *option = nullptr;
         (option = av_opt_next(object, option));) {
      if (option->type == AV_OPT_TYPE_CONST ||
          (option->flags & AV_OPT_FLAG_ENCODING_PARAM) == 0 ||
          (option->flags & AV_OPT_FLAG_VIDEO_PARAM) == 0 ||
          (option->flags & (AV_OPT_FLAG_READONLY | AV_OPT_FLAG_DEPRECATED)) !=
              0 ||
          !option->name || !seen.insert(option->name).second) {
        continue;
      }

      EncoderOptionInfo info;
      info.name = option->name;
      info.type = option_type_name(option->type);
      info.help = option->help ? option->help : "";
      if (option_has_numeric_range(option->type)) {
        info.minimum = number_text(option->min);
        info.maximum = number_text(option->max);
      }

      uint8_t *value = nullptr;
      if (av_opt_get(object, option->name, 0, &value) >= 0 && value) {
        info.default_value = reinterpret_cast<char *>(value);
      }
      av_free(value);

      if (option->unit) {
        std::vector<std::string> choices;
        for (const AVOption *choice = nullptr;
             (choice = av_opt_next(object, choice));) {
          if (choice->type == AV_OPT_TYPE_CONST && choice->unit &&
              choice->name && std::string_view(choice->unit) == option->unit) {
            choices.emplace_back(choice->name);
          }
        }
        for (size_t index = 0; index < choices.size(); ++index) {
          if (index > 0) {
            info.choices += ", ";
          }
          info.choices += choices[index];
        }
      }
      result.push_back(std::move(info));
    }
  }

  } // namespace

  std::vector<EncoderInfo> available_video_encoders() {
    std::vector<EncoderInfo> result;
    void *iterator = nullptr;
    while (const AVCodec *codec = av_codec_iterate(&iterator)) {
      if (!av_codec_is_encoder(codec) || codec->type != AVMEDIA_TYPE_VIDEO ||
          !codec->name) {
        continue;
      }
      EncoderInfo info;
      info.name = codec->name;
      info.long_name = codec->long_name ? codec->long_name : codec->name;
      info.codec_name = avcodec_get_name(codec->id);
      info.hardware = codec_uses_hardware(codec);
      info.experimental =
          (codec->capabilities & AV_CODEC_CAP_EXPERIMENTAL) != 0;
      const std::vector<AVPixelFormat> formats = supported_pixel_formats(codec);
      for (AVPixelFormat format : formats) {
        const char *name = av_get_pix_fmt_name(format);
        if (!name) {
          continue;
        }
        if (!info.pixel_formats.empty()) {
          info.pixel_formats += ", ";
        }
        info.pixel_formats += name;
      }
      result.push_back(std::move(info));
    }
    std::sort(result.begin(), result.end(),
              [](const EncoderInfo &left, const EncoderInfo &right) {
                if (left.codec_name != right.codec_name) {
                  return left.codec_name < right.codec_name;
                }
                if (left.hardware != right.hardware) {
                  return left.hardware > right.hardware;
                }
                return left.name < right.name;
              });
    return result;
  }

  std::vector<EncoderOptionInfo> video_encoder_options(
      std::string_view encoder_name) {
    const std::string name(encoder_name);
    const AVCodec *codec = avcodec_find_encoder_by_name(name.c_str());
    if (!codec || codec->type != AVMEDIA_TYPE_VIDEO) {
      return {};
    }
    AVCodecContext *context = avcodec_alloc_context3(codec);
    if (!context) {
      return {};
    }
    std::vector<EncoderOptionInfo> result;
    std::set<std::string> seen;
    append_encoder_options(context->priv_data, result, seen);
    append_encoder_options(context, result, seen);
    avcodec_free_context(&context);
    std::sort(
        result.begin(), result.end(),
        [](const EncoderOptionInfo &left, const EncoderOptionInfo &right) {
          return left.name < right.name;
        });
    return result;
  }

  bool Writer::open(const std::string &filename, int w, int h, float fps,
                    const char *crf) {
    std::lock_guard<std::mutex> lock(writer_mutex);
    EncodeOptions opts;
    if (crf && *crf) {
      try {
        opts.crf = std::stoi(crf);
      } catch (...) {
      }
    }
    // Preserve legacy low-latency behaviour for old callers.
    opts.preset = "ultrafast";
    opts.tune = "zerolatency";
    opts.realtime = true;
    return openInternal(filename, w, h, fps, opts, false);
  }

  bool Writer::open(const std::string &filename, int w, int h, float fps,
                    const EncodeOptions &opts) {
    std::lock_guard<std::mutex> lock(writer_mutex);
    return openInternal(filename, w, h, fps, opts, false);
  }

  bool Writer::open_ts(const std::string &filename, int w, int h, float fps,
                       const char *crf) {
    std::lock_guard<std::mutex> lock(writer_mutex);
    EncodeOptions opts;
    if (crf && *crf) {
      try {
        opts.crf = std::stoi(crf);
      } catch (...) {
      }
    }
    opts.preset = "ultrafast";
    opts.tune = "zerolatency";
    opts.realtime = true;
    return openInternal(filename, w, h, fps, opts, true);
  }

  bool Writer::open_ts(const std::string &filename, int w, int h, float fps,
                       const EncodeOptions &opts) {
    std::lock_guard<std::mutex> lock(writer_mutex);
    return openInternal(filename, w, h, fps, opts, true);
  }

  bool Writer::initHardwareEncoding(const AVCodec *codec,
                                    AVPixelFormat requested_format,
                                    bool prefer_cuda_rgba) {
    const AVCodecHWConfig *hardware_config = nullptr;
    for (int index = 0;
         const AVCodecHWConfig *config = avcodec_get_hw_config(codec, index);
         ++index) {
      if ((config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_FRAMES_CTX) == 0 ||
          config->device_type == AV_HWDEVICE_TYPE_NONE ||
          config->pix_fmt == AV_PIX_FMT_NONE) {
        continue;
      }
      if (!hardware_config ||
          (prefer_cuda_rgba && config->device_type == AV_HWDEVICE_TYPE_CUDA)) {
        hardware_config = config;
      }
      if (prefer_cuda_rgba && config->device_type == AV_HWDEVICE_TYPE_CUDA) {
        break;
      }
    }
    if (!hardware_config ||
        av_hwdevice_ctx_create(&hw_device_ctx, hardware_config->device_type,
                               nullptr, nullptr, 0) < 0) {
      return false;
    }

    codec_ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx);
    codec_ctx->pix_fmt = hardware_config->pix_fmt;

    hw_frames_ctx = av_hwframe_ctx_alloc(hw_device_ctx);
    if (!hw_frames_ctx) {
      return false;
    }

    auto *frames_ctx =
        reinterpret_cast<AVHWFramesContext *>(hw_frames_ctx->data);
    frames_ctx->format = hardware_config->pix_fmt;

    AVHWFramesConstraints *constraints =
        av_hwdevice_get_hwframe_constraints(hw_device_ctx, nullptr);
    auto valid_software_format = [constraints](AVPixelFormat format) {
      if (format == AV_PIX_FMT_NONE || is_hardware_pixel_format(format) ||
          !sws_isSupportedOutput(format)) {
        return false;
      }
      if (!constraints || !constraints->valid_sw_formats) {
        return true;
      }
      for (const AVPixelFormat *valid = constraints->valid_sw_formats;
           *valid != AV_PIX_FMT_NONE; ++valid) {
        if (*valid == format) {
          return true;
        }
      }
      return false;
    };

    AVPixelFormat upload_format = AV_PIX_FMT_NONE;
    if (prefer_cuda_rgba && valid_software_format(AV_PIX_FMT_RGBA)) {
      upload_format = AV_PIX_FMT_RGBA;
    } else if (valid_software_format(requested_format)) {
      upload_format = requested_format;
    } else {
      static constexpr AVPixelFormat preferred_upload_formats[] = {
          AV_PIX_FMT_NV12,        AV_PIX_FMT_YUV420P, AV_PIX_FMT_P010LE,
          AV_PIX_FMT_YUV420P10LE, AV_PIX_FMT_YUV422P, AV_PIX_FMT_BGRA,
          AV_PIX_FMT_RGBA};
      for (AVPixelFormat format : preferred_upload_formats) {
        if (valid_software_format(format)) {
          upload_format = format;
          break;
        }
      }
      if (upload_format == AV_PIX_FMT_NONE && constraints &&
          constraints->valid_sw_formats) {
        for (const AVPixelFormat *valid = constraints->valid_sw_formats;
             *valid != AV_PIX_FMT_NONE; ++valid) {
          if (valid_software_format(*valid)) {
            upload_format = *valid;
            break;
          }
        }
      }
    }
    av_hwframe_constraints_free(&constraints);
    if (upload_format == AV_PIX_FMT_NONE) {
      return false;
    }

    frames_ctx->sw_format = upload_format;
    frames_ctx->width = width;
    frames_ctx->height = height;
    // Pool must comfortably exceed MAX_QUEUE_SIZE so av_hwframe_get_buffer()
    // on the producer thread never becomes the throttle.  A few extra slots
    // cover frames currently in-flight inside the encoder.
    frames_ctx->initial_pool_size = static_cast<int>(MAX_QUEUE_SIZE) + 8;

    if (av_hwframe_ctx_init(hw_frames_ctx) < 0) {
      return false;
    }

    codec_ctx->hw_frames_ctx = av_buffer_ref(hw_frames_ctx);
    codec_ctx->sw_pix_fmt = upload_format;

    upload_sw_frame = av_frame_alloc();
    if (!upload_sw_frame) {
      return false;
    }
    upload_sw_frame->format = upload_format;
    upload_sw_frame->width = width;
    upload_sw_frame->height = height;
    if (av_frame_get_buffer(upload_sw_frame, 32) < 0) {
      return false;
    }

    if (upload_format != AV_PIX_FMT_RGBA) {
      sws_ctx = sws_getContext(width, height, AV_PIX_FMT_RGBA, width, height,
                               upload_format, SWS_FAST_BILINEAR, nullptr,
                               nullptr, nullptr);
      if (!sws_ctx) {
        return false;
      }
    }

    direct_cuda_upload =
        hardware_config->device_type == AV_HWDEVICE_TYPE_CUDA &&
        upload_format == AV_PIX_FMT_RGBA;

#ifdef MXWRITE_HAS_CUDA_COPY
    // Non-blocking stream so device-to-device uploads from write_cuda_rgba()
    // do not serialise against the renderer's CUDA work on the default stream.
    if (direct_cuda_upload && !cuda_upload_stream) {
      if (cudaStreamCreateWithFlags(&cuda_upload_stream,
                                    cudaStreamNonBlocking) != cudaSuccess) {
        cuda_upload_stream = nullptr;
      }
    }
#endif

    return true;
  }

  bool Writer::openInternal(const std::string &filename, int w, int h,
                            float fps, const EncodeOptions &opts,
                            bool ts_mode) {
    avformat_network_init();
    av_log_set_level(AV_LOG_ERROR);
    opened = false;
    active_encoder_hardware = false;
    stop_requested = false;
    frame_count = 0;
    last_duration = 0.0;
    block_when_full.store(opts.block_when_full, std::memory_order_relaxed);

    while (!encode_queue.empty()) {
      releaseFrame(encode_queue.front());
      encode_queue.pop();
    }

    std::vector<FfmpegOption> extra_options;
    if (!opts.ffmpeg_options.empty() &&
        !parse_ffmpeg_options(opts.ffmpeg_options, extra_options)) {
      return false;
    }

    const std::string *codec_override = find_ffmpeg_option(extra_options, "c");
    if (!codec_override) {
      codec_override = find_ffmpeg_option(extra_options, "codec");
    }
    if (!codec_override) {
      codec_override = find_ffmpeg_option(extra_options, "vcodec");
    }

    const std::string *pixel_format_option =
        find_ffmpeg_option(extra_options, "pix_fmt");
    if (!pixel_format_option) {
      pixel_format_option = find_ffmpeg_option(extra_options, "pixel_format");
    }
    AVPixelFormat requested_pixel_format = AV_PIX_FMT_NONE;
    if (pixel_format_option) {
      requested_pixel_format = av_get_pix_fmt(pixel_format_option->c_str());
      if (requested_pixel_format == AV_PIX_FMT_NONE) {
        std::cerr << "MXWrite: unknown pixel format '" << *pixel_format_option
                  << "'.\n";
        return false;
      }
    }

    // Pass nullptr for format_name so libavformat picks the container based
    // on the filename extension (mp4, mkv, mov, avi...).
    if (avformat_alloc_output_context2(&format_ctx, nullptr, nullptr,
                                       filename.c_str()) < 0) {
      std::cerr << "Could not allocate output context.\n";
      return false;
    }

    width = w;
    height = h;
    hdr_output = opts.hdr.enabled;
    hdr_info = opts.hdr;

    // ---- HDR (HEVC Main10 + BT.2020/PQ) path ------------------------------
    // Short-circuits the normal SDR codec selection when opts.hdr.enabled is
    // true. Forces software libx265 + YUV420P10LE + PQ metadata, writes the
    // color tags and mastering/content-light side data, and bypasses NVENC.
    if (hdr_output) {
      const AVCodec *hdr_codec = avcodec_find_encoder_by_name("libx265");
      if (!hdr_codec) {
        std::cerr << "MXWrite: HDR output requested but libx265 encoder not "
                     "available.\n";
        avformat_free_context(format_ctx);
        format_ctx = nullptr;
        return false;
      }

      stream = avformat_new_stream(format_ctx, hdr_codec);
      if (!stream) {
        std::cerr << "MXWrite: could not create HDR stream.\n";
        avformat_free_context(format_ctx);
        format_ctx = nullptr;
        return false;
      }

      calculateFPSFraction(fps, fps_num, fps_den);
      AVRational tb_hdr = {fps_den, fps_num};
      stream->time_base = tb_hdr;

      codec_ctx = avcodec_alloc_context3(hdr_codec);
      if (!codec_ctx) {
        std::cerr << "MXWrite: could not allocate HDR codec context.\n";
        avformat_free_context(format_ctx);
        format_ctx = nullptr;
        return false;
      }

      codec_ctx->width = width;
      codec_ctx->height = height;
      codec_ctx->time_base = stream->time_base;
      codec_ctx->framerate = AVRational{fps_num, fps_den};
      codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P10LE;
      codec_ctx->profile = AV_PROFILE_HEVC_MAIN_10;
      codec_ctx->bits_per_raw_sample = 10;
      codec_ctx->gop_size = 30;
      codec_ctx->max_b_frames = 0;
      codec_ctx->thread_count =
          std::max(1u, std::thread::hardware_concurrency());
      codec_ctx->thread_type = FF_THREAD_SLICE;
      codec_ctx->delay = 0;

      // Tag the stream with BT.2020 + PQ (or whatever the input used).
      codec_ctx->color_primaries = static_cast<AVColorPrimaries>(
          hdr_info.color_primaries ? hdr_info.color_primaries
                                   : AVCOL_PRI_BT2020);
      codec_ctx->color_trc = static_cast<AVColorTransferCharacteristic>(
          hdr_info.color_trc ? hdr_info.color_trc : AVCOL_TRC_SMPTE2084);
      codec_ctx->colorspace = static_cast<AVColorSpace>(
          hdr_info.color_space ? hdr_info.color_space : AVCOL_SPC_BT2020_NCL);
      codec_ctx->color_range = static_cast<AVColorRange>(
          hdr_info.color_range ? hdr_info.color_range : AVCOL_RANGE_MPEG);
      codec_ctx->chroma_sample_location = AVCHROMA_LOC_LEFT;

      // Encoder options: Main10, matching x265-params for color volume.
      std::string preset_hdr =
          opts.preset.empty() ? std::string("medium") : opts.preset;
      av_opt_set(codec_ctx->priv_data, "preset", preset_hdr.c_str(), 0);
      int crf_val_hdr = opts.crf;
      if (crf_val_hdr < 0)
        crf_val_hdr = 0;
      if (crf_val_hdr > 51)
        crf_val_hdr = 51;
      const std::string crf_hdr = std::to_string(crf_val_hdr);
      av_opt_set(codec_ctx->priv_data, "crf", crf_hdr.c_str(), 0);

      // x265 params: colorprim, transfer, colormatrix, range, hdr flag.
      // These drive the stream VUI + SEI so players recognise the file as HDR.
      std::string x265_params =
          "profile=main10:colorprim=bt2020:transfer=smpte2084:colormatrix="
          "bt2020nc:range=limited:repeat-headers=1";
      // When HLG transfer is requested, swap transfer + mark hlg.
      if (codec_ctx->color_trc == AVCOL_TRC_ARIB_STD_B67) {
        x265_params = "profile=main10:colorprim=bt2020:transfer=arib-std-b67:"
                      "colormatrix=bt2020nc:range=limited:repeat-headers=1";
      }
      av_opt_set(codec_ctx->priv_data, "x265-params", x265_params.c_str(), 0);

      if (pixel_format_option &&
          requested_pixel_format != AV_PIX_FMT_YUV420P10LE) {
        std::cerr << "MXWrite: HDR output forces yuv420p10le; ignoring "
                     "requested pixel format '"
                  << *pixel_format_option << "'.\n";
      }
      if (!apply_ffmpeg_options(extra_options, codec_ctx, format_ctx)) {
        avcodec_free_context(&codec_ctx);
        avformat_free_context(format_ctx);
        format_ctx = nullptr;
        return false;
      }

      time_base = tb_hdr;
      if (format_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
        codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
      }
      if (avcodec_open2(codec_ctx, hdr_codec, nullptr) < 0) {
        std::cerr << "MXWrite: could not open libx265 for HDR output.\n";
        avcodec_free_context(&codec_ctx);
        avformat_free_context(format_ctx);
        format_ctx = nullptr;
        return false;
      }
      if (avcodec_parameters_from_context(stream->codecpar, codec_ctx) < 0) {
        std::cerr << "MXWrite: could not copy HDR codec parameters.\n";
        avcodec_free_context(&codec_ctx);
        avformat_free_context(format_ctx);
        format_ctx = nullptr;
        return false;
      }

      // Attach mastering-display / content-light side data to the stream
      // codec parameters. Uses the modern AVCodecParameters coded_side_data
      // API. Failures are logged but non-fatal.
      auto attach_side = [&](AVPacketSideDataType type,
                             const std::vector<uint8_t> &payload) {
        if (payload.empty())
          return;
        uint8_t *buf = static_cast<uint8_t *>(av_malloc(payload.size()));
        if (!buf)
          return;
        std::memcpy(buf, payload.data(), payload.size());
        const AVPacketSideData *added =
            av_packet_side_data_add(&stream->codecpar->coded_side_data,
                                    &stream->codecpar->nb_coded_side_data, type,
                                    buf, payload.size(), 0);
        if (!added) {
          av_free(buf);
          std::cerr << "MXWrite: failed to attach HDR side data (type "
                    << (int)type << ").\n";
        }
      };
      attach_side(AV_PKT_DATA_MASTERING_DISPLAY_METADATA,
                  hdr_info.mastering_display);
      attach_side(AV_PKT_DATA_CONTENT_LIGHT_LEVEL, hdr_info.content_light);

      if (!(format_ctx->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&format_ctx->pb, filename.c_str(), AVIO_FLAG_WRITE) < 0) {
          std::cerr << "MXWrite: could not open HDR output file: " << filename
                    << "\n";
          avcodec_free_context(&codec_ctx);
          avformat_free_context(format_ctx);
          format_ctx = nullptr;
          return false;
        }
      }
      if (avformat_write_header(format_ctx, nullptr) < 0) {
        std::cerr << "MXWrite: error writing HDR MP4 header.\n";
        avio_closep(&format_ctx->pb);
        avcodec_free_context(&codec_ctx);
        avformat_free_context(format_ctx);
        format_ctx = nullptr;
        return false;
      }

      // Allocate the 10-bit YUV staging frame used by encodeAndWriteFrame.
      frame10 = av_frame_alloc();
      if (!frame10) {
        std::cerr << "MXWrite: could not allocate YUV420P10LE frame.\n";
        avio_closep(&format_ctx->pb);
        avcodec_free_context(&codec_ctx);
        avformat_free_context(format_ctx);
        format_ctx = nullptr;
        return false;
      }
      frame10->format = AV_PIX_FMT_YUV420P10LE;
      frame10->width = width;
      frame10->height = height;
      if (av_frame_get_buffer(frame10, 32) < 0) {
        std::cerr << "MXWrite: could not allocate YUV420P10LE buffer.\n";
        av_frame_free(&frame10);
        avio_closep(&format_ctx->pb);
        avcodec_free_context(&codec_ctx);
        avformat_free_context(format_ctx);
        format_ctx = nullptr;
        return false;
      }

      opened = true;
      use_hw_encode = false;
      recordingStart = std::chrono::steady_clock::now();
      startEncoderThread();
      std::cout << "MXWrite: HDR output active (libx265 Main10, BT.2020, "
                << (codec_ctx->color_trc == AVCOL_TRC_ARIB_STD_B67 ? "HLG"
                                                                   : "PQ")
                << ")\n";
      return true;
    }
    // ---- End HDR path -----------------------------------------------------

    const bool is_high_res = (width > 3840 || height > 2160);
    std::string codec_pref =
        lowercase_ascii(codec_override ? *codec_override : opts.codec);
    if (codec_pref.empty()) {
      codec_pref = "auto";
    }
    const bool explicit_hevc_nvenc =
        (codec_pref == "hevc_nvenc" || codec_pref == "h265_nvenc");
    const bool explicit_h264_nvenc = (codec_pref == "h264_nvenc");
    const bool explicit_hevc_software =
        (codec_pref == "hevc" || codec_pref == "h265" ||
         codec_pref == "libx265");
    const bool explicit_h264_software =
        (codec_pref == "h264" || codec_pref == "libx264");
    const bool use_hevc_codec =
        explicit_hevc_nvenc || explicit_hevc_software ||
        (!explicit_h264_nvenc && !explicit_h264_software && is_high_res);
    std::string hw_codec_name = use_hevc_codec ? "hevc_nvenc" : "h264_nvenc";
    AVCodecID sw_codec_id =
        use_hevc_codec ? AV_CODEC_ID_HEVC : AV_CODEC_ID_H264;

    // Codec selection based on user preference.
    const AVCodec *codec = nullptr;
    bool wants_hw = false;
    if (codec_pref == "software" || codec_pref == "x264" ||
        codec_pref == "cpu" || explicit_hevc_software ||
        explicit_h264_software) {
      if (codec_pref == "libx264" || codec_pref == "libx265") {
        codec = avcodec_find_encoder_by_name(codec_pref.c_str());
      } else {
        codec = avcodec_find_encoder(sw_codec_id);
      }
      wants_hw = false;
    } else if (codec_pref == "auto" || codec_pref == "nvenc" ||
               explicit_hevc_nvenc || explicit_h264_nvenc) {
      // "auto" or "nvenc" keeps the resolution-based default; concrete
      // names like "hevc_nvenc" and "h264_nvenc" select that NVENC codec.
      codec = avcodec_find_encoder_by_name(hw_codec_name.c_str());
      wants_hw = (codec != nullptr);
      if (!codec) {
        if (codec_pref == "nvenc" || explicit_hevc_nvenc ||
            explicit_h264_nvenc) {
          std::cerr << "MXWrite: NVENC requested but " << hw_codec_name
                    << " not available; falling back to software.\n";
        }
        codec = avcodec_find_encoder(sw_codec_id);
      }
    } else {
      // Concrete FFmpeg encoder names are kept distinct. This permits all
      // linked encoders that accept system-memory frames (for example AV1,
      // VP9, ProRes, FFV1, QSV, AMF, VideoToolbox, and V4L2 M2M) instead of
      // collapsing the request to the default H.264 encoder.
      codec = avcodec_find_encoder_by_name(codec_pref.c_str());
      wants_hw = codec && (codec_pref.ends_with("_nvenc"));
      if (wants_hw) {
        hw_codec_name = codec_pref;
        sw_codec_id = codec->id;
      }
    }

    if (!codec) {
      std::cerr << "MXWrite: could not find requested video encoder '"
                << codec_pref
                << "'. Use acmx2 --list-encoders to see this FFmpeg build.\n";
      avformat_free_context(format_ctx);
      format_ctx = nullptr;
      return false;
    }

    // Validate / sanitise preset and CRF.
    std::string preset =
        opts.preset.empty() ? std::string("medium") : opts.preset;
    if (!is_valid_x264_preset(preset)) {
      // Accept unknown names; forward as-is. If empty, medium.
    }
    int crf_val = opts.crf;
    if (crf_val < 0)
      crf_val = 0;
    if (crf_val > 51)
      crf_val = 51;
    const std::string crf_str = std::to_string(crf_val);

    stream = avformat_new_stream(format_ctx, codec);
    if (!stream) {
      std::cerr << "Could not create new stream.\n";
      avformat_free_context(format_ctx);
      format_ctx = nullptr;
      return false;
    }

    calculateFPSFraction(fps, fps_num, fps_den);

    AVRational tb = {fps_den, fps_num};
    stream->time_base = tb;

    codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
      std::cerr << "Could not allocate codec context.\n";
      avformat_free_context(format_ctx);
      format_ctx = nullptr;
      return false;
    }

    codec_ctx->width = width;
    codec_ctx->height = height;
    codec_ctx->time_base = stream->time_base;
    codec_ctx->framerate = AVRational{fps_num, fps_den};
    AVPixelFormat software_pixel_format = choose_software_pixel_format(
        wants_hw ? avcodec_find_encoder(sw_codec_id) : codec,
        requested_pixel_format);
    const bool requires_hardware_frames =
        !wants_hw && software_pixel_format == AV_PIX_FMT_NONE &&
        codec_uses_hardware(codec);
    if (software_pixel_format == AV_PIX_FMT_NONE && !requires_hardware_frames) {
      const char *requested_name =
          requested_pixel_format == AV_PIX_FMT_NONE
              ? "an automatic system-memory format"
              : av_get_pix_fmt_name(requested_pixel_format);
      std::cerr << "MXWrite: encoder '" << codec->name << "' does not accept "
                << (requested_name ? requested_name
                                   : "the requested pixel format")
                << " that MXWrite can convert from RGBA. Choose a supported "
                   "-pix_fmt; "
                   "hardware-frame-only encoders require a device-specific "
                   "upload path.\n";
      avcodec_free_context(&codec_ctx);
      avformat_free_context(format_ctx);
      format_ctx = nullptr;
      return false;
    }
    codec_ctx->pix_fmt = wants_hw ? AV_PIX_FMT_CUDA : software_pixel_format;
    codec_ctx->gop_size = 30;
    codec_ctx->max_b_frames = 0;
    codec_ctx->thread_count = std::max(1u, std::thread::hardware_concurrency());
    // Frame threading scales much better than slice threading for x264 when
    // latency is not a concern; switch only to slice threading in realtime/ts.
    if (ts_mode || opts.realtime) {
      codec_ctx->thread_type = FF_THREAD_SLICE;
      codec_ctx->slices = 4;
    } else {
      codec_ctx->thread_type = FF_THREAD_FRAME | FF_THREAD_SLICE;
    }
    codec_ctx->delay = 0;

    if (ts_mode || opts.realtime) {
      codec_ctx->flags |= AV_CODEC_FLAG_LOW_DELAY;
    }

    bool extra_options_applied = false;
    bool fell_back_from_hardware = false;
    if (requires_hardware_frames) {
      set_named_encoder_option(codec_ctx->priv_data, "preset", preset);
      if (!opts.tune.empty() && opts.tune != "none") {
        set_named_encoder_option(codec_ctx->priv_data, "tune", opts.tune);
      }
      set_named_encoder_option(codec_ctx->priv_data, "crf", crf_str);
      if (!apply_ffmpeg_options(extra_options, codec_ctx, format_ctx)) {
        avcodec_free_context(&codec_ctx);
        avformat_free_context(format_ctx);
        format_ctx = nullptr;
        return false;
      }
      extra_options_applied = true;
      if (!initHardwareEncoding(codec, requested_pixel_format, false)) {
        std::cerr
            << "MXWrite: encoder '" << codec->name
            << "' requires hardware frames, but its FFmpeg hardware device "
               "could not be initialized.\n";
        av_buffer_unref(&hw_frames_ctx);
        av_buffer_unref(&hw_device_ctx);
        av_frame_free(&upload_sw_frame);
        sws_freeContext(sws_ctx);
        sws_ctx = nullptr;
        avcodec_free_context(&codec_ctx);
        avformat_free_context(format_ctx);
        format_ctx = nullptr;
        return false;
      }
      use_hw_encode = true;
      std::cout << "MXWrite: hardware encoder selected (" << codec->name
                << ")\n";
    }
    if (wants_hw) {
      const char *nv_preset = x264_preset_to_nvenc(preset);
      av_opt_set(codec_ctx->priv_data, "preset", nv_preset, 0);
      // NVENC "tune": hq (high quality), ll (low latency), ull (ultra low
      // latency), lossless.
      const std::string requested_tune = lowercase_ascii(opts.tune);
      const std::string nv_tune =
          opts.realtime ? std::string("ll")
                        : (is_nvenc_tune(requested_tune) ? requested_tune
                                                         : std::string("hq"));
      av_opt_set(codec_ctx->priv_data, "tune", nv_tune.c_str(), 0);
      const std::string *custom_tune =
          find_ffmpeg_option(extra_options, "tune");
      const bool lossless_tune =
          nv_tune == "lossless" ||
          (custom_tune && lowercase_ascii(*custom_tune) == "lossless");
      if (!lossless_tune) {
        av_opt_set(codec_ctx->priv_data, "rc", "vbr", 0);
        av_opt_set(codec_ctx->priv_data, "cq", crf_str.c_str(), 0);
      }
      if (opts.realtime) {
        av_opt_set(codec_ctx->priv_data, "zerolatency", "1", 0);
      }
      if (use_hevc_codec) {
        av_opt_set(codec_ctx->priv_data, "tier", "high", 0);
      }

      if (requested_pixel_format == AV_PIX_FMT_YUV444P) {
        if (av_opt_set(codec_ctx->priv_data, "rgb_mode", "yuv444", 0) < 0) {
          std::cerr << "MXWrite: this NVENC build cannot convert RGBA input to "
                       "yuv444p.\n";
          avcodec_free_context(&codec_ctx);
          avformat_free_context(format_ctx);
          format_ctx = nullptr;
          return false;
        }
        if (!find_ffmpeg_option(extra_options, "profile")) {
          const char *profile = use_hevc_codec ? "rext" : "high444p";
          av_opt_set(codec_ctx->priv_data, "profile", profile, 0);
        }
      } else if (requested_pixel_format != AV_PIX_FMT_NONE &&
                 requested_pixel_format != AV_PIX_FMT_YUV420P &&
                 requested_pixel_format != AV_PIX_FMT_RGBA) {
        std::cerr << "MXWrite: NVENC RGBA ingestion supports custom -pix_fmt "
                     "yuv420p, "
                     "yuv444p, or rgba; requested '"
                  << av_get_pix_fmt_name(requested_pixel_format) << "'.\n";
        avcodec_free_context(&codec_ctx);
        avformat_free_context(format_ctx);
        format_ctx = nullptr;
        return false;
      }

      if (!apply_ffmpeg_options(extra_options, codec_ctx, format_ctx)) {
        avcodec_free_context(&codec_ctx);
        avformat_free_context(format_ctx);
        format_ctx = nullptr;
        return false;
      }
      extra_options_applied = true;

      if (initHardwareEncoding(codec, requested_pixel_format, true)) {
        use_hw_encode = true;
        std::cout << "MXWrite: hardware encoder selected (" << hw_codec_name
                  << ")\n";
      } else {
        std::cerr << "MXWrite: " << hw_codec_name
                  << " present but CUDA context failed, falling back to "
                     "software encoder\n";
        fell_back_from_hardware = true;
        av_buffer_unref(&hw_frames_ctx);
        av_buffer_unref(&hw_device_ctx);
        av_frame_free(&upload_sw_frame);
        sws_freeContext(sws_ctx);
        sws_ctx = nullptr;
        direct_cuda_upload = false;
        avcodec_free_context(&codec_ctx);

        codec = avcodec_find_encoder(sw_codec_id);
        if (!codec) {
          std::cerr << "Could not find software fallback encoder.\n";
          avformat_free_context(format_ctx);
          format_ctx = nullptr;
          return false;
        }

        codec_ctx = avcodec_alloc_context3(codec);
        if (!codec_ctx) {
          std::cerr << "Could not allocate fallback codec context.\n";
          avformat_free_context(format_ctx);
          format_ctx = nullptr;
          return false;
        }

        codec_ctx->width = width;
        codec_ctx->height = height;
        codec_ctx->time_base = stream->time_base;
        codec_ctx->framerate = AVRational{fps_num, fps_den};
        codec_ctx->pix_fmt = software_pixel_format;
        codec_ctx->gop_size = 30;
        codec_ctx->max_b_frames = 0;
        codec_ctx->thread_count =
            std::max(1u, std::thread::hardware_concurrency());
        if (ts_mode || opts.realtime) {
          codec_ctx->thread_type = FF_THREAD_SLICE;
          codec_ctx->slices = 4;
        } else {
          codec_ctx->thread_type = FF_THREAD_FRAME | FF_THREAD_SLICE;
        }
        codec_ctx->delay = 0;

        if (ts_mode || opts.realtime) {
          codec_ctx->flags |= AV_CODEC_FLAG_LOW_DELAY;
        }
        extra_options_applied = false;
      }
    }

    if (!use_hw_encode) {
      const std::string software_preset =
          fell_back_from_hardware
              ? nvenc_preset_to_software(lowercase_ascii(preset))
              : preset;
      set_named_encoder_option(codec_ctx->priv_data, "preset", software_preset);
      // Apply tune: realtime forces zerolatency; otherwise honour user value.
      std::string tune = opts.realtime ? std::string("zerolatency") : opts.tune;
      if (fell_back_from_hardware) {
        const std::string lowered_tune = lowercase_ascii(tune);
        if (lowered_tune == "hq" || lowered_tune == "uhq") {
          tune.clear();
        } else if (lowered_tune == "ll" || lowered_tune == "ull") {
          tune = "zerolatency";
        } else if (lowered_tune == "lossless") {
          tune.clear();
          if (use_hevc_codec) {
            av_opt_set(codec_ctx->priv_data, "x265-params", "lossless=1", 0);
          } else {
            av_opt_set(codec_ctx->priv_data, "qp", "0", 0);
          }
        }
      }
      if (!tune.empty() && tune != "none") {
        set_named_encoder_option(codec_ctx->priv_data, "tune", tune);
      }
      set_named_encoder_option(codec_ctx->priv_data, "crf", crf_str);
      if (opts.realtime && codec->id == AV_CODEC_ID_H264) {
        // Legacy low-latency parameters kept for realtime path to avoid
        // pipeline stalls during live capture.
        av_opt_set(codec_ctx->priv_data, "x264-params",
                   "bframes=0:ref=1:me=dia:subme=0", 0);
        av_opt_set(codec_ctx->priv_data, "force_cfr", "1", 0);
      }
    }

    const std::vector<FfmpegOption> translated_fallback_options =
        fell_back_from_hardware
            ? software_fallback_options(extra_options, use_hevc_codec)
            : std::vector<FfmpegOption>{};
    const std::vector<FfmpegOption> &options_to_apply =
        fell_back_from_hardware ? translated_fallback_options : extra_options;
    if (!extra_options_applied &&
        !apply_ffmpeg_options(options_to_apply, codec_ctx, format_ctx)) {
      avcodec_free_context(&codec_ctx);
      avformat_free_context(format_ctx);
      format_ctx = nullptr;
      return false;
    }

    time_base = tb;

    if (format_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
      codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    if (avcodec_open2(codec_ctx, codec, nullptr) < 0) {
      std::cerr
          << "MXWrite: could not open encoder '" << codec->name
          << "'. The encoder may require unavailable hardware or options.\n";
      avcodec_free_context(&codec_ctx);
      avformat_free_context(format_ctx);
      format_ctx = nullptr;
      return false;
    }
    active_encoder_hardware = codec_uses_hardware(codec);
    if (avcodec_parameters_from_context(stream->codecpar, codec_ctx) < 0) {
      std::cerr << "Could not copy codec parameters.\n";
      avcodec_free_context(&codec_ctx);
      avformat_free_context(format_ctx);
      format_ctx = nullptr;
      return false;
    }
    if (!(format_ctx->oformat->flags & AVFMT_NOFILE)) {
      if (avio_open(&format_ctx->pb, filename.c_str(), AVIO_FLAG_WRITE) < 0) {
        std::cerr << "Could not open output file: " << filename << "\n";
        avcodec_free_context(&codec_ctx);
        avformat_free_context(format_ctx);
        format_ctx = nullptr;
        return false;
      }
    }
    if (avformat_write_header(format_ctx, nullptr) < 0) {
      std::cerr << "Error writing MP4 header.\n";
      avio_closep(&format_ctx->pb);
      avcodec_free_context(&codec_ctx);
      avformat_free_context(format_ctx);
      format_ctx = nullptr;
      return false;
    }

    if (!use_hw_encode) {
      frameYUV = av_frame_alloc();
      if (!frameYUV) {
        std::cerr << "Could not allocate YUV frame.\n";
        avio_closep(&format_ctx->pb);
        avcodec_free_context(&codec_ctx);
        avformat_free_context(format_ctx);
        format_ctx = nullptr;
        return false;
      }
      frameYUV->format = software_pixel_format;
      frameYUV->width = width;
      frameYUV->height = height;
      if (av_frame_get_buffer(frameYUV, 32) < 0) {
        std::cerr << "Could not allocate frame buffer for YUV frame.\n";
        av_frame_free(&frameYUV);
        avio_closep(&format_ctx->pb);
        avcodec_free_context(&codec_ctx);
        avformat_free_context(format_ctx);
        format_ctx = nullptr;
        return false;
      }

      sws_ctx = sws_getContext(width, height, AV_PIX_FMT_RGBA, width, height,
                               software_pixel_format, SWS_FAST_BILINEAR,
                               nullptr, nullptr, nullptr);
      if (!sws_ctx) {
        std::cerr << "Could not initialize conversion context.\n";
        av_frame_free(&frameYUV);
        avio_closep(&format_ctx->pb);
        avcodec_free_context(&codec_ctx);
        avformat_free_context(format_ctx);
        format_ctx = nullptr;
        return false;
      }
    }

    opened = true;
    recordingStart = std::chrono::steady_clock::now();
    startEncoderThread();
    return true;
  }

  void Writer::write(void *rgba_buffer) {
    write_at_pts(rgba_buffer, AV_NOPTS_VALUE);
  }

  void Writer::write_at_pts(void *rgba_buffer, int64_t pts) {
    if (!rgba_buffer) {
      return;
    }

    {
      std::lock_guard<std::mutex> lock(writer_mutex);
      if (!opened) {
        return;
      }
    }

    AVFrame *queued_frame = av_frame_alloc();
    if (!queued_frame) {
      std::cerr << "Writer: failed to allocate queued frame\n";
      return;
    }

    if (use_hw_encode) {
      queued_frame->format = codec_ctx->pix_fmt;
      queued_frame->width = width;
      queued_frame->height = height;
      if (av_hwframe_get_buffer(hw_frames_ctx, queued_frame, 0) < 0) {
        std::cerr << "Writer: failed to allocate frame from hardware pool\n";
        releaseFrame(queued_frame);
        return;
      }

      if (av_frame_make_writable(upload_sw_frame) < 0) {
        std::cerr << "Writer: software upload frame not writable\n";
        releaseFrame(queued_frame);
        return;
      }

      const auto *src = static_cast<const uint8_t *>(rgba_buffer);
      if (upload_sw_frame->format == AV_PIX_FMT_RGBA) {
        for (int y = 0; y < height; ++y) {
          std::memcpy(upload_sw_frame->data[0] +
                          static_cast<size_t>(y) * upload_sw_frame->linesize[0],
                      src + static_cast<size_t>(y) *
                                static_cast<size_t>(width) * 4,
                      static_cast<size_t>(width) * 4);
        }
      } else {
        const uint8_t *source_data[1] = {src};
        const int source_linesize[1] = {width * 4};
        sws_scale(sws_ctx, source_data, source_linesize, 0, height,
                  upload_sw_frame->data, upload_sw_frame->linesize);
      }

      if (av_hwframe_transfer_data(queued_frame, upload_sw_frame, 0) < 0) {
        std::cerr
            << "Writer: failed to transfer system frame to hardware frame\n";
        releaseFrame(queued_frame);
        return;
      }
    } else {
      queued_frame->format = AV_PIX_FMT_RGBA;
      queued_frame->width = width;
      queued_frame->height = height;
      if (av_frame_get_buffer(queued_frame, 32) < 0) {
        std::cerr << "Writer: failed to allocate queued RGBA frame buffer\n";
        releaseFrame(queued_frame);
        return;
      }
      if (av_frame_make_writable(queued_frame) < 0) {
        std::cerr << "Writer: queued RGBA frame not writable\n";
        releaseFrame(queued_frame);
        return;
      }

      const auto *src = static_cast<const uint8_t *>(rgba_buffer);
      for (int y = 0; y < height; ++y) {
        std::memcpy(queued_frame->data[0] +
                        static_cast<size_t>(y) * queued_frame->linesize[0],
                    src +
                        static_cast<size_t>(y) * static_cast<size_t>(width) * 4,
                    static_cast<size_t>(width) * 4);
      }
    }

    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      if (block_when_full.load(std::memory_order_relaxed)) {
        if (encode_queue.size() >= NO_DROP_QUEUE_SIZE) {
          // Keep file processing paced with the encoder. Wake whenever
          // one queue slot becomes available rather than waiting for a
          // large queue to fill and then drain completely.
          queue_cv.wait(lock, [this] {
            return stop_requested || encode_queue.size() < NO_DROP_QUEUE_SIZE;
          });
        }
        if (stop_requested) {
          releaseFrame(queued_frame);
          return;
        }
      } else if (stop_requested || encode_queue.size() >= MAX_QUEUE_SIZE) {
        static int drop_counter = 0;
        if (++drop_counter % 30 == 0) {
          std::cerr << "Writer: dropped " << drop_counter
                    << " SDR frames (encoder queue full)\n";
        }
        releaseFrame(queued_frame);
        return;
      }
      if (pts != AV_NOPTS_VALUE && pts < frame_count) {
        // A timestamped live frame landed in a nominal frame slot that
        // was already filled. Drop it instead of extending the video
        // timeline and allowing it to drift behind the capture clock.
        releaseFrame(queued_frame);
        return;
      }
      queued_frame->pts = pts == AV_NOPTS_VALUE ? frame_count : pts;
      frame_count = queued_frame->pts + 1;
      encode_queue.push(queued_frame);
    }

    queue_cv.notify_one();
  }

  void Writer::write_hdr_rgba16(void *rgba16_buffer) {
    write_hdr_rgba16_at_pts(rgba16_buffer, AV_NOPTS_VALUE);
  }

  void Writer::write_hdr_rgba16_at_pts(void *rgba16_buffer, int64_t pts) {
    if (!rgba16_buffer) {
      return;
    }

    {
      std::lock_guard<std::mutex> lock(writer_mutex);
      if (!opened) {
        return;
      }
      if (!hdr_output) {
        std::cerr
            << "Writer: write_hdr_rgba16 called but writer not in HDR mode\n";
        return;
      }
    }

    AVFrame *queued_frame = av_frame_alloc();
    if (!queued_frame) {
      std::cerr << "Writer: failed to allocate queued HDR frame\n";
      return;
    }
    queued_frame->format = AV_PIX_FMT_YUV420P10LE;
    queued_frame->width = width;
    queued_frame->height = height;
    if (av_frame_get_buffer(queued_frame, 32) < 0) {
      std::cerr << "Writer: failed to allocate YUV420P10 buffer\n";
      releaseFrame(queued_frame);
      return;
    }
    if (av_frame_make_writable(queued_frame) < 0) {
      std::cerr << "Writer: queued HDR frame not writable\n";
      releaseFrame(queued_frame);
      return;
    }

    // Convert the already-PQ/HLG-encoded 16-bit BT.2020 RGBA into the
    // 10-bit limited-range YUV420 plane that libx265 Main10 expects.
    convertBt2020Rgba16EncodedToYuv420p10(
        reinterpret_cast<const uint16_t *>(rgba16_buffer), width * 4,
        reinterpret_cast<uint16_t *>(queued_frame->data[0]),
        queued_frame->linesize[0] / 2,
        reinterpret_cast<uint16_t *>(queued_frame->data[1]),
        queued_frame->linesize[1] / 2,
        reinterpret_cast<uint16_t *>(queued_frame->data[2]),
        queued_frame->linesize[2] / 2, width, height);

    queued_frame->color_primaries = static_cast<AVColorPrimaries>(
        hdr_info.color_primaries ? hdr_info.color_primaries : AVCOL_PRI_BT2020);
    queued_frame->color_trc = static_cast<AVColorTransferCharacteristic>(
        hdr_info.color_trc ? hdr_info.color_trc : AVCOL_TRC_SMPTE2084);
    queued_frame->colorspace = static_cast<AVColorSpace>(
        hdr_info.color_space ? hdr_info.color_space : AVCOL_SPC_BT2020_NCL);
    queued_frame->color_range = static_cast<AVColorRange>(
        hdr_info.color_range ? hdr_info.color_range : AVCOL_RANGE_MPEG);

    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      if (block_when_full.load(std::memory_order_relaxed)) {
        if (encode_queue.size() >= NO_DROP_QUEUE_SIZE) {
          queue_cv.wait(lock, [this] {
            return stop_requested || encode_queue.size() < NO_DROP_QUEUE_SIZE;
          });
        }
        if (stop_requested) {
          releaseFrame(queued_frame);
          return;
        }
      } else if (stop_requested || encode_queue.size() >= MAX_QUEUE_SIZE) {
        static int drop_counter = 0;
        if (++drop_counter % 30 == 0) {
          std::cerr << "Writer: dropped " << drop_counter
                    << " HDR frames (encoder queue full)\n";
        }
        releaseFrame(queued_frame);
        return;
      }
      if (pts != AV_NOPTS_VALUE && pts < frame_count) {
        releaseFrame(queued_frame);
        return;
      }
      queued_frame->pts = pts == AV_NOPTS_VALUE ? frame_count : pts;
      frame_count = queued_frame->pts + 1;
      encode_queue.push(queued_frame);
    }

    queue_cv.notify_one();
  }

  bool Writer::write_cuda_rgba(void *cuda_rgba_buffer, int src_stride,
                               [[maybe_unused]] bool bottom_up) {
    return write_cuda_rgba_at_pts(cuda_rgba_buffer, src_stride, AV_NOPTS_VALUE,
                                  bottom_up);
  }

  bool Writer::write_cuda_rgba_at_pts(void *cuda_rgba_buffer, int src_stride,
                                      int64_t pts,
                                      [[maybe_unused]] bool bottom_up) {
    if (!cuda_rgba_buffer || src_stride <= 0) {
      return false;
    }

    {
      std::lock_guard<std::mutex> lock(writer_mutex);
      if (!opened || !use_hw_encode || !direct_cuda_upload) {
        return false;
      }
    }

    AVFrame *queued_frame = av_frame_alloc();
    if (!queued_frame) {
      std::cerr << "Writer: failed to allocate queued CUDA frame\n";
      return false;
    }

    queued_frame->format = AV_PIX_FMT_CUDA;
    queued_frame->width = width;
    queued_frame->height = height;

    if (av_hwframe_get_buffer(hw_frames_ctx, queued_frame, 0) < 0) {
      std::cerr << "Writer: failed to allocate CUDA frame from hardware pool\n";
      releaseFrame(queued_frame);
      return false;
    }

#ifdef MXWRITE_HAS_CUDA_COPY
    cudaStream_t stream = cuda_upload_stream;
    const bool use_async_stream = (stream != nullptr);
    if (!bottom_up) {
      const auto copy_err =
          use_async_stream
              ? cudaMemcpy2DAsync(
                    queued_frame->data[0],
                    static_cast<size_t>(queued_frame->linesize[0]),
                    cuda_rgba_buffer, static_cast<size_t>(src_stride),
                    static_cast<size_t>(width) * 4, static_cast<size_t>(height),
                    cudaMemcpyDeviceToDevice, stream)
              : cudaMemcpy2D(queued_frame->data[0],
                             static_cast<size_t>(queued_frame->linesize[0]),
                             cuda_rgba_buffer, static_cast<size_t>(src_stride),
                             static_cast<size_t>(width) * 4,
                             static_cast<size_t>(height),
                             cudaMemcpyDeviceToDevice);

      if (copy_err != cudaSuccess) {
        std::cerr << "Writer: cudaMemcpy2D device upload failed: "
                  << cudaGetErrorString(copy_err) << "\n";
        releaseFrame(queued_frame);
        return false;
      }
    } else {
      // Flip vertically by issuing one async row copy per destination row
      // onto a single stream — kept asynchronous so launch overhead overlaps
      // and the producer thread only blocks once at cudaStreamSynchronize.
      auto *src_base = static_cast<unsigned char *>(cuda_rgba_buffer);
      auto *dst_base = queued_frame->data[0];
      const size_t row_bytes = static_cast<size_t>(width) * 4;

      for (int y = 0; y < height; ++y) {
        auto *src_row = src_base + static_cast<size_t>(height - 1 - y) *
                                       static_cast<size_t>(src_stride);
        auto *dst_row =
            dst_base + static_cast<size_t>(y) *
                           static_cast<size_t>(queued_frame->linesize[0]);
        const auto row_copy_err =
            use_async_stream ? cudaMemcpyAsync(dst_row, src_row, row_bytes,
                                               cudaMemcpyDeviceToDevice, stream)
                             : cudaMemcpy(dst_row, src_row, row_bytes,
                                          cudaMemcpyDeviceToDevice);
        if (row_copy_err != cudaSuccess) {
          std::cerr << "Writer: cudaMemcpy row upload failed: "
                    << cudaGetErrorString(row_copy_err) << "\n";
          releaseFrame(queued_frame);
          return false;
        }
      }
    }
    // Single synchronisation point — NVENC requires the data to be ready when
    // avcodec_send_frame() reads it, and the encoder thread is decoupled by
    // the queue so this sync only serialises this one producer call.
    if (use_async_stream) {
      const auto sync_err = cudaStreamSynchronize(stream);
      if (sync_err != cudaSuccess) {
        std::cerr << "Writer: cudaStreamSynchronize failed: "
                  << cudaGetErrorString(sync_err) << "\n";
        releaseFrame(queued_frame);
        return false;
      }
    }
#else
    std::cerr << "Writer: CUDA copy support disabled at build time\n";
    releaseFrame(queued_frame);
    return false;
#endif

    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      if (block_when_full.load(std::memory_order_relaxed)) {
        if (encode_queue.size() >= NO_DROP_QUEUE_SIZE) {
          queue_cv.wait(lock, [this] {
            return stop_requested || encode_queue.size() < NO_DROP_QUEUE_SIZE;
          });
        }
        if (stop_requested) {
          releaseFrame(queued_frame);
          return false;
        }
      } else if (stop_requested || encode_queue.size() >= MAX_QUEUE_SIZE) {
        static int drop_counter = 0;
        if (++drop_counter % 30 == 0) {
          std::cerr << "Writer: dropped " << drop_counter
                    << " frames (encoder queue full)\n";
        }
        releaseFrame(queued_frame);
        // Return TRUE so the producer does NOT fall back to the slow CPU
        // write() path — we already "handled" the frame (by dropping it).
        // Falling back would double-process every frame and double the drops.
        queue_cv.notify_one();
        return true;
      }
      if (pts != AV_NOPTS_VALUE && pts < frame_count) {
        releaseFrame(queued_frame);
        queue_cv.notify_one();
        return true;
      }
      queued_frame->pts = pts == AV_NOPTS_VALUE ? frame_count : pts;
      frame_count = queued_frame->pts + 1;
      encode_queue.push(queued_frame);
    }

    queue_cv.notify_one();
    return true;
  }

  void Writer::write_ts(void *rgba_buffer) { write(rgba_buffer); }

  void Writer::startEncoderThread() {
    stop_requested = false;
    encode_thread =
        std::jthread([this](std::stop_token st) { encodeLoop(st); });
  }

  void Writer::stopEncoderThread() {
    {
      std::lock_guard<std::mutex> lock(queue_mutex);
      stop_requested = true;
    }
    queue_cv.notify_all();

    if (encode_thread.joinable()) {
      encode_thread.request_stop();
      encode_thread.join();
    }
  }

  void Writer::releaseFrame(AVFrame * f) {
    if (!f) {
      return;
    }
    av_frame_free(&f);
  }

  void Writer::drainEncoderPackets() {
    AVPacket *pkt = av_packet_alloc();
    if (!pkt) {
      std::cerr << "Writer: failed to allocate packet\n";
      return;
    }

    while (true) {
      int ret = avcodec_receive_packet(codec_ctx, pkt);
      if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        break;
      }
      if (ret < 0) {
        std::cerr << "Writer: error receiving packet: " << ret << "\n";
        break;
      }

      av_packet_rescale_ts(pkt, codec_ctx->time_base, stream->time_base);
      pkt->stream_index = stream->index;

      if (av_interleaved_write_frame(format_ctx, pkt) < 0) {
        std::cerr << "Writer: error writing frame\n";
        av_packet_unref(pkt);
        break;
      }
      av_packet_unref(pkt);
    }

    av_packet_free(&pkt);
  }

  void Writer::encodeAndWriteFrame(AVFrame * in_frame) {
    if (!in_frame) {
      return;
    }

    AVFrame *encode_frame = in_frame;
    if (hdr_output) {
      if (in_frame->format == AV_PIX_FMT_YUV420P10LE) {
        // Frame has already been converted to BT.2020 PQ YUV420P10LE
        // by write_hdr_rgba16(). Use directly.
        encode_frame = in_frame;
      } else {
        // in_frame is RGBA 8-bit from the shader pipeline. Convert to BT.2020
        // PQ YUV420P10LE in frame10 and submit that instead.
        if (av_frame_make_writable(frame10) < 0) {
          std::cerr << "Writer: HDR frame not writable\n";
          return;
        }
        convertRgbaToBt2020PqYuv420p10(
            in_frame->data[0], in_frame->linesize[0],
            reinterpret_cast<uint16_t *>(frame10->data[0]),
            frame10->linesize[0] / 2,
            reinterpret_cast<uint16_t *>(frame10->data[1]),
            frame10->linesize[1] / 2,
            reinterpret_cast<uint16_t *>(frame10->data[2]),
            frame10->linesize[2] / 2, width, height);
        frame10->pts = in_frame->pts;
        frame10->color_primaries = static_cast<AVColorPrimaries>(
            hdr_info.color_primaries ? hdr_info.color_primaries
                                     : AVCOL_PRI_BT2020);
        frame10->color_trc = static_cast<AVColorTransferCharacteristic>(
            hdr_info.color_trc ? hdr_info.color_trc : AVCOL_TRC_SMPTE2084);
        frame10->colorspace = static_cast<AVColorSpace>(
            hdr_info.color_space ? hdr_info.color_space : AVCOL_SPC_BT2020_NCL);
        frame10->color_range = static_cast<AVColorRange>(
            hdr_info.color_range ? hdr_info.color_range : AVCOL_RANGE_MPEG);
        encode_frame = frame10;
      }
    } else if (!use_hw_encode) {
      const uint8_t *src_data[1] = {in_frame->data[0]};
      int src_linesize[1] = {in_frame->linesize[0]};
      sws_scale(sws_ctx, src_data, src_linesize, 0, height, frameYUV->data,
                frameYUV->linesize);
      frameYUV->pts = in_frame->pts;
      encode_frame = frameYUV;
    }

    int ret = avcodec_send_frame(codec_ctx, encode_frame);
    if (ret == AVERROR(EAGAIN)) {
      // Encoder output queue is full; drain and retry this frame once.
      drainEncoderPackets();
      ret = avcodec_send_frame(codec_ctx, encode_frame);
    }
    if (ret < 0) {
      std::cerr << "Writer: error sending frame to encoder: " << ret << "\n";
      return;
    }

    drainEncoderPackets();
  }

  void Writer::encodeLoop(std::stop_token stop_token) {
    while (true) {
      AVFrame *frame = nullptr;
      {
        std::unique_lock<std::mutex> lock(queue_mutex);
        queue_cv.wait(lock, [this, &stop_token]() {
          return stop_requested || stop_token.stop_requested() ||
                 !encode_queue.empty();
        });

        if ((stop_requested || stop_token.stop_requested()) &&
            encode_queue.empty()) {
          break;
        }

        frame = encode_queue.front();
        encode_queue.pop();
      }
      // Wake any producer blocked in write() waiting for queue space.
      queue_cv.notify_one();

      encodeAndWriteFrame(frame);
      releaseFrame(frame);
    };

    if (codec_ctx) {
      const int flush_ret = avcodec_send_frame(codec_ctx, nullptr);
      if (flush_ret >= 0) {
        drainEncoderPackets();
      }
    }
  }
  void Writer::close() {
    std::lock_guard<std::mutex> lock(writer_mutex);
    if (!opened) {
      return;
    }

    stopEncoderThread();

    if (stream && stream->duration > 0) {
      last_duration =
          static_cast<double>(stream->duration) * av_q2d(stream->time_base);
    } else if (fps_num > 0 && fps_den > 0) {
      last_duration = static_cast<double>(frame_count) *
                      static_cast<double>(fps_den) /
                      static_cast<double>(fps_num);
    }

    av_write_trailer(format_ctx);

    if (!(format_ctx->oformat->flags & AVFMT_NOFILE)) {
      avio_closep(&format_ctx->pb);
    }

    av_frame_free(&frameRGBA);
    av_frame_free(&frameYUV);
    av_frame_free(&frame10);
    sws_freeContext(sws_ctx);
    av_frame_free(&upload_sw_frame);
#ifdef MXWRITE_HAS_CUDA_COPY
    // Destroy stream before tearing down FFmpeg CUDA device/frames contexts.
    if (cuda_upload_stream) {
      cudaStreamSynchronize(cuda_upload_stream);
      cudaStreamDestroy(cuda_upload_stream);
      cuda_upload_stream = nullptr;
    }
#endif
    avcodec_free_context(&codec_ctx);
    av_buffer_unref(&hw_frames_ctx);
    av_buffer_unref(&hw_device_ctx);
    avformat_free_context(format_ctx);

    while (!encode_queue.empty()) {
      releaseFrame(encode_queue.front());
      encode_queue.pop();
    }
    opened = false;
    format_ctx = nullptr;
    codec_ctx = nullptr;
    sws_ctx = nullptr;
    frameRGBA = nullptr;
    frameYUV = nullptr;
    frame10 = nullptr;
    upload_sw_frame = nullptr;
    use_hw_encode = false;
    active_encoder_hardware = false;
    direct_cuda_upload = false;
    hdr_output = false;
    stop_requested = false;
  }

  double Writer::get_duration() const {
    if (!opened && last_duration > 0.0) {
      return last_duration;
    }
    if (stream && stream->duration > 0) {
      return static_cast<double>(stream->duration) * av_q2d(stream->time_base);
    }
    if (fps_num > 0 && fps_den > 0) {
      return static_cast<double>(frame_count) * static_cast<double>(fps_den) /
             static_cast<double>(fps_num);
    }
    return 0.0;
  }
}
