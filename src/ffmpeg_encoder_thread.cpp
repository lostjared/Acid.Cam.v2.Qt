/*
 * Acid Cam v2 - Qt/OpenCV Edition
 * FFmpeg Encoder Thread Implementation - Background Video Encoding
 * written by Jared Bruni ( http://lostsidedead.com )
 * (C) 2017 GPL
 */

#include "ffmpeg_encoder_thread.h"
#include <algorithm>
#include <iostream>

namespace {
bool parseResolution(const std::string &resolution, int &width, int &height) {
    const auto separator = resolution.find('x');
    if(separator == std::string::npos)
        return false;
    try {
        width = std::stoi(resolution.substr(0, separator));
        height = std::stoi(resolution.substr(separator + 1));
    } catch(const std::exception &) {
        return false;
    }
    return width > 0 && height > 0;
}
}

FFmpegEncoderThread::FFmpegEncoderThread(QObject *parent)
    : QThread(parent), stop_encoding(false), is_encoding(false),
      output_width(0), output_height(0),
      timestamped_input(false), block_when_full(false), frames_encoded(0) {
}

FFmpegEncoderThread::~FFmpegEncoderThread() {
    stopEncoding();
    wait();  // Wait for thread to finish
}

bool FFmpegEncoderThread::startEncoding(const std::string &output,
                                       const std::string &src_res, const std::string &dst_res,
                                       double fps, const FFmpegEncodeOptions &options) {
    (void)src_res;
    if (is_encoding) {
        emit encodingError(QString::fromStdString("Encoding already in progress"));
        return false;
    }

    // A previous run may have cleared is_encoding immediately before returning.
    if (isRunning()) {
        wait();
    }

    {
        QMutexLocker locker(&queue_mutex);
        std::queue<cv::Mat> empty_queue;
        frame_queue.swap(empty_queue);
    }
    
    if(!parseResolution(dst_res, output_width, output_height)) {
        emit encodingError(QString::fromStdString("Invalid output resolution"));
        return false;
    }

    mx::EncodeOptions mxOptions;
    mxOptions.codec = options.codec;
    mxOptions.crf = std::clamp(options.quality, 0, 51);
    mxOptions.preset = options.preset;
    mxOptions.tune = options.tune == "none" ? "" : options.tune;
    mxOptions.ffmpeg_options = options.ffmpegOptions;
    mxOptions.realtime = options.realtime;
    mxOptions.block_when_full = options.blockWhenFull;
    timestamped_input = options.timestampInput;
    block_when_full = options.blockWhenFull;
    frames_encoded = 0;

    const bool opened = timestamped_input
                            ? video_writer.open_ts(output, output_width,
                                                   output_height, fps, mxOptions)
                            : video_writer.open(output, output_width,
                                                output_height, fps, mxOptions);
    if(!opened) {
        emit encodingError(QString::fromStdString(
            "MXWrite failed to open the output video"));
        return false;
    }
    
    stop_encoding = false;
    is_encoding = true;
    
    // Start the encoding thread
    start();
    emit encodingStarted();
    
    return true;
}

void FFmpegEncoderThread::enqueueFrame(const cv::Mat &frame) {
    if (frame.empty()) {
        return;
    }

    if(block_when_full) {
        // MXWrite's no-drop queue has exactly one pending slot. Submit from
        // the playback/processing thread so its write() call provides the
        // backpressure directly; do not build a second Qt-side frame queue.
        if(is_encoding && !stop_encoding)
            writeFrame(frame);
        return;
    }

    QMutexLocker locker(&queue_mutex);
    if(!is_encoding || stop_encoding)
        return;
    // Bound memory use if the selected encoder cannot keep up with playback.
    const size_t queueLimit = timestamped_input ? 2 : max_queued_frames;
    if(frame_queue.size() >= queueLimit) {
        frame_queue.pop();
    }
    frame_queue.push(frame.clone());
    queue_condition.wakeOne();  // Signal that a frame is available
}

void FFmpegEncoderThread::stopEncoding() {
    if (!is_encoding) {
        return;
    }
    
    {
        QMutexLocker locker(&queue_mutex);
        stop_encoding = true;
        queue_condition.wakeOne();  // Wake thread to check stop condition
    }
    
    // Wait for thread to process remaining frames and exit
    if (!wait(5000)) {  // Report slow shutdown, but never kill a C++ thread asynchronously.
        std::cerr << "acidcam: MXWrite encoder thread did not stop gracefully\n";
        wait();
    }
}

size_t FFmpegEncoderThread::getQueueSize() {
    QMutexLocker locker(&queue_mutex);
    return frame_queue.size();
}

void FFmpegEncoderThread::run() {
    while (true) {
        cv::Mat frame;
        
        {
            QMutexLocker locker(&queue_mutex);
            
            // Wait for a frame or stop signal
            while (frame_queue.empty() && !stop_encoding) {
                if(!queue_condition.wait(&queue_mutex, 100))
                    break;
            }
            
            // Check if we should stop
            if (frame_queue.empty() && stop_encoding) {
                break;
            }
            
            // Get frame from queue
            if (!frame_queue.empty()) {
                frame = frame_queue.front();
                frame_queue.pop();
                queue_condition.wakeAll();
            }
        }

        writeFrame(frame);
    }
    
    if(video_writer.is_open())
        video_writer.close();
    
    is_encoding = false;
    emit encodingStopped();
}

void FFmpegEncoderThread::writeFrame(const cv::Mat &frame) {
    if(frame.empty() || !video_writer.is_open())
        return;

    cv::Mat sizedFrame;
    if(frame.cols != output_width || frame.rows != output_height)
        cv::resize(frame, sizedFrame, cv::Size(output_width, output_height));
    else
        sizedFrame = frame;

    cv::Mat rgbaFrame;
    if(sizedFrame.channels() == 4)
        cv::cvtColor(sizedFrame, rgbaFrame, cv::COLOR_BGRA2RGBA);
    else if(sizedFrame.channels() == 3)
        cv::cvtColor(sizedFrame, rgbaFrame, cv::COLOR_BGR2RGBA);
    else
        cv::cvtColor(sizedFrame, rgbaFrame, cv::COLOR_GRAY2RGBA);

    if(timestamped_input)
        video_writer.write_ts(rgbaFrame.data);
    else
        video_writer.write(rgbaFrame.data);

    const int processed = ++frames_encoded;
    if(processed % 30 == 0)
        emit framesProcessed(processed);
}
