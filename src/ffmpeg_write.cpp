/*
 * Acid Cam v2 - Qt/OpenCV Edition
 * written by Jared Bruni ( http://lostsidedead.com )
 * (C) 2017 GPL
 */

#include "ffmpeg_write.h"
#include <algorithm>
#include <cctype>

#ifndef _WIN32
#include <unistd.h>
#endif

static const std::string ffmpeg_path = "ffmpeg";

namespace {
std::string lowercase(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char ch) {
                       return static_cast<char>(std::tolower(ch));
                   });
    return value;
}

bool isSoftwarePreset(const std::string &preset) {
    static const char *presets[] = {"ultrafast", "superfast", "veryfast",
                                    "faster",    "fast",      "medium",
                                    "slow",      "slower",    "veryslow"};
    return std::find(std::begin(presets), std::end(presets), preset) !=
           std::end(presets);
}

std::string softwarePreset(std::string preset) {
    preset = lowercase(preset);
    if(preset == "p1") return "ultrafast";
    if(preset == "p2") return "superfast";
    if(preset == "p3") return "veryfast";
    if(preset == "p4") return "fast";
    if(preset == "p5") return "medium";
    if(preset == "p6") return "slow";
    if(preset == "p7") return "veryslow";
    return isSoftwarePreset(preset) ? preset : "medium";
}

std::string nvencPreset(std::string preset) {
    preset = lowercase(preset);
    if(preset.size() == 2 && preset[0] == 'p' && preset[1] >= '1' &&
       preset[1] <= '7')
        return preset;
    if(preset == "ultrafast") return "p1";
    if(preset == "superfast") return "p2";
    if(preset == "veryfast" || preset == "faster") return "p3";
    if(preset == "fast") return "p4";
    if(preset == "slow" || preset == "slower") return "p6";
    if(preset == "veryslow") return "p7";
    return "p5";
}

bool isSoftwareTune(const std::string &tune) {
    static const char *tunes[] = {"film",       "animation", "grain",
                                  "stillimage", "psnr",      "ssim",
                                  "fastdecode", "zerolatency"};
    return std::find(std::begin(tunes), std::end(tunes), tune) !=
           std::end(tunes);
}

bool isNvencTune(const std::string &tune) {
    return tune == "hq" || tune == "uhq" || tune == "ll" ||
           tune == "ull" || tune == "lossless";
}

std::string softwareTune(std::string tune, bool realtime) {
    if(realtime) return "zerolatency";
    tune = lowercase(tune);
    if(tune == "ll" || tune == "ull") return "zerolatency";
    return isSoftwareTune(tune) ? tune : "";
}
}

const char* getCodecName(FFmpegCodec codec) {
    switch (codec) {
        case FFmpegCodec::LIBX264:    return "libx264";
        case FFmpegCodec::LIBX265:    return "libx265";
        case FFmpegCodec::H264_NVENC: return "h264_nvenc";
        case FFmpegCodec::HEVC_NVENC: return "hevc_nvenc";
        case FFmpegCodec::H264_VAAPI: return "h264_vaapi";
        case FFmpegCodec::HEVC_VAAPI: return "hevc_vaapi";
        default:                      return "libx264";
    }
}

const char* getCodecDescription(FFmpegCodec codec) {
    switch (codec) {
        case FFmpegCodec::LIBX264:    return "H.264 (CPU - libx264)";
        case FFmpegCodec::LIBX265:    return "H.265/HEVC (CPU - libx265)";
        case FFmpegCodec::H264_NVENC: return "H.264 (NVIDIA GPU - NVENC)";
        case FFmpegCodec::HEVC_NVENC: return "H.265/HEVC (NVIDIA GPU - NVENC)";
        case FFmpegCodec::H264_VAAPI: return "H.264 (Intel/AMD - VAAPI)";
        case FFmpegCodec::HEVC_VAAPI: return "H.265/HEVC (Intel/AMD - VAAPI)";
        default:                      return "Unknown";
    }
}

