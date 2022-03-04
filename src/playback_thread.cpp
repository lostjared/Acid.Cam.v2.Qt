/*
 * Acid Cam v2 - Qt/OpenCV Edition
 * written by Jared Bruni ( http://lostsidedead.com )
 * (C) 2017 GPL
 */


#include"playback_thread.h"
#include<chrono>

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
    fadefilter = false;
    cycle_on = 0;
    cycle_index = 0;
    frame_num = 0;
    _custom_cycle = false;
    _custom_cycle_index = 0;
    fps_delay = 60;
    draw_strings = ac::draw_strings;
    filter_map_ex = filter_map;
    blend_image_copy_set = false;
}

bool Playback::VideoRelease() {
    bool ret = false;
    mutex.lock();
    if(ac::v_cap.isOpened()) {
        ac::v_cap.release();
        ret = true;
    }
    mutex.unlock();
    return ret;
}

void Playback::setCustomCycle(bool b) {
    _custom_cycle = b;
    _custom_cycle_index = 0;
}

void Playback::setCustomCycleDelay(int delay) {
    fps_delay = delay;
}

void Playback::Play() {
    if(!isRunning()) {
        if(isStopped()) {
            stop = false;
        }
    }
    prev_filter = FilterValue(0, 0, -1);
    alpha = 0;
    start(LowPriority);
    //start(HighPriority);
}

void Playback::setPngPath(std::string path) {
    mutex.lock();
    png_path = path;
    png_index = 0;
    mutex.unlock();
}

void Playback::setVideo(cv::VideoCapture cap, cv::VideoWriter wr, bool record, bool rec_png) {
    mutex.lock();
    mode = MODE_VIDEO;
    record_png = rec_png;
    capture = cap;
    writer = wr;
    recording = record;
    if(capture.isOpened()) {
        frame_rate =  capture.get(cv::CAP_PROP_FPS);
        if(frame_rate <= 0) frame_rate = 24;
    }
    mutex.unlock();
}

bool Playback::setVideoCamera(std::string name, int type, int device, int res, cv::VideoWriter wr, bool record) {
    mutex.lock();
    mode = MODE_CAMERA;
    device_num = device;

//#if defined(__linux__) || defined(__APPLE__)
#ifdef _WIN32
    capture.open(device, cv::CAP_DSHOW);
    //capture.open(0, cv::CAP_MSMF);
#else
    capture.open(device);
#endif
    if(!capture.isOpened()) {
        mutex.unlock();
        return false;
    }
    recording = record;
    writer = wr;
    int res_w = 640, res_h = 480;
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

   capture.set(cv::CAP_PROP_FRAME_WIDTH, res_w);
   capture.set(cv::CAP_PROP_FRAME_HEIGHT, res_h);
   double fps = capture.get(cv::CAP_PROP_FPS);
   res_w = capture.get(cv::CAP_PROP_FRAME_WIDTH);
   res_h = capture.get(cv::CAP_PROP_FRAME_HEIGHT);
    if(record == true && name.size()>0) {
        writer = cv::VideoWriter(name, type, fps, cv::Size(res_w, res_h), true);
        if(!writer.isOpened()) {
            return false;
        }
    }
    mutex.unlock();
    return true;
}

bool Playback::openVideo(std::string vname) {
    mutex.lock();
    ac::v_cap.open(vname);
    if(ac::v_cap.isOpened() == false) {
        mutex.unlock();
        return false;
    }
    mutex.unlock();
    return true;
}
void Playback::setVector(std::vector<FilterValue> v) {
    mutex.lock();
    // here:
    //ac::release_all_objects();
    current = v;
    mutex.unlock();
}

void Playback::setFilterMapEx(std::unordered_map<std::string, FilterValue> f) {
    mutex.lock();
    filter_map_ex_set = f;
    setFilterMap = true;
    mutex.unlock();
}

unsigned long Playback::calcMem() {
    mutex.lock();
    unsigned long calc = ac::calculateMemory();
    mutex.unlock();
    return calc;
}

void Playback::setAlpha(int a) {
    mutex.lock();
    ac::alpha_increase = a;
    mutex.unlock();
}

bool Playback::getProgramMode() {
    return single_mode;
}

void Playback::setProcMode(int p) {
    mutex.lock();
    ac::setProcMode(ac::PROC_MODE_TYPE(p));
    mutex.unlock();
}

void Playback::setMaxAlloc(int a) {
    mutex.lock();
    ac::setMaxAllocated(a);
    mutex.unlock();
}

unsigned int Playback::getObjectSize() {
    mutex.lock();
    unsigned int s = ac::all_objects.size();
    mutex.unlock();
    return s;
}

unsigned long Playback::allocatedFrames() {
    mutex.lock();
    unsigned long l = ac::getCurrentAllocatedFrames();
    mutex.unlock();
    return l;
    
}

