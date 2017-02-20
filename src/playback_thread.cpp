#include"playback_thread.h"

Playback::Playback(QObject *parent) : QThread(parent) {
    stop = true;
    isStep = false;
    isPaused = false;
}

void Playback::Play() {
    if(!isRunning()) {
        if(isStopped()) {
            stop = false;
        }
    }
    //start(LowPriority);
    start(TimeCriticalPriority);
}

void Playback::setVideo(cv::VideoCapture cap, cv::VideoWriter wr, bool record) {
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

void Playback::setDisplayed(bool shown) {
    mutex_shown.lock();
    video_shown = shown;
    mutex_shown.unlock();
}

void Playback::run() {
    while(!stop) {
        mutex.lock();
        if(!capture.read(frame)) {
            stop = true;
            mutex.unlock();
            emit stopRecording();
            return;
        }
        
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
        }
        if(recording && writer.isOpened()) {
            writer.write(frame);
        }
        mutex.unlock();
        bool shown_var;
        mutex_shown.lock();
        shown_var = video_shown;
        mutex_shown.unlock();
        
        if(shown_var == true) {
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
    mutex.lock();
    stop = true;
    mutex.unlock();
}

void Playback::Release() {
    mutex.lock();
    stop = true;
    if(capture.isOpened()) capture.release();
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
