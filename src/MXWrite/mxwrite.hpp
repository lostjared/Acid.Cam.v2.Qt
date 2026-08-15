/**
 * @file mxwrite.hpp
 * @brief FFmpeg-based video writer used by MXWrite.
 */
#ifndef FFWRITE_HPP
#define FFWRITE_HPP
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/hwcontext.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <queue>
#include <string>
#include <string_view>
#include <thread>
#include <vector>
#ifdef MXWRITE_HAS_CUDA_COPY
#include <cuda_runtime.h>
#endif

namespace mx {

  /**
   * @brief Queue entry that stores a frame pointer and its capture timestamp.
   */
  struct Frame_Data {
    void *data; ///< Pointer to RGBA frame data owned by the producer.
    std::chrono::steady_clock::time_point
        capture_time; ///< Capture time for timestamp-based encoding.
  };

  /** @brief A video encoder reported by the linked FFmpeg installation. */
  struct EncoderInfo {
    std::string name; ///< Exact libavcodec encoder name (for example, libx265).
    std::string long_name;  ///< Human-readable encoder description.
    std::string codec_name; ///< Encoded format name (for example, hevc or av1).
    std::string
        pixel_formats;     ///< Comma-separated supported input pixel formats.
    bool hardware = false; ///< True for hardware or hybrid encoders.
    bool experimental =
        false; ///< True when FFmpeg marks the encoder experimental.
  };

  /** @brief One configurable AVOption exposed by a video encoder. */
  struct EncoderOptionInfo {
    std::string
        name; ///< Option name accepted by EncodeOptions::ffmpeg_options.
    std::string type; ///< FFmpeg option type.
    std::string
        default_value; ///< Encoder default, when it can be represented as text.
    std::string minimum; ///< Minimum value for numeric options.
    std::string maximum; ///< Maximum value for numeric options.
    std::string
        choices;      ///< Comma-separated named values for enum-like options.
    std::string help; ///< Human-readable FFmpeg option description.
  };

  /** @return Video encoders registered by the linked FFmpeg libraries. */
  std::vector<EncoderInfo> available_video_encoders();

  /**
   * @brief Return the options exposed by one registered video encoder.
   * @param encoder_name Exact encoder name returned by
   * available_video_encoders().
   */
  std::vector<EncoderOptionInfo> video_encoder_options(
      std::string_view encoder_name);

  /**
   * @brief User-configurable video encoder quality options.
   *
   * preset: x264 preset name — ultrafast, superfast, veryfast, faster, fast,
   *         medium, slow, slower, veryslow. Mapped to NVENC p1..p7.
   * tune:   Software tune, or NVENC hq, uhq, ll, ull, or lossless. Empty uses
   *         the encoder default (NVENC hq).
   * crf:    Constant Rate Factor, 0 (lossless) .. 51 (worst). 18 is visually
   *         near-lossless; 23 is default for x264; 28 is typical "small file".
   *         For NVENC this is forwarded as `cq`.
   * codec:  "auto" (NVENC if available, else software), "software" (force
   * software), "nvenc" (force resolution-selected NVENC), or any exact video
   * encoder name registered by FFmpeg, such as "libx264", "libx265",
   * "libsvtav1", "h264_qsv", or "hevc_vaapi". NVENC policy requests fall back
   * to the matching software codec. ffmpeg_options: Additional FFmpeg-style
   * video encoder options, for example
   *         "-preset p6 -tune lossless -profile:v rext -pix_fmt yuv444p".
   *         These options override the corresponding built-in settings. MXWrite
   *         uses libavcodec directly, so input/output filenames are not
   * accepted. realtime: when true, applies low-latency defaults
   * (tune=zerolatency for x264, tune=ll + zerolatency=1 for NVENC). Extra
   * options may override them. block_when_full: when true, producer threads
   * block if the encoder queue is full instead of dropping frames.
   */
  struct EncodeOptions {
    std::string preset = "medium"; ///< Encoder preset name.
    std::string tune = "";         ///< Optional tuning mode.
    int crf = 18;                  ///< Constant Rate Factor.
    std::string codec =
        "auto"; ///< Encoder selection policy or exact FFmpeg encoder name.
    std::string
        ffmpeg_options;    ///< Additional FFmpeg-style video encoder options.
    bool realtime = false; ///< Enable low-latency settings.
    bool block_when_full = false; ///< Pace producers to encoder throughput
                                  ///< instead of dropping frames.

