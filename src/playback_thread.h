
/*
 * Acid Cam v2 - Qt/OpenCV Edition
 * written by Jared Bruni ( http://lostsidedead.com )
 * (C) 2017 GPL
 */

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
    double frame_rate;
    bool recording;
    cv::VideoCapture capture;
    cv::VideoWriter  writer;
    cv::Mat rgb_frame;
    QImage img;
    std::vector<FilterValue> current;
    bool isPaused, isStep;
    VideoMode mode;
    int device_num;
    unsigned long *frame_index;
    unsigned int red, green, blue;
    unsigned int bright_, gamma_, saturation_;
    bool single_mode;
    FilterValue current_filter, prev_filter;
    double alpha;
    bool flip_frame1, flip_frame2;
    bool repeat_video;
    bool fadefilter;
    std::vector<cv::Mat> cycle_values;
    int cycle_on;
    int cycle_index;
    int frame_num;
    bool _custom_cycle;
    int _custom_cycle_index;
    int fps_delay;
public:
    Playback(QObject *parent = 0);
    ~Playback();
    void setFadeFilter(bool f);
    void setFrameIndex(const long &index);
    bool getFrame(QImage &img, const int &index);
    void setRGB(int r, int g, int b);
    void setColorOptions(int b, int g, int s);
    void setColorMap(int c);
    void Play();
    void Stop();
    void Release();
    void SetFlip(bool f1, bool f2);
    void setVideo(cv::VideoCapture cap, cv::VideoWriter writer, bool record);
    bool setVideoCamera(int device, int res, cv::VideoWriter writer, bool record);
    bool isStopped() const;
    void run();
    void Clear();
    void msleep(int ms);
    void setVector(std::vector<FilterValue> s);
    void setOptions(bool n, int c);
    void setImage(const cv::Mat &image);
    void setColorKey(const cv::Mat &image);
    void setStep();
    void setDisplayed(bool shown);
    void setIndexChanged(std::string name);
    void setSingleMode(bool val);
    void drawFilter(cv::Mat &frame, FilterValue &filter);
    void drawEffects(cv::Mat &frame);
    void filterFade(cv::Mat &frame, FilterValue &filter1, FilterValue &filter2, double alpha);
    void reset_filters();
    void setSubFilter(int index);
    void enableRepeat(bool re);
    void setCycle(int type, int frame_skip, std::vector<std::string> &val);
    void setCycle(int type);
    void setPref(int thread_count, int intense);
    void setCustomCycle(bool b);
    void setCustomCycleDelay(int delay);
signals:
    void procImage(const QImage image);
    void stopRecording();
    void frameIncrement();
    void resetIndex();
    
};

#endif
