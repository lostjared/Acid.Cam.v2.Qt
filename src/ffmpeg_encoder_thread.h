/*
 * Acid Cam v2 - Qt/OpenCV Edition
 * FFmpeg Encoder Thread - Background Video Encoding
 * written by Jared Bruni ( http://lostsidedead.com )
 * (C) 2017 GPL
 */

#ifndef _FFMPEG_ENCODER_THREAD_H_
#define _FFMPEG_ENCODER_THREAD_H_

#include "qtheaders.h"
#include "ffmpeg_write.h"
#include <opencv2/opencv.hpp>
#include <queue>
#include <atomic>
#include <memory>

class FFmpegEncoderThread : public QThread {
    Q_OBJECT
    
private:
    std::queue<cv::Mat> frame_queue;
    QMutex queue_mutex;
    QWaitCondition queue_condition;
    std::atomic<bool> stop_encoding;
    std::atomic<bool> is_encoding;
    FILE *ffmpeg_pipe;
    
public:
    FFmpegEncoderThread(QObject *parent = nullptr);
    ~FFmpegEncoderThread();
    
    // Start encoding to output file
    bool startEncoding(const std::string &output, FFmpegCodec codec,
                      const std::string &src_res, const std::string &dst_res,
                      double fps, int crf);
    
    // Queue a frame for encoding
    void enqueueFrame(const cv::Mat &frame);
    
    // Stop encoding and close pipe
    void stopEncoding();
    
    // Check if currently encoding
    bool isEncoding() const { return is_encoding; }
    
    // Get queue size for monitoring
    size_t getQueueSize();
    
protected:
    void run() override;
    
signals:
    void encodingStarted();
    void encodingStopped();
    void encodingError(const QString &error);
    void framesProcessed(int count);
};

#endif