    /**
     * @brief HDR output options.
     *
     * When @ref HdrInfo::enabled is true, the writer switches to a dedicated
     * HEVC Main10 + BT.2020 output path that:
     *  - Encodes with libx265 at 10-bit (AV_PIX_FMT_YUV420P10LE).
     *  - Tags the stream with BT.2020 primaries, BT.2020 non-constant luminance
     *    matrix, and SMPTE ST.2084 (PQ) transfer.
     *  - Converts incoming 8-bit sRGB RGBA shader output into PQ-encoded
     *    10-bit YUV, placing SDR-range content at the 100-nit reference level
     *    inside the PQ signal (SDR-in-HDR-container).
     *  - Copies @ref mastering_display and @ref content_light side data from
     *    the input stream when provided, so player HDR metadata is preserved.
     *
     * This mode is intended for use when the *input* video is HDR; the 8-bit
     * GL pipeline cannot reconstruct the original highlight precision, but the
     * resulting file is a correctly-tagged HDR container.
     */
    struct HdrInfo {
      bool enabled = false;    ///< Enables the HDR output path.
      int color_primaries = 0; ///< AVColorPrimaries value.
      int color_trc = 0;       ///< AVColorTransferCharacteristic value.
      int color_space = 0;     ///< AVColorSpace value.
      int color_range = 0;     ///< AVColorRange value.
      /// Raw AVMasteringDisplayMetadata side-data bytes, or empty.
      std::vector<uint8_t> mastering_display;
      /// Raw AVContentLightMetadata side-data bytes, or empty.
      std::vector<uint8_t> content_light;
    } hdr;
  };

  /**
   * @brief FFmpeg-backed RGBA video writer.
   *
   * The writer accepts host RGBA buffers, optional CUDA device buffers, and
   * 16-bit HDR RGBA buffers. It can encode either frame-by-frame or from
   * timestamped frames, depending on the open mode.
   */
  class Writer {
  public:
    /** @brief Construct a closed writer. */
    Writer() = default;

