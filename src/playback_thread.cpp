/*
 * Acid Cam v2 - Qt/OpenCV Edition
 * written by Jared Bruni ( http://lostsidedead.com )
 * (C) 2017 GPL
 */


#include"playback_thread.h"

Playback::Playback(QObject *parent) : QThread(parent) {
    stop = true;
    isStep = false;
    isPaused = false;
    bright_ = gamma_ = saturation_ = 0;
    single_mode = true;
    alpha = 0;
    prev_filter = FilterValue(0, 0, -1);
    flip_frame1 = false;
    flip_frame2 = false;
    repeat_video = false;
    fadefilter = true;
}

void Playback::Play() {
    if(!isRunning()) {
        if(isStopped()) {
            stop = false;
        }
    }
    //start(LowPriority);
    start(HighPriority);
}

void Playback::setVideo(cv::VideoCapture cap, cv::VideoWriter wr, bool record) {
    mode = MODE_VIDEO;
    mutex.lock();
    capture = cap;
    writer = wr;
    recording = record;
    if(capture.isOpened()) {
        frame_rate =  capture.get(cv::CAP_PROP_FPS);
        if(frame_rate <= 0) frame_rate = 24;
    }
    mutex.unlock();
}

bool Playback::setVideoCamera(int device, int res, cv::VideoWriter wr, bool record) {
    mode = MODE_CAMERA;
    device_num = device;
    mutex.lock();
#if defined(__linux__) || defined(__APPLE__)
    capture.open(device);
    if(!capture.isOpened()) {
        mutex.unlock();
        return false;
    }
#else
    if(!capture.isOpened()) {
        capture.open(device);
        if(!capture.isOpened()) {
            mutex.unlock();
            return false;
        }
    }
#endif
    
    recording = record;
    writer = wr;
    int res_w = 0, res_h = 0, ores_w = 640, ores_h = 480;
    switch(res) {
        case 0:
            res_w = 640;
            res_h = 480;
            break;
        case 1:
            res_w = 1280;
            res_h = 720;
            break;
        case 2:
            res_w = 1920;
            res_h = 1080;
            break;
    }
    bool cw = capture.set(cv::CAP_PROP_FRAME_WIDTH, res_w);
    bool ch = capture.set(cv::CAP_PROP_FRAME_HEIGHT, res_h);
    if(cw == false || ch == false) {
        res_w = ores_w;
        res_h = ores_h;
        capture.set(cv::CAP_PROP_FRAME_WIDTH, res_w);
        capture.set(cv::CAP_PROP_FRAME_HEIGHT, res_h);
    }
    mutex.unlock();
    return true;
}

void Playback::setVector(std::vector<FilterValue> v) {
    mutex_add.lock();
    current = v;
    mutex_add.unlock();
}

void Playback::setOptions(bool n, int c) {
    mutex.lock();
    ac::isNegative = n;
    negate = n;
    reverse = c;
    ac::color_order = c;
    ac::in_custom = true;
    mutex.unlock();
}

void Playback::reset_filters() {
    mutex.lock();
    if(ac::reset_alpha == false) {
        ac::reset_alpha = true;
    }
    ac::frames_released = true;
    mutex.unlock();
}

void Playback::SetFlip(bool f1, bool f2) {
    mutex.lock();
    flip_frame1 = f1;
    flip_frame2 = f2;
    mutex.unlock();
}

void Playback::setColorOptions(int b, int g, int s) {
    mutex.lock();
    bright_ = b;
    gamma_ = g;
    saturation_ = s;
    mutex.unlock();
}

void Playback::setIndexChanged(std::string value) {
    prev_filter = current_filter;
    current_filter = filter_map[value];
    alpha = 1.0;
}

void Playback::setSubFilter(int index) {
    mutex.lock();
    ac::setSubFilter(index);
    mutex.unlock();
}

void Playback::setSingleMode(bool val) {
    single_mode = val;
}

