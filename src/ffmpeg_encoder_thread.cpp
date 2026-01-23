/*
 * Acid Cam v2 - Qt/OpenCV Edition
 * FFmpeg Encoder Thread Implementation - Background Video Encoding
 * written by Jared Bruni ( http://lostsidedead.com )
 * (C) 2017 GPL
 */

#include "ffmpeg_encoder_thread.h"
#include <iostream>

FFmpegEncoderThread::FFmpegEncoderThread(QObject *parent)
    : QThread(parent), stop_encoding(false), is_encoding(false), ffmpeg_pipe(nullptr) {
}

FFmpegEncoderThread::~FFmpegEncoderThread() {
    stopEncoding();
    wait();  // Wait for thread to finish
}

bool FFmpegEncoderThread::startEncoding(const std::string &output, FFmpegCodec codec,
                                       const std::string &src_res, const std::string &dst_res,
                                       double fps, int crf) {
    if (is_encoding) {
        emit encodingError(QString::fromStdString("Encoding already in progress"));
        return false;
    }
    
    // Open FFmpeg pipe
    ffmpeg_pipe = ffmpeg_open(output, codec, src_res, dst_res, fps, crf);
    if (!ffmpeg_pipe) {
        emit encodingError(QString::fromStdString("Failed to open FFmpeg pipe"));
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
    if (!is_encoding) {
        return;
    }
    
    // Make a copy to avoid issues with frame reuse
    cv::Mat frame_copy = frame.clone();
    
    QMutexLocker locker(&queue_mutex);
    frame_queue.push(frame_copy);
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
    if (!wait(5000)) {  // 5 second timeout
        std::cerr << "acidcam: FFmpeg encoder thread did not stop gracefully\n";
        terminate();
        wait();
    }
}

size_t FFmpegEncoderThread::getQueueSize() {
    QMutexLocker locker(&queue_mutex);
    return frame_queue.size();
}

void FFmpegEncoderThread::run() {
    int frames_encoded = 0;
    
    while (true) {
        cv::Mat frame;
        
        {
            QMutexLocker locker(&queue_mutex);
            
            // Wait for a frame or stop signal
            while (frame_queue.empty() && !stop_encoding) {
                queue_condition.wait(&queue_mutex);
            }
            
            // Check if we should stop
            if (frame_queue.empty() && stop_encoding) {
                break;
            }
            
            // Get frame from queue
            if (!frame_queue.empty()) {
                frame = frame_queue.front();
                frame_queue.pop();
            }
        }
        
        // Encode frame outside of mutex lock
        if (!frame.empty() && ffmpeg_pipe) {
            ffmpeg_write_frame(ffmpeg_pipe, frame);
            ++frames_encoded;
            
            // Emit progress signal periodically (every 30 frames)
            if (frames_encoded % 30 == 0) {
                emit framesProcessed(frames_encoded);
            }
        }
    }
    
    // Close the FFmpeg pipe
    if (ffmpeg_pipe) {
        ffmpeg_close(ffmpeg_pipe);
        ffmpeg_pipe = nullptr;
    }
    
    is_encoding = false;
    emit encodingStopped();
}
