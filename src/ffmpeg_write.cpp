/*
 * Acid Cam v2 - Qt/OpenCV Edition
 * written by Jared Bruni ( http://lostsidedead.com )
 * (C) 2017 GPL
 */

#include "ffmpeg_write.h"

#ifndef _WIN32
#include <unistd.h>
#endif

static const std::string ffmpeg_path = "ffmpeg";

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
                  double fps, int crf) {
    
    const char* codec_name = getCodecName(codec);
    
    std::ostringstream cmd;
    cmd << ffmpeg_path 
        << " -y"
        << " -s " << src_res
        << " -pixel_format bgr24"
        << " -f rawvideo"
        << " -r " << fps
        << " -i pipe:"
        << " -vcodec " << codec_name
        << " -pix_fmt yuv420p";
    
    switch (codec) {
        case FFmpegCodec::LIBX264:
        case FFmpegCodec::LIBX265:
            cmd << " -crf " << crf;
            if (codec == FFmpegCodec::LIBX265) {
                cmd << " -tag:v hvc1";
            }
            break;
            
        case FFmpegCodec::H264_NVENC:
        case FFmpegCodec::HEVC_NVENC:
            cmd << " -preset p5";
            cmd << " -cq " << crf;
            cmd << " -b:v 0";
            if (codec == FFmpegCodec::HEVC_NVENC) {
                cmd << " -tag:v hvc1";
            }
            break;
            
        case FFmpegCodec::H264_VAAPI:
        case FFmpegCodec::HEVC_VAAPI:
            cmd << " -vaapi_device /dev/dri/renderD128";
            cmd << " -qp " << crf;
            if (codec == FFmpegCodec::HEVC_VAAPI) {
                cmd << " -tag:v hvc1";
            }
            break;
            
        default:
            cmd << " -crf " << crf;
            break;
    }
    
    if (src_res != dst_res) {
        cmd << " -s " << dst_res;
    }
    
    // Output file
    cmd << " \"" << output << "\"";
    
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
                      const std::string &output) {
    std::ostringstream cmd;
    cmd << ffmpeg_path
        << " -y -i \"" << temp_video << "\""
        << " -i \"" << source << "\""
        << " -c copy"
        << " -map 0:v:0"
        << " -map 1:a:0?"  // Optional audio track
        << " -shortest"
        << " \"" << output << "\"";
    
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
    
    // Wait for completion
#ifndef _WIN32
    int status = pclose(fptr);
    return WEXITSTATUS(status) == 0;
#else
    _pclose(fptr);
    return true;
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
