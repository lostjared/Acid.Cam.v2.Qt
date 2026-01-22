/*
 * Acid Cam v2 - Qt/OpenCV Edition
 * written by Jared Bruni ( http://lostsidedead.com )
 * (C) 2017 GPL
 */

#ifndef _FFMPEG_WRITE_H_
#define _FFMPEG_WRITE_H_

#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <opencv2/opencv.hpp>
#include <sstream>

enum class FFmpegCodec {
    LIBX264,        // CPU H.264 encoder
    LIBX265,        // CPU H.265/HEVC encoder
    H264_NVENC,     // NVIDIA GPU H.264 encoder
    HEVC_NVENC,     // NVIDIA GPU H.265/HEVC encoder
    H264_VAAPI,     // Intel/AMD VAAPI H.264 encoder
    HEVC_VAAPI,     // Intel/AMD VAAPI H.265 encoder
    CODEC_COUNT
};

const char* getCodecName(FFmpegCodec codec);
const char* getCodecDescription(FFmpegCodec codec);
FILE* ffmpeg_open(const std::string &output, FFmpegCodec codec,
                  const std::string &src_res, const std::string &dst_res,
                  double fps, int crf);
void ffmpeg_write_frame(FILE *fptr, const cv::Mat &frame);
void ffmpeg_close(FILE *fptr);
bool ffmpeg_mux_audio(const std::string &temp_video, const std::string &source,
                      const std::string &output);
bool ffmpeg_check_nvenc();
bool ffmpeg_check_vaapi();

#endif