unsigned long Playback::getMaxAlloc() {
    mutex.lock();
    unsigned long l = ac::getMaxAllocated();
    mutex.unlock();
    return l;
}

void Playback::setWaitColorLevel(int w, int l) {
    mutex.lock();
    ac::setVariableWait(w);
    ac::setColorLevel(l);
    mutex.unlock();
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

void Playback::setCycle(int type, int frame_skip, std::vector<std::string> &v) {
    mutex.lock();
    cycle_on = type;
    if(!cycle_values.empty())
        cycle_values.erase(cycle_values.begin(), cycle_values.end());
    if(v.size() > 1) {
        for(auto &i : v) {
            cv::Mat value = cv::imread(i);
            cycle_values.push_back(value);
        }
        cycle_index = 0;
        frame_num = frame_skip;
        blend_image_copy = cv::imread(v[0]);
        blend_image_copy_set = true;
        blend_set = true;
        static std::random_device r;
        static auto rng = std::default_random_engine(r());
        std::shuffle(cycle_values.begin(), cycle_values.end(), rng);
    }
    mutex.unlock();
}

void Playback::setCycle(int type) {
    cycle_on = type;
    cycle_index = 0;
    blend_set = false;
}

void Playback::reset_filters() {
    mutex.lock();
    if(ac::reset_alpha == false) {
        ac::reset_alpha = true;
    }
    ac::image_matrix_reset = true;
    ac::frames_released = true;
    mutex.unlock();
}

void Playback::clearImage() {
    mutex.lock();
    blend_set = false;
    blend_image.release();
    mutex.unlock();
}

void Playback::SetFlip(bool f1, bool f2) {
    flip_frame1 = f1;
    flip_frame2 = f2;
}

void Playback::setColorOptions(int b, int g, int s) {
    bright_ = b;
    gamma_ = g;
    saturation_ = s;
}

void Playback::setPref(int thread_count, int intense) {
    mutex.lock();
    ac::setThreadCount(thread_count);
    ac::setPixelCollection(intense);
    mutex.unlock();
}

void Playback::setIndexChanged(std::string value) {
    mutex.lock();
    prev_filter = current_filter;
    current_filter = filter_map[value];
    // here:
    //ac::release_all_objects();
    alpha = 1.0;
    mutex.unlock();
    
}

void Playback::setSingleMode(bool val) {
    mutex.lock();
    single_mode = val;
    mutex.unlock();
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
    mutex.lock();
    video_shown = shown;
    mutex.unlock();

}

void Playback::drawEffects(cv::Mat &frame) {
    if(ac::set_color_map > 0) ac::ApplyColorMap(frame);
    ac::frames_released = false;
    ac::reset_alpha = false;
    ac::image_matrix_reset = false;
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
        if(single_mode == true &&
           draw_strings[f.filter].find("SubFilter") != std::string::npos)
            return;
        
        if(single_mode == false && draw_strings[f.filter].find("SubFilter") != std::string::npos && f.subfilter == -1)
            return;
        
        if(ac::getMaxAllocated() < 1080 && draw_strings[f.filter].find("Intertwine") != std::string::npos)
            return;
        if(ac::getMaxAllocated() < 1080 && draw_strings[f.filter].find("InOrder") != std::string::npos)
            return;
        if(ac::getMaxAllocated() < 1080 && draw_strings[f.filter].find("Slit") != std::string::npos)
            return;
        
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

#ifdef _WIN32
    int duration = (1000/ac::fps)/4;
#else
    int duration = 10;
#endif
    while(!stop) {
        
        std::chrono::time_point<std::chrono::system_clock> now =
        std::chrono::system_clock::now();
        
        mutex.lock();
        if(ac::release_frames) {
            std::cout << "Cleared Frames...\n";
            ac::release_all_objects();
            ac::release_frames = false;
        }
        if(!capture.read(frame)) {
            if(repeat_video && mode == MODE_VIDEO) {
                mutex.unlock();
                setFrameIndex(0);
                emit resetIndex();
                continue;
            }
            stop = true;
            ac::release_all_objects();
            mutex.unlock();
            emit stopRecording();
            return;
        }
        static std::vector<FilterValue> cur;
        cur = current;
        ac::orig_frame = frame.clone();
        FilterValue current_filterxv = current_filter, prev_filterx = prev_filter;
        std::string png_pathx = png_path;
        if(blend_image_copy_set) {
            blend_image = blend_image_copy.clone();
            blend_image_copy_set = false;
        }
        if(chroma_image_set) {
            color_replace_image = chroma_image.clone();
            chroma_image_set = false;
        }
        if(setFilterMap) {
            filter_map_ex = filter_map_ex_set;
            setFilterMap = false;
        }
        
        mutex.unlock();
        
        cv::Mat temp_frame;
        if(flip_frame1 == true) {
            cv::flip(frame, temp_frame, 1);
            frame = temp_frame;
        }
        if(flip_frame2 == true) {
            cv::flip(frame, temp_frame, 0);
            frame = temp_frame;
        }

        if(cycle_on > 0) {
            cv::Mat *cycle_image = 0;
            static int frame_count = 0;
            ++frame_count;
            if(frame_count > frame_num) {
                mutex.lock();
                frame_count = 0;
                switch(cycle_on) {
                    case 0:
                        break;
                    case 1:
                        cycle_image = &cycle_values[rand()%cycle_values.size()];
                        break;
                    case 2:
                        cycle_image = &cycle_values[cycle_index];
                        ++cycle_index;
                        if(cycle_index > static_cast<int>(cycle_values.size()-1))
                            cycle_index = 0;

                        break;
                    case 3:
                        cycle_image = &cycle_values[cycle_index];
                        ++cycle_index;
                        if(cycle_index > static_cast<int>(cycle_values.size()-1)) {
                            cycle_index = 0;
                            static std::random_device r;
                            static auto rng = std::default_random_engine(r());
                            std::shuffle(cycle_values.begin(), cycle_values.end(), rng);
                        }
                        break;
                }
                if(blend_set == true && cycle_image != 0)
                    blend_image = cycle_image->clone();
                mutex.unlock();
            }
        }
        
        if(single_mode == true && alpha > 0) {
            if(fadefilter == true) filterFade(frame, current_filterxv, prev_filter, alpha);
            drawEffects(frame);
            alpha = alpha-0.08;
        } else if(single_mode == true) {
            ac::setSubFilter(-1);
            ac::in_custom = false;
            drawFilter(frame, current_filter);
            drawEffects(frame);
            msleep(duration);
        } else if(cur.size()>0) {
            ac::in_custom = true;
            if(_custom_cycle == false) {
                for(unsigned int i = 0; i < cur.size(); ++i) {
                    if(i == cur.size()-1)
                        ac::in_custom = false;
                    drawFilter(frame, cur[i]);
                    //msleep(duration);
                }
            } else {
                if(_custom_cycle_index > static_cast<int>(cur.size()-1))
                    _custom_cycle_index = 0;

                if(_custom_cycle_index >= 0 && _custom_cycle_index < static_cast<int>(cur.size())) {
                    drawFilter(frame, cur[_custom_cycle_index]);
                    //msleep(duration);
                }
            }
            drawEffects(frame);
            static int delay_counter = 0;
            ++delay_counter;
            if(delay_counter > (fps_delay * static_cast<int>(ac::fps))) {
                delay_counter = 0;
                ++_custom_cycle_index;
                if(_custom_cycle_index > static_cast<int>(cur.size()-1))
                    _custom_cycle_index = 0;
            }
        } else {
            msleep(duration);
        }
        
        if(record_png) {
            std::ostringstream stream;
            stream << png_pathx << "/" << std::setfill('0') << std::setw(15) << png_index << ".png";
            ++png_index;
            cv::imwrite(stream.str(), frame);
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
        
        std::chrono::time_point<std::chrono::system_clock> nowx =
        std::chrono::system_clock::now();
        auto m = std::chrono::duration_cast<std::chrono::milliseconds>(nowx - now).count();
        if (ac::fps > 0) {
            int fps_mil = 1000 / ac::fps;
            if (m < fps_mil)
                std::this_thread::sleep_for(std::chrono::milliseconds(fps_mil - m - 1));
        }
        
    }
    mutex.lock();
    ac::release_all_objects();
    mutex.unlock();
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
    repeat_video = re;
}


void Playback::Clear() {
    blend_set = false;
    colorkey_set = false;
    mutex.lock();
    blend_image.release();
    color_image.release();
    mutex.unlock();
}

void Playback::Stop() {
    stop = true;
    alpha = 0;
    mutex.lock();
    prev_filter = FilterValue(0, 0, -1);
    mutex.unlock();
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
    isStep = true;
}

void Playback::setImage(const cv::Mat &frame) {
    blend_set = true;
    mutex.lock();
    blend_image_copy = frame.clone();
    blend_image_copy_set = true;
    mutex.unlock();
}

void Playback::setChromaImage(cv::Mat &frame) {
    mutex.lock();
    chroma_image = frame.clone();
    chroma_image_set = true;
    mutex.unlock();
}

void Playback::setFadeFilter(bool f) {
    mutex.lock();
    fadefilter = f;
    mutex.unlock();
}

void Playback::setColorKey(const cv::Mat &image) {
    colorkey_set = true;
    mutex.lock();
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