FILE* ffmpeg_open(const std::string &output, FFmpegCodec codec,
                  const std::string &src_res, const std::string &dst_res,
                  double fps, const FFmpegEncodeOptions &options,
                  const std::string &diagnosticLogPath) {
    
    const char* codec_name = getCodecName(codec);
    const int quality = std::max(0, std::min(options.quality, 51));
    const std::string requested_tune = lowercase(options.tune);
    
    std::ostringstream cmd;
    cmd << ffmpeg_path 
        << " -y"
        << " -s " << src_res
        << " -pixel_format bgr24"
        << " -f rawvideo"
        << " -r " << fps;
    cmd << " -i pipe:"
        << (options.timestampInput
                ? " -vf \"setpts=(RTCTIME-RTCSTART)/(TB*1000000)\" -fps_mode vfr"
                : " -fps_mode cfr")
        << " -vcodec " << codec_name
        << " -pix_fmt yuv420p";
    
    switch (codec) {
        case FFmpegCodec::LIBX264:
        case FFmpegCodec::LIBX265:
            cmd << " -preset " << softwarePreset(options.preset);
            if(requested_tune == "lossless") {
                if(codec == FFmpegCodec::LIBX265)
                    cmd << " -x265-params lossless=1";
                else
                    cmd << " -qp 0";
            } else {
                const std::string tune =
                    softwareTune(requested_tune, options.realtime);
                if(!tune.empty()) cmd << " -tune " << tune;
                cmd << " -crf " << quality;
            }
            if(options.realtime) cmd << " -bf 0";
            if (codec == FFmpegCodec::LIBX265) {
                cmd << " -tag:v hvc1";
            }
            break;
            
        case FFmpegCodec::H264_NVENC:
        case FFmpegCodec::HEVC_NVENC:
            cmd << " -preset " << nvencPreset(options.preset);
            if(requested_tune == "lossless") {
                cmd << " -tune lossless";
            } else if(options.realtime) {
                cmd << " -tune ll -bf 0";
            } else if(isNvencTune(requested_tune)) {
                cmd << " -tune " << requested_tune;
            }
            if(requested_tune != "lossless")
                cmd << " -cq " << quality << " -b:v 0";
            if (codec == FFmpegCodec::HEVC_NVENC) {
                cmd << " -tag:v hvc1";
            }
            break;
            
        case FFmpegCodec::H264_VAAPI:
        case FFmpegCodec::HEVC_VAAPI:
            cmd << " -vaapi_device /dev/dri/renderD128";
            cmd << " -qp " << quality;
            if(options.realtime) cmd << " -bf 0";
            if (codec == FFmpegCodec::HEVC_VAAPI) {
                cmd << " -tag:v hvc1";
            }
            break;
            
        default:
            cmd << " -crf " << quality;
            break;
    }
    
    if (src_res != dst_res) {
        cmd << " -s " << dst_res;
    }
    
    // Output file
    cmd << " \"" << output << "\"";
    if(!diagnosticLogPath.empty())
        cmd << " > \"" << diagnosticLogPath << "\" 2>&1";
    
    std::cout << "acidcam: Starting FFmpeg: " << cmd.str() << "\n";
    
#ifndef _WIN32
    FILE *fptr = popen(cmd.str().c_str(), "w");
#else
    FILE *fptr = _popen(cmd.str().c_str(), "wb");
#endif

    if (!fptr) {
        std::cerr << "acidcam: Error: Could not open FFmpeg pipe\n";
        return nullptr;
    }
    return fptr;
}

void ffmpeg_write_frame(FILE *fptr, const cv::Mat &frame) {
    if (fptr && !frame.empty()) {
        fwrite(frame.ptr(), sizeof(char), frame.total() * frame.elemSize(), fptr);
        fflush(fptr);
    }
}

void ffmpeg_close(FILE *fptr) {
    if (fptr) {
#ifndef _WIN32
        pclose(fptr);
#else
        _pclose(fptr);
#endif
    }
}

bool ffmpeg_mux_audio(const std::string &temp_video, const std::string &source,
                      const std::string &output,
                      const FFmpegLogCallback &logCallback) {
    std::ostringstream cmd;
    cmd << ffmpeg_path
        << " -y -i \"" << temp_video << "\""
        << " -i \"" << source << "\""
        << " -c copy"
        << " -map 0:v:0"
        << " -map 1:a:0?"  // Optional audio track
        << " -shortest"
        << " \"" << output << "\""
        << " 2>&1";
    
    std::cout << "acidcam: Muxing audio: " << cmd.str() << "\n";
    
#ifndef _WIN32
    FILE *fptr = popen(cmd.str().c_str(), "r");
#else
    FILE *fptr = _popen(cmd.str().c_str(), "r");
#endif
    
    if (!fptr) {
        std::cerr << "acidcam: Error: Could not mux audio\n";
        return false;
    }
    
    char buffer[4096];
    while(fgets(buffer, sizeof(buffer), fptr) != nullptr) {
        std::cerr << buffer;
        if(logCallback)
            logCallback(buffer);
    }

#ifndef _WIN32
    int status = pclose(fptr);
    return status != -1 && WIFEXITED(status) && WEXITSTATUS(status) == 0;
#else
    return _pclose(fptr) == 0;
#endif
}

bool ffmpeg_check_nvenc() {
    std::string cmd = ffmpeg_path + " -hide_banner -encoders 2>/dev/null | grep -q h264_nvenc";
#ifndef _WIN32
    int result = system(cmd.c_str());
    return WEXITSTATUS(result) == 0;
#else
    // On Windows, assume NVENC is available if NVIDIA GPU present
    return true;
#endif
}

bool ffmpeg_check_vaapi() {
    std::string cmd = ffmpeg_path + " -hide_banner -encoders 2>/dev/null | grep -q h264_vaapi";
#ifndef _WIN32
    int result = system(cmd.c_str());
    return WEXITSTATUS(result) == 0;
#else
    return false;  // VAAPI is Linux-only
#endif
}
