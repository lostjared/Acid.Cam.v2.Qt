#ifndef __PLAYBACK_WINDOW_H__
#define __PLAYBACK_WINDOW_H__

#include "qtheaders.h"
#include<atomic>

enum VideoMode { MODE_CAMERA = 0, MODE_VIDEO };

class Playback : public QThread {
    Q_OBJECT
private:
    std::atomic<bool> stop;
    std::atomic<bool> video_shown;
    QMutex mutex, mutex_shown, mutex_add;
    QWaitCondition condition;
    cv::Mat frame;
    int frame_rate;
    bool recording;
    cv::VideoCapture capture;
    cv::VideoWriter  writer;
    cv::Mat rgb_frame;
    QImage img;
    std::vector<std::pair<int, int>> current;
    bool isPaused, isStep;
    VideoMode mode;
public:
    Playback(QObject *parent = 0);
    ~Playback();
    void Play();
    void Stop();
    void Release();
    void setVideo(cv::VideoCapture cap, cv::VideoWriter writer, bool record);
    bool setVideoCamera(int device, int res, cv::VideoWriter writer, bool record);
    bool isStopped() const;
    void run();
    void msleep(int ms);
    void setVector(std::vector<std::pair<int, int>> s);
    void setOptions(bool n, int c);
    void setImage(const cv::Mat &image);
    void setStep();
    void setDisplayed(bool shown);
signals:
    void procImage(const QImage image);
    void stopRecording();
    void frameIncrement();
    
};

#endif
