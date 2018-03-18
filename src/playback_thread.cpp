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
}

void Playback::Play() {
    if(!isRunning()) {
        if(isStopped()) {
            stop = false;
        }
    }
    start(LowPriority);
    //start(HighPriority);
}

void Playback::setVideo(cv::VideoCapture cap, cv::VideoWriter wr, bool record) {
    mode = MODE_VIDEO;
    mutex.lock();
    capture = cap;
    writer = wr;
    recording = record;
    if(capture.isOpened()) {
        frame_rate = (int) capture.get(CV_CAP_PROP_FPS);
        if(frame_rate <= 0) frame_rate = 24;
    }
    mutex.unlock();
}

bool Playback::setVideoCamera(int device, int res, cv::VideoWriter wr, bool record) {
    mode = MODE_CAMERA;
    device_num = device;
    mutex.lock();
    if(capture.isOpened()) {
#if defined(__linux__) || defined(_WIN32)
        capture.open(device);
        if(!capture.isOpened()) {
            mutex.unlock();
            return false;
        }
#endif
    } else {
        capture.open(device);
        if(!capture.isOpened()) {
            mutex.unlock();
            return false;
        }
    }
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
    bool cw = capture.set(CV_CAP_PROP_FRAME_WIDTH, res_w);
    bool ch = capture.set(CV_CAP_PROP_FRAME_HEIGHT, res_h);
    if(cw == false || ch == false) {
        res_w = ores_w;
        res_h = ores_h;
        capture.set(CV_CAP_PROP_FRAME_WIDTH, res_w);
        capture.set(CV_CAP_PROP_FRAME_HEIGHT, res_h);
    }
    mutex.unlock();
    return true;
}

void Playback::setVector(std::vector<std::pair<int, int>> v) {
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

void Playback::setColorOptions(int b, int g, int s) {
    mutex.lock();
    bright_ = b;
    gamma_ = g;
    saturation_ = s;
    mutex.unlock();
}

void Playback::setIndexChanged(int pos) {
    
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

void Playback::run() {
    
    int duration = 1000/ac::fps;
    
    while(!stop) {
        mutex.lock();
        if(!capture.read(frame)) {
            stop = true;
            mutex.unlock();
            emit stopRecording();
            return;
        }
        mutex.unlock();
        static std::vector<std::pair<int, int>> cur;
        mutex_shown.lock();
        cur = current;
        mutex_shown.unlock();
        ac::orig_frame = frame.clone();
        if(cur.size()>0) {
            ac::in_custom = true;
            for(unsigned int i = 0; i < cur.size(); ++i) {
                if(i == cur.size()-1)
                    ac::in_custom = false;
                if(cur[i].first == 0) {
                    ac::draw_func[cur[i].second](frame);
                } else if(cur[i].first == 1) {
                    current_filterx = cur[i].second;
                    ac::alphaFlame(frame);
                } else if(cur[i].first == 2) {
                    draw_plugin(frame, cur[i].second);
                }
            }
            mutex.lock();
            if(ac::set_color_map > 0) ac::ApplyColorMap(frame);
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
                cv::cvtColor(frame, rgb_frame, CV_BGR2RGB);
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

void Playback::Stop() {
    stop = true;
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