    /**
     * @brief Open an output file using the legacy CRF string interface.
     * @param filename Output file path.
     * @param width Output width in pixels.
     * @param height Output height in pixels.
     * @param fps Output frame rate.
     * @param crf Constant Rate Factor as a string.
     * @return true on success.
     */
    bool open(const std::string &filename, int width, int height, float fps,
              const char *crf);
    /**
     * @brief Open an output file using explicit encoder options.
     * @param filename Output file path.
     * @param width Output width in pixels.
     * @param height Output height in pixels.
     * @param fps Output frame rate.
     * @param opts Encoder configuration.
     * @return true on success.
     */
    bool open(const std::string &filename, int width, int height, float fps,
              const EncodeOptions &opts);
    /**
     * @brief Queue a host RGBA frame for immediate-mode encoding.
     * @param rgba_buffer Pointer to tightly packed RGBA8 pixels.
     */
    void write(void *rgba_buffer);
    /**
     * @brief Queue a host RGBA frame with an explicit presentation timestamp.
     * @param rgba_buffer Pointer to tightly packed RGBA8 pixels.
     * @param pts Presentation timestamp in units of the configured frame time
     * base.
     */
    void write_at_pts(void *rgba_buffer, int64_t pts);
    /**
     * @brief Write a 16-bit RGBA frame that is already PQ- or HLG-encoded in
     *        BT.2020 primaries (8 bytes/pixel: R16,G16,B16,A16, little-endian
     *        unsigned normalised).
     *
     * The data is expected to originate from the HDR GPU encode pass: it is
     * BT.2020 primaries with a non-linear PQ (or HLG) transfer already
     * applied, so this path skips colour-space conversion and only performs
     *   (a) the BT.2020 non-constant-luminance RGB'->YCbCr' matrix, and
     *   (b) 16-bit -> 10-bit limited-range scaling,
     * producing AV_PIX_FMT_YUV420P10LE for libx265 Main10. Requires the
     * writer to have been opened with @ref EncodeOptions::HdrInfo::enabled.
     * @param rgba16_buffer Pointer to tightly packed RGBA16 pixels.
     */
    void write_hdr_rgba16(void *rgba16_buffer);
    /**
     * @brief Queue a 16-bit HDR RGBA frame with an explicit presentation
     * timestamp.
     * @param rgba16_buffer Pointer to tightly packed RGBA16 pixels.
     * @param pts Presentation timestamp in units of the configured frame time
     * base.
     */
    void write_hdr_rgba16_at_pts(void *rgba16_buffer, int64_t pts);
    /**
     * @brief Queue a CUDA RGBA frame for encoding.
     * @param cuda_rgba_buffer CUDA device pointer.
     * @param src_stride Source row pitch in bytes.
     * @param bottom_up Whether the source is stored bottom-up.
     * @return true if the frame was accepted.
     */
    bool write_cuda_rgba(void *cuda_rgba_buffer, int src_stride,
                         bool bottom_up = false);
    /**
     * @brief Queue a CUDA RGBA frame with an explicit presentation timestamp.
     * @param cuda_rgba_buffer CUDA device pointer.
     * @param src_stride Source row pitch in bytes.
     * @param pts Presentation timestamp in units of the configured frame time
     * base.
     * @param bottom_up Whether the source is stored bottom-up.
     * @return true if the frame was accepted.
     */
    bool write_cuda_rgba_at_pts(void *cuda_rgba_buffer, int src_stride,
                                int64_t pts, bool bottom_up = false);
    /**
     * @brief Open a timestamp-based output stream using the legacy CRF string
     * interface.
     * @param filename Output file path.
     * @param width Output width in pixels.
     * @param height Output height in pixels.
     * @param fps Nominal input frame rate.
     * @param crf Constant Rate Factor as a string.
     * @return true on success.
     */
    bool open_ts(const std::string &filename, int width, int height, float fps,
                 const char *crf);
    /**
     * @brief Open a timestamp-based output stream using explicit encoder
     * options.
     * @param filename Output file path.
     * @param width Output width in pixels.
     * @param height Output height in pixels.
     * @param fps Nominal input frame rate.
     * @param opts Encoder configuration.
     * @return true on success.
     */
    bool open_ts(const std::string &filename, int width, int height, float fps,
                 const EncodeOptions &opts);
    /**
     * @brief Queue a host RGBA frame using capture timestamps.
     * @param rgba_buffer Pointer to tightly packed RGBA8 pixels.
     */
    void write_ts(void *rgba_buffer);
    /** @brief Close the writer and flush pending packets. */
    void close();
    /** @brief Check whether the writer is currently open. */
    bool is_open() const { return opened; }
    /// @brief True when FFmpeg identifies the active encoder as hardware or
    /// hybrid.
    bool is_hardware_encode() const { return active_encoder_hardware; }
    /// @brief If true, keep one pending frame and pace producer threads to the
    /// encoder instead of dropping frames. Intended for headless/batch
    /// transcoding where every input frame must reach the output. Default:
    /// false.
    void set_block_when_full(bool value) { block_when_full = value; }
    /** @brief Check whether the encoder queue blocks instead of dropping
     * frames. */
    bool get_block_when_full() const { return block_when_full; }
    /**
     * @brief Return the output timeline length in nominal frame ticks.
     *
     * For sequential writes this equals the submitted frame count. Explicit
     * PTS writes may leave gaps, in which case it is the highest accepted PTS
     * plus one.
     */
    int64_t get_frame_count() const { return frame_count; }
    /** @brief Return the encoded duration in seconds. */
    double get_duration() const;
    /** @brief Close the writer on destruction if it is still open. */
    ~Writer() {
      if (is_open()) {
        close();
        opened = false;
      }
    }

