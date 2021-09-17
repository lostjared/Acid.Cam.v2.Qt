
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
    std::atomic<double> frame_rate;
    std::atomic<bool> recording;
    cv::VideoCapture capture;
    cv::VideoWriter  writer;
    cv::Mat rgb_frame;
    QImage img;
    std::vector<FilterValue> current;
    std::atomic<bool> isPaused, isStep;
    std::atomic<bool> record_png;
    std::atomic<int> png_index;
    std::string png_path;
    VideoMode mode;
    std::atomic<int> device_num;
    unsigned long *frame_index;
    std::atomic<unsigned int> red, green, blue;
    std::atomic<unsigned int> bright_, gamma_, saturation_;
    std::atomic<bool> single_mode;
    FilterValue current_filter, prev_filter;
    std::atomic<double> alpha;
    std::atomic<bool> flip_frame1, flip_frame2;
    std::atomic<bool> repeat_video;
    std::atomic<bool> fadefilter;
    std::vector<cv::Mat> cycle_values;
    std::atomic<int> cycle_on;
    std::atomic<int> cycle_index;
    std::atomic<int> frame_num;
    std::atomic<bool> _custom_cycle;
    std::atomic<int> _custom_cycle_index;
    std::atomic<int> fps_delay;
    std::vector<std::string> draw_strings;
    std::unordered_map<std::string, FilterValue> filter_map_ex;
public:
    Playback(QObject *parent = 0);
    ~Playback();
    void setFadeFilter(bool f);
    void setFrameIndex(const long &index);
    bool getFrame(QImage &img, const int &index);
    bool getProgramMode();
    void setRGB(int r, int g, int b);
    void setColorOptions(int b, int g, int s);
    void setAlpha(int a);
    void setProcMode(int p);
    void setMaxAlloc(int a);
    unsigned int getObjectSize();
    unsigned long allocatedFrames();
    unsigned long getMaxAlloc();
    void setWaitColorLevel(int c, int l);
    void setColorMap(int c);
    void setPngPath(std::string path);
    void clearImage();
    void Play();
    void Stop();
    void Release();
    bool VideoRelease();
    void SetFlip(bool f1, bool f2);
    void setVideo(cv::VideoCapture cap, cv::VideoWriter writer, bool record, bool record_png);
    bool setVideoCamera(std::string name, int type, int device, int res, cv::VideoWriter writer, bool record);
    bool openVideo(std::string video);
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
    void enableRepeat(bool re);
    void setCycle(int type, int frame_skip, std::vector<std::string> &val);
    void setCycle(int type);
    void setPref(int thread_count, int intense);
    unsigned long calcMem();
    void setCustomCycle(bool b);
    void setCustomCycleDelay(int delay);
signals:
    void procImage(const QImage image);
    void stopRecording();
    void frameIncrement();
    void resetIndex();
};

#endif
