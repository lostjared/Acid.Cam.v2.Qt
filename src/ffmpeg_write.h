/*
 * Acid Cam v2 - Qt/OpenCV Edition
 * written by Jared Bruni ( http://lostsidedead.com )
 * (C) 2017 GPL
 */

#ifndef _FFMPEG_WRITE_H_
#define _FFMPEG_WRITE_H_

#include <iostream>
#include <string>
#include <functional>
#include <opencv2/opencv.hpp>

struct FFmpegEncodeOptions {
    int quality = 23;
    std::string preset = "medium";
    std::string tune = "none";
    std::string codec = "auto";
    std::string ffmpegOptions;
    bool realtime = false;
    bool timestampInput = false;
    bool blockWhenFull = false;
};

using FFmpegLogCallback = std::function<void(const std::string &)>;

bool ffmpeg_mux_audio(const std::string &temp_video, const std::string &source,
                      const std::string &output,
                      const FFmpegLogCallback &logCallback = {});

#endif