  private:
    bool opened{false}; ///< Internal open-state flag.
    int width = 0;      ///< Output width in pixels.
    int height = 0;     ///< Output height in pixels.
    int fps_num = 0;    ///< Output FPS numerator.
    int fps_den = 0;    ///< Output FPS denominator.
    int64_t frame_count =
        0; ///< Next sequential PTS / explicit-PTS timeline length.
    double last_duration = 0.0; ///< Cached duration from the last encode step.
    AVFormatContext *format_ctx = nullptr; ///< Active container context.
    AVCodecContext *codec_ctx = nullptr;   ///< Active codec context.
    AVStream *stream = nullptr;            ///< Output video stream.
    AVFrame *frameYUV = nullptr;           ///< Software-converted YUV frame.
    AVFrame *frameRGBA = nullptr;          ///< Staging RGBA frame.
    AVFrame *frame10 = nullptr; ///< YUV420P10LE frame used for HDR output.
    AVFrame *upload_sw_frame =
        nullptr; ///< Software upload frame used by CUDA/hardware paths.
    AVBufferRef *hw_device_ctx =
        nullptr; ///< Hardware device context, when available.
    AVBufferRef *hw_frames_ctx =
        nullptr;                ///< Hardware frames pool, when available.
    bool use_hw_encode = false; ///< True when hardware encoding is active.
    bool active_encoder_hardware =
        false; ///< True when the selected FFmpeg encoder is hardware-backed.
    bool direct_cuda_upload =
        false; ///< True when CUDA RGBA frames can be copied without conversion.
#ifdef MXWRITE_HAS_CUDA_COPY
    // Dedicated stream so the producer's RGBA→hwframe copy does not serialise
    // with the renderer's default-stream work or with the encoder thread.
    cudaStream_t cuda_upload_stream = nullptr;
#endif
    bool hdr_output =
        false; ///< True when HDR (HEVC Main10/PQ) output is active.
    EncodeOptions::HdrInfo hdr_info; ///< HDR metadata captured at open() time.
    SwsContext *sws_ctx = nullptr;   ///< Frame conversion context.
    AVRational time_base;            ///< Stream time base.
    /** @brief Convert a frame rate into a rational numerator/denominator pair.
     */
    void calculateFPSFraction(float fps, int &fps_num, int &fps_den);
    std::chrono::steady_clock::time_point
        recordingStart; ///< Start time for timestamp mode.

    std::queue<AVFrame *> encode_queue; ///< Pending encoded frames.
    // Deep enough to absorb encoder hiccups (~4s at 30fps, ~2s at 60fps).
    // Memory cost is bounded by the NVENC frame pool / sw RGBA frame buffer.
    static constexpr size_t MAX_QUEUE_SIZE = 120;
    // No-drop mode intentionally keeps only one pending frame. Together with
    // the frame currently being encoded, this paces file processing to encoder
    // throughput instead of producing 120-frame burst/stall cycles.
    static constexpr size_t NO_DROP_QUEUE_SIZE = 1;
    std::condition_variable queue_cv; ///< Signals queue availability.
    std::jthread encode_thread;       ///< Background encoder thread.

    std::mutex queue_mutex{};    ///< Guards the frame queue.
    std::mutex writer_mutex{};   ///< Guards writer state transitions.
    bool stop_requested = false; ///< Signals encoder shutdown.
    std::atomic<bool> block_when_full{false}; ///< Queue backpressure mode.

    /** @brief Shared implementation for open() and open_ts(). */
    bool openInternal(const std::string &filename, int w, int h, float fps,
                      const EncodeOptions &opts, bool ts_mode);
    /** @brief Initialize an FFmpeg hardware device and frame pool for an
     * encoder. */
    bool initHardwareEncoding(const AVCodec *codec,
                              AVPixelFormat requested_format,
                              bool prefer_cuda_rgba);
    /** @brief Start the background encoder thread. */
    void startEncoderThread();
    /** @brief Stop the background encoder thread. */
    void stopEncoderThread();
    /** @brief Encoder thread main loop. */
    void encodeLoop(std::stop_token stop_token);
    /** @brief Encode and write one frame. */
    void encodeAndWriteFrame(AVFrame *in_frame);
    /** @brief Drain packets from the codec into the container. */
    void drainEncoderPackets();
    /** @brief Release a frame allocated for the encode queue. */
    void releaseFrame(AVFrame *f);
  };

  /**
   * @brief Copy audio from one video file to another.
   * @param sourceAudioFile Input media file containing the audio stream.
   * @param destVideoFile Output video file to receive the audio stream.
   */
  extern bool transfer_audio(std::string_view sourceAudioFile,
                             std::string_view destVideoFile);
  /**
   * @brief Free FFmpeg format contexts used during transfer operations.
   * @param source_ctx Source format context.
   * @param dest_ctx Destination format context.
   * @param output_ctx Output format context.
   */
  extern void cleanup_contexts(AVFormatContext * source_ctx,
                               AVFormatContext * dest_ctx,
                               AVFormatContext * output_ctx);
}

#endif