void Playback::setRGB(int r, int g, int b) {
    mutex.lock();
    ac::swapColor_r = r;
    ac::swapColor_g = g;
    ac::swapColor_b = b;
    mutex.unlock();
}

void Playback::setColorMap(int c) {
    mutex.lock();
    ac::set_color_map = c;
    mutex.unlock();
}

void Playback::setDisplayed(bool shown) {
    video_shown = shown;
}

void Playback::drawEffects(cv::Mat &frame) {
    if(ac::set_color_map > 0) ac::ApplyColorMap(frame);
    ac::frames_released = false;
    ac::reset_alpha = false;
    if(bright_ > 0) {
        ac::setBrightness(frame, 1.0, bright_);
    }
    if(gamma_ > 0) {
        cv::Mat gam = frame.clone();
        ac::setGamma(gam, frame, gamma_);
    }
    if(saturation_ > 0) {
        ac::setSaturation(frame, saturation_);
    }
    if(colorkey_filter == true ||  (colorkey_replace == true && !color_replace_image.empty())) {
        cv::Mat cframe = frame.clone();
        cv::Vec3b well_color(255,0,255);
        ac::filterColorKeyed(well_color, ac::orig_frame, cframe, frame);
    }
}

void Playback::drawFilter(cv::Mat &frame, FilterValue &f) {
    if(f.index == 0) {
        ac::setSubFilter(f.subfilter);
        //ac::draw_func[f.filter](frame);
        ac::CallFilter(f.filter, frame);
        ac::setSubFilter(-1);
    } else if(current_filter.index == 1) {
        current_filterx = f.filter;
        ac::alphaFlame(frame);
    } else if(f.index == 2) {
        draw_plugin(frame, f.filter);
    }
}

void Playback::run() {
    
    int duration = 1000/ac::fps;
    
    while(!stop) {
        mutex.lock();
        if(!capture.read(frame)) {
            if(repeat_video && mode == MODE_VIDEO) {
                mutex.unlock();
                setFrameIndex(0);
                emit resetIndex();
                continue;
            }
            stop = true;
            mutex.unlock();
            emit stopRecording();
            ac::release_all_objects();
            return;
        }
        cv::Mat temp_frame;
        if(flip_frame1 == true) {
            cv::flip(frame, temp_frame, 1);
            frame = temp_frame;
        }
        if(flip_frame2 == true) {
            cv::flip(frame, temp_frame, 0);
            frame = temp_frame;
        }
        
        mutex.unlock();
        static std::vector<FilterValue> cur;
        mutex_shown.lock();
        cur = current;
        mutex_shown.unlock();
        ac::orig_frame = frame.clone();
        if(single_mode == true && alpha > 0) {
            if(fadefilter == true) filterFade(frame, current_filter, prev_filter, alpha);
            drawEffects(frame);
            alpha -= 0.08;
        } else if(single_mode == true) {
            mutex.lock();
            ac::in_custom = false;
            drawFilter(frame, current_filter);
            drawEffects(frame);
            msleep(duration/2);
            mutex.unlock();
        } else if(cur.size()>0) {
            mutex.lock();
            ac::in_custom = true;
            for(unsigned int i = 0; i < cur.size(); ++i) {
                if(i == cur.size()-1)
                    ac::in_custom = false;
                drawFilter(frame, cur[i]);
                msleep(duration/2);
            }
            drawEffects(frame);
            mutex.unlock();
        } else {
            msleep(duration);
        }
        mutex.lock();
        if(recording && writer.isOpened()) {
            writer.write(frame);
        }
        mutex.unlock();
        if(video_shown == true) {
            if(frame.channels()==3) {
                cv::cvtColor(frame, rgb_frame, cv::COLOR_BGR2RGB);
                img = QImage((const unsigned char*)(rgb_frame.data), rgb_frame.cols, rgb_frame.rows, QImage::Format_RGB888);
            } else {
                img = QImage((const unsigned char*)(frame.data), frame.cols, frame.rows, QImage::Format_Indexed8);
            }
            emit procImage(img);
            if(isStep == true) {
                isStep = false;
                return;
            }
        } else {
            emit frameIncrement();
        }
    }
    ac::release_all_objects();
}


Playback::~Playback() {
    mutex.lock();
    stop = true;
#if defined(__linux__) || defined(__APPLE__)
    condition.wakeOne();
#endif
    mutex.unlock();
#if defined(__linux__) || defined(__APPLE__)
    wait();
#endif
}

void Playback::setFrameIndex(const long &index) {
    mutex.lock();
    capture.set(cv::CAP_PROP_POS_FRAMES, index);
    mutex.unlock();
}

bool Playback::getFrame(QImage &img, const int &index) {
    QImage image;
    setFrameIndex(index);
    mutex.lock();
    cv::Mat frame;
    if(mode == MODE_VIDEO && capture.read(frame)) {
        cv::cvtColor(frame, rgb_frame, cv::COLOR_BGR2RGB);
        img = QImage((const unsigned char*)(rgb_frame.data), rgb_frame.cols, rgb_frame.rows, QImage::Format_RGB888);
        mutex.unlock();
        setFrameIndex(index);
        return true;
    }
    mutex.unlock();
    return false;
}

void Playback::enableRepeat(bool re) {
    mutex.lock();
    repeat_video = re;
    mutex.unlock();
}


void Playback::Clear() {
    mutex.lock();
    blend_set = false;
    colorkey_set = false;
    blend_image.release();
    color_image.release();
    mutex.unlock();
}

void Playback::Stop() {
    stop = true;
    alpha = 0;
    prev_filter = FilterValue(0, 0, -1);
}

void Playback::Release() {
    mutex.lock();
    stop = true;
    if(capture.isOpened() && mode == MODE_VIDEO) capture.release();
    if(writer.isOpened()) writer.release();
    mutex.unlock();
}

void Playback::msleep(int ms) {
    QThread::msleep(ms);
}

bool Playback::isStopped() const {
    return this->stop;
}

void Playback::setStep() {
    mutex.lock();
    isStep = true;
    mutex.unlock();
}

void Playback::setImage(const cv::Mat &frame) {
    mutex.lock();
    blend_set = true;
    blend_image = frame;
    mutex.unlock();
}

void Playback::setFadeFilter(bool f) {
    mutex.lock();
    fadefilter = f;
    mutex.unlock();
}

void Playback::setColorKey(const cv::Mat &image) {
    mutex.lock();
    colorkey_set = true;
    color_image = image;
    mutex.unlock();
}

void Playback::filterFade(cv::Mat &frame, FilterValue &filter1, FilterValue &filter2, double alpha) {
    unsigned int h = frame.rows; // frame height
    unsigned int w = frame.cols;// framew idth
    // make copies of original frame
    cv::Mat frame1 = frame.clone(), frame2 = frame.clone();
    // apply filters on two copies of original frame
    drawFilter(frame1,filter1);
    drawFilter(frame2,filter2);
    // loop through image setting each pixel with alphablended pixel
    for(unsigned int z = 0; z < h; ++z) {
        for(unsigned int i = 0; i < w; ++i) {
            cv::Vec3b &pixel = frame.at<cv::Vec3b>(z, i); // target pixel
            cv::Vec3b frame1_pix = frame1.at<cv::Vec3b>(z, i); // frame1 pixel
            cv::Vec3b frame2_pix = frame2.at<cv::Vec3b>(z, i); // frame2 pixel
            // loop through pixel components and set target pixel to alpha blended pixel of two frames
            for(unsigned int q = 0; q < 3; ++q)
                pixel[q] = frame2_pix[q]+(frame1_pix[q]*alpha);
        }
    }
}

