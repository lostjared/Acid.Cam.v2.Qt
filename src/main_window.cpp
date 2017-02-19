#include "main_window.h"
#include<mutex>
#include"plugin.h"


std::unordered_map<std::string, std::pair<int, int>> filter_map;

const char *filter_names[] = { "AC Self AlphaBlend", "Reverse Self AlphaBlend",
    "Opposite Self AlphaBlend", "AC2 Distort", "Reverse Distort", "Opposite Distort",
    "Full Distort", "A New One", "AC NewOne", "AC Thought Filter", "Line Draw",
    "Gradient Square", "Color Wave", "Pixelated Gradient", "Combined Gradient",
    "Diagonal", "Average", "Average Divide", "Cos/Sin Multiply", "Modulus Multiply",
    "Positive/Negative", "z+1 Blend", "Diamond Pattern", "Pixelated Shift","Pixelated Mix",
    "Color Accumulate", "Color Accumulate #2", "Color Accumulate #3", "Angle",
    "Vertical Average", "Circular Blend", "Average Blend", "~Divide", "Mix", "Random Number",
    "Gradient Repeat", 0 };

void generate_map() {
    for(int i = 0; i < ac::draw_max; ++i )
        filter_map[ac::draw_strings[i]] = std::make_pair(0, i);
    
    int index = 0;
    while(filter_names[index] != 0) {
        std::string filter_n = "AF_";
        filter_n += filter_names[index];
        filter_map[filter_n] = std::make_pair(1, index);
        ++index;
    }
    
    for(unsigned int j = 0; j < plugins.plugin_list.size(); ++j) {
        std::string name = "plugin " + plugins.plugin_list[j]->name();
        filter_map[name] = std::make_pair(2, j);
    }
    
}

void custom_filter(cv::Mat &) {
    
}


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
    start(LowPriority);
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
    mutex.lock();
    current = v;
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
        ac::orig_frame = frame.clone();
        if(current.size()>0) {
            ac::in_custom = true;
            for(unsigned int i = 0; i < current.size(); ++i) {
                if(i == current.size()-1)
                    ac::in_custom = false;
                
                if(current[i].first == 0) {
                    ac::draw_func[current[i].second](frame);
                } else if(current[i].first == 1) {
                    current_filterx = current[i].second;
                    ac::alphaFlame(frame);
                } else if(current[i].first == 2) {
                    draw_plugin(frame, current[i].second);
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

DisplayWindow::DisplayWindow(QWidget *parent) : QDialog(parent) {
    createControls();
    setGeometry(950, 200, 640, 480);
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
    setWindowTitle(tr("Acid Cam v2 - Display Window"));
    hide();
}
void DisplayWindow::createControls() {
    img_label = new QLabel(this);
    img_label->setGeometry(0,0,640, 480);
}
void DisplayWindow::displayImage(const QImage &img) {
    QRect src(QPoint(0, 0), size());
    QPixmap p = QPixmap::fromImage(img).scaled(size(),Qt::KeepAspectRatio, Qt::FastTransformation);
    QRect dst(QPoint(0,0),p.size());
    dst.moveCenter(src.center());
    img_label->setGeometry(dst);
    img_label->setPixmap(p);
}


void DisplayWindow::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.fillRect(QRect(QPoint(0, 0), size()), QColor(0,0,0));
}


AC_MainWindow::~AC_MainWindow() {
    
    controls_Stop();
    
    delete playback;
}
AC_MainWindow::AC_MainWindow(QWidget *parent) : QMainWindow(parent) {
    programMode = MODE_CAMERA;
    init_plugins();
    generate_map();
    setGeometry(100, 100, 800, 600);
    setFixedSize(800, 600);
    setWindowTitle(tr("Acid Cam v2 - Qt"));
    createControls();
    createMenu();
    
    cap_camera = new CaptureCamera(this);
    cap_camera->setParent(this);
    
    cap_video = new CaptureVideo(this);
    cap_video->setParent(this);
    
    statusBar()->showMessage(tr("Acid Cam v2 Loaded - Use File Menu to Start"));
    take_snapshot = false;
    disp = new DisplayWindow(this);
    playback = new Playback();
    QObject::connect(playback, SIGNAL(procImage(QImage)), this, SLOT(updateFrame(QImage)));
    QObject::connect(playback, SIGNAL(stopRecording()), this, SLOT(stopRecording()));
    QObject::connect(playback, SIGNAL(frameIncrement()), this, SLOT(frameInc()));
    
    for(unsigned int i = 0; i < plugins.plugin_list.size(); ++i) {
        QString text;
        QTextStream stream(&text);
        stream << "Loaded Plugin: " << plugins.plugin_list[i]->name().c_str() << "\n";
        Log(text);
    }
}

void AC_MainWindow::createControls() {
    
    filters = new QListWidget(this);
    filters->setGeometry(10, 30, 390, 180);
    filters->show();
    
    custom_filters = new QListWidget(this);
    custom_filters->setGeometry(400, 30, 390, 180);
    custom_filters->show();
    
    for(int i = 0; i < ac::draw_max-4; ++i) {
        filters->addItem(ac::draw_strings[i].c_str());
    }
    
    for(int i = 0; filter_names[i] != 0; ++i) {
        std::string filter_n = "AF_";
        filter_n += filter_names[i];
        filters->addItem(filter_n.c_str());
    }
    
    for(unsigned int i = 0; i < plugins.plugin_list.size(); ++i) {
    	std::string name = "plugin " + plugins.plugin_list[i]->name();
        filters->addItem(name.c_str());
    }
    
    btn_add = new QPushButton(tr("Add"), this);
    btn_remove = new QPushButton(tr("Remove"), this);
    btn_moveup = new QPushButton(tr("Move Up"), this);
    btn_movedown = new QPushButton(tr("Move Down"), this);
    
    btn_add->setGeometry(10, 215, 100, 20);
    btn_remove->setGeometry(400, 215, 100, 20);
    btn_moveup->setGeometry(500, 215, 100, 20);
    btn_movedown->setGeometry(600, 215, 100, 20);
    
    connect(btn_add, SIGNAL(clicked()), this, SLOT(addClicked()));
    connect(btn_remove, SIGNAL(clicked()), this, SLOT(rmvClicked()));
    connect(btn_moveup, SIGNAL(clicked()), this, SLOT(upClicked()));
    connect(btn_movedown, SIGNAL(clicked()), this, SLOT(downClicked()));
    
    log_text = new QTextEdit(this);
    log_text->setGeometry(10, 250, 780,310);
    log_text->setReadOnly(true);
    
    QString text = tr("Acid Cam Filters v");
    text += ac::version.c_str();
    text += " loaded.\n";
    log_text->setText(text);
    
    filters->setCurrentRow(0);
    
    chk_negate = new QCheckBox(tr("Negate"), this);
    chk_negate->setGeometry(120,215,100, 20);
    chk_negate->setCheckState(Qt::Unchecked);
    
    connect(chk_negate, SIGNAL(clicked()), this, SLOT(chk_Clicked()));
    
    combo_rgb = new QComboBox(this);
    combo_rgb->setGeometry(200,215, 190, 25);
    combo_rgb->addItem(tr("RGB"));
    combo_rgb->addItem(tr("BGR"));
    combo_rgb->addItem(tr("BRG"));
    combo_rgb->addItem(tr("GRB"));
    
    setWindowIcon(QPixmap(":/images/icon.png"));
    
}

void AC_MainWindow::createMenu() {
    
    file_menu = menuBar()->addMenu(tr("&File"));
    controls_menu = menuBar()->addMenu(tr("&Controls"));
    help_menu = menuBar()->addMenu(tr("Help"));
    
    file_new_capture = new QAction(tr("Capture from Webcam"),this);
    file_new_capture->setShortcut(tr("Ctrl+N"));
    file_menu->addAction(file_new_capture);
    
    file_new_video = new QAction(tr("Capture from Video"), this);
    file_new_video->setShortcut(tr("Ctrl+V"));
    file_menu->addAction(file_new_video);
    
    file_exit = new QAction(tr("E&xit"), this);
    file_exit->setShortcut(tr("Ctrl+X"));
    file_menu->addAction(file_exit);
    
    connect(file_new_capture, SIGNAL(triggered()), this, SLOT(file_NewCamera()));
    connect(file_new_video, SIGNAL(triggered()), this, SLOT(file_NewVideo()));
    connect(file_exit, SIGNAL(triggered()), this, SLOT(file_Exit()));
    
    controls_stop = new QAction(tr("Sto&p"), this);
    controls_stop->setShortcut(tr("Ctrl+C"));
    controls_menu->addAction(controls_stop);
    
    controls_stop->setEnabled(false);
    
    controls_snapshot = new QAction(tr("Take &Snapshot"), this);
    controls_snapshot->setShortcut(tr("S"));
    controls_menu->addAction(controls_snapshot);
    
    controls_pause = new QAction(tr("&Pause"), this);
    controls_pause->setShortcut(tr("P"));
    controls_menu->addAction(controls_pause);
    
    controls_step = new QAction(tr("Step"), this);
    controls_step->setShortcut(tr("I"));
    controls_menu->addAction(controls_step);
    
    controls_setimage = new QAction(tr("Set Image"), this);
    controls_setimage->setShortcut(tr("Ctrl+I"));
    controls_menu->addAction(controls_setimage);
    
    controls_showvideo = new QAction(tr("Hide Display Video"), this);
    controls_showvideo->setShortcut(tr("Ctrl+V"));
    controls_menu->addAction(controls_showvideo);
    
    controls_showvideo->setEnabled(false);
    controls_showvideo->setCheckable(true);
    
    connect(controls_snapshot, SIGNAL(triggered()), this, SLOT(controls_Snap()));
    connect(controls_pause, SIGNAL(triggered()), this, SLOT(controls_Pause()));
    connect(controls_step, SIGNAL(triggered()), this, SLOT(controls_Step()));
    connect(controls_stop, SIGNAL(triggered()), this, SLOT(controls_Stop()));
    connect(controls_setimage, SIGNAL(triggered()), this, SLOT(controls_SetImage()));
    connect(controls_showvideo, SIGNAL(triggered()), this, SLOT(controls_ShowVideo()));
    
    connect(combo_rgb, SIGNAL(currentIndexChanged(int)), this, SLOT(cb_SetIndex(int)));
    controls_pause->setText(tr("Pause"));
    help_about = new QAction(tr("About"), this);
    help_about->setShortcut(tr("Ctrl+A"));
    help_menu->addAction(help_about);
    connect(help_about, SIGNAL(triggered()), this, SLOT(help_About()));
    timer_video = new QTimer(this);
    timer_camera = new QTimer(this);
}

void AC_MainWindow::chk_Clicked() {
    playback->setOptions(chk_negate->isChecked(), combo_rgb->currentIndex());
}
void AC_MainWindow::cb_SetIndex(int index) {
    playback->setOptions(chk_negate->isChecked(), index);
    
}
void AC_MainWindow::addClicked() {
    
    int row = filters->currentRow();
    if(row != -1) {
        QListWidgetItem *item = filters->item(row);
        custom_filters->addItem(item->text());
        QString qs;
        QTextStream stream(&qs);
        stream << "Added Filter: " << item->text() << "\n";
        Log(qs);
        std::vector<std::pair<int, int>> v;
        buildVector(v);
        playback->setVector(v);
    }
}

void AC_MainWindow::rmvClicked() {
    int item = custom_filters->currentRow();
    if(item != -1) {
        QListWidgetItem *i = custom_filters->takeItem(item);
        QString qs;
        QTextStream stream(&qs);
        stream << "Removed Filter: " << i->text() << "\n";
        Log(qs);
        std::vector<std::pair<int, int>> v;
        buildVector(v);
        playback->setVector(v);
        
    }
}

void AC_MainWindow::upClicked() {
    int item = custom_filters->currentRow();
    if(item > 0) {
        QListWidgetItem *i = custom_filters->takeItem(item);
        custom_filters->insertItem(item-1, i->text());
        custom_filters->setCurrentRow(item-1);
        std::vector<std::pair<int, int>> v;
        buildVector(v);
        playback->setVector(v);
    
    }
}

void AC_MainWindow::downClicked() {
    int item = custom_filters->currentRow();
    if(item >= 0 && item < custom_filters->count()-1) {
    	QListWidgetItem *i = custom_filters->takeItem(item);
    	custom_filters->insertItem(item+1, i->text());
    	custom_filters->setCurrentRow(item+1);
        std::vector<std::pair<int, int>> v;
        buildVector(v);
        playback->setVector(v);
    }
  
}

void AC_MainWindow::Log(const QString &s) {
    QString text;
    text = log_text->toPlainText();
    text += s;
    log_text->setText(text);
    QTextCursor tmpCursor = log_text->textCursor();
    tmpCursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    log_text->setTextCursor(tmpCursor);
}

bool AC_MainWindow::startCamera(int res, int dev, const QString &outdir, bool record) {
    programMode = MODE_CAMERA;
    
    controls_showvideo->setEnabled(false);
    playback->setDisplayed(true);

    // setup device
    step_frame = false;
    video_file_name = "";
    frame_index = 0;
    capture_camera.open(dev);
    if(!capture_camera.isOpened()) {
        return false;
    }
    video_frames = 0;
    video_fps = 24;
    int res_w = capture_camera.get(CV_CAP_PROP_FRAME_WIDTH);
    int res_h = capture_camera.get(CV_CAP_PROP_FRAME_HEIGHT);
    QString str;
    QTextStream stream(&str);
    stream << "Opened capture device " << res_w << "x" << res_h << "\n";
    stream << "FPS: " << video_fps << "\n";
    output_directory = outdir;
    frame_index = 0;
    Log(str);
    paused = false;
    recording = record;
    QString output_name;
    QTextStream stream_(&output_name);
    static unsigned int index = 0;
    time_t t = time(0);
    struct tm *m;
    m = localtime(&t);
    std::ostringstream time_stream;
    time_stream << "-" << (m->tm_year + 1900) << "." << (m->tm_mon + 1) << "." << m->tm_mday << "_" << m->tm_hour << "." << m->tm_min << "." << m->tm_sec <<  "_";
    stream_ << outdir << "/" << "Video." << time_stream.str().c_str() << "AC2.Output." << (++index) << ".avi";
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
    capture_camera.set(CV_CAP_PROP_FRAME_WIDTH, res_w);
    capture_camera.set(CV_CAP_PROP_FRAME_HEIGHT, res_h);
    QString res_str;
    QTextStream res_s(&res_str);
    res_s << "Resolution set to: " << res_w << "x" << res_h << "\n";
    Log(res_str);
    if(recording) {
        video_file_name = output_name;
#if defined(__linux__) || defined(__APPLE__)
        writer = cv::VideoWriter(output_name.toStdString(), CV_FOURCC('X','V','I','D'), video_fps, cv::Size(res_w, res_h), true);
#else
        writer = cv::VideoWriter(output_name.toStdString(), -1, video_fps, cv::Size(res_w, res_h), true);
#endif
        if(!writer.isOpened()) {
            Log(tr("Could not create video writer..\n"));
        }
        QString out_s;
        QTextStream out_stream(&out_s);
        out_stream << "Now recording to: " << output_name << "\nResolution: " << res_w << "x" << res_h << " FPS: " << video_fps << "\n";
        Log(out_s);
    }
    // if successful
    file_new_capture->setEnabled(false);
    file_new_video->setEnabled(false);
    controls_stop->setEnabled(true);
    connect(timer_camera, SIGNAL(timeout()), this, SLOT(timer_Camera()));
    playback->setVideo(capture_camera, writer, recording);
    playback->Play();
    disp->show();
    return true;
}

bool AC_MainWindow::startVideo(const QString &filename, const QString &outdir, bool record) {
    programMode = MODE_VIDEO;
    if(record == true) controls_showvideo->setEnabled(true);
    playback->setDisplayed(true);
    
    video_file_name = "";
    step_frame = false;
    capture_video.open(filename.toStdString());
    if(!capture_video.isOpened()) {
        return false;
    }
    video_frames = capture_video.get(CV_CAP_PROP_FRAME_COUNT);
    if(video_frames <= 0) return false;
    video_fps = capture_video.get(CV_CAP_PROP_FPS);
    int res_w = capture_video.get(CV_CAP_PROP_FRAME_WIDTH);
    int res_h = capture_video.get(CV_CAP_PROP_FRAME_HEIGHT);
    QString str;
    QTextStream stream(&str);
    stream << "Opened capture device " << res_w << "x" << res_h << "\n";
    stream << "FPS: " << video_fps << "\n";
    stream << "Frame Count: " << video_frames << "\n";
    output_directory = outdir;
    frame_index = 0;
    Log(str);
    // if successful
    file_new_capture->setEnabled(false);
    file_new_video->setEnabled(false);
    controls_stop->setEnabled(true);
    paused = false;
    recording = record;
    QString output_name;
    QTextStream stream_(&output_name);
    static unsigned int index = 0;
    time_t t = time(0);
    struct tm *m;
    m = localtime(&t);
    std::ostringstream time_stream;
    time_stream << "-" << (m->tm_year + 1900) << "." << (m->tm_mon + 1) << "." << m->tm_mday << "_" << m->tm_hour << "." << m->tm_min << "." << m->tm_sec <<  "_";
    stream_ << outdir << "/" << "Video." << time_stream.str().c_str() << "AC2.Output." << (++index) << ".avi";

    
    if(recording) {
        video_file_name = output_name;
#if defined(__linux__) || defined(__APPLE__)
        writer = cv::VideoWriter(output_name.toStdString(), CV_FOURCC('X','V','I','D'), video_fps, cv::Size(res_w, res_h), true);
#else
        writer = cv::VideoWriter(output_name.toStdString(), -1, video_fps, cv::Size(res_w, res_h), true);
#endif
        if(!writer.isOpened()) {
            Log("Error could not open video writer.\n");
        }
        QString out_s;
        QTextStream out_stream(&out_s);
        out_stream << "Now recording to: " << output_name << "\nResolution: " << res_w << "x" << res_h << " FPS: " << video_fps << "\n";
        Log(out_s);
    }
    playback->setVideo(capture_video,writer,recording);
    playback->Play();
    disp->show();
    return true;
}

void AC_MainWindow::controls_Stop() {
    playback->Stop();
    controls_showvideo->setEnabled(false);
    
    if(capture_video.isOpened()) {
        capture_video.release();
        if(recording == true) writer.release();
        cv::destroyWindow("Acid Cam v2");
        file_new_capture->setEnabled(true);
        file_new_video->setEnabled(true);
        if(recording) {
            QString stream_;
            QTextStream stream(&stream_);
            stream << "Wrote video file: " << video_file_name << "\n";
            Log(stream_);
        }
        disp->hide();
        playback->Release();
    }
    if(capture_camera.isOpened()) {
        capture_camera.release();
        if(recording == true) writer.release();
        cv::destroyWindow("Acid Cam v2");
        file_new_capture->setEnabled(true);
        file_new_video->setEnabled(true);
        if(recording) {
            QString stream_;
            QTextStream stream(&stream_);
            stream << "Wrote video file: " << video_file_name << "\n";
            Log(stream_);
        }
        disp->hide();
        playback->Release();
    }
    
}

void AC_MainWindow::controls_ShowVideo() {
    QString st = controls_showvideo->text();
    
    if(st == "Hide Display Video") {
        playback->setDisplayed(false);
        disp->hide();
        controls_showvideo->setText("Show Display Video");
    } else {
        controls_showvideo->setText("Hide Display Video");
        playback->setDisplayed(true);
        disp->show();
    }
}

void AC_MainWindow::file_Exit() {
    QApplication::exit(0);
}

void AC_MainWindow::file_NewVideo() {
    cap_video->show();
}

void AC_MainWindow::file_NewCamera() {
    cap_camera->show();
}

void AC_MainWindow::controls_Snap() {
    take_snapshot = true;
}

void AC_MainWindow::controls_Pause() {
    QString p = controls_pause->text();
    if(p == "Pause") {
        controls_pause->setText("Paused");
        controls_pause->setChecked(Qt::Checked);
        paused = true;
        playback->Stop();
    } else {
        controls_pause->setText("Pause");
        controls_pause->setChecked(Qt::Unchecked);
        playback->Play();
        paused = false;
    }
}

void AC_MainWindow::controls_SetImage() {
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open Image"), "/home", tr("Image Files (*.png *.jpg)"));
    if(fileName != "") {
        cv::Mat tblend_image = cv::imread(fileName.toStdString());
        if(!tblend_image.empty()) {
            playback->setImage(tblend_image);
            QMessageBox::information(this, tr("Loaded Image"), tr("Image set"));
        }
    }
}

void AC_MainWindow::controls_Step() {
    playback->setStep();
    playback->Play();
    step_frame = true;
}

void AC_MainWindow::timer_Camera() {
    /*
    if(step_frame == true  && paused == true) {
        step_frame = false;
    }
    else if(paused == true) return;
    ac::isNegative = chk_negate->isChecked();
    ac::color_order = combo_rgb->currentIndex();
    negate = ac::isNegative;
    reverse = ac::color_order;
    cv::Mat mat;
    if(capture_camera.read(mat) == false) {
        controls_Stop();
        return;
    }
    ac::in_custom = true;
    for(int i = 0; i < custom_filters->count(); ++i) {
        if(i == custom_filters->count()-1)
            ac::in_custom = false;
        
        QListWidgetItem *val = custom_filters->item(i);
        QString name = val->text();
        if(filter_map[name.toStdString()].first == 0)
            ac::draw_func[filter_map[name.toStdString()].second](mat);
        else {
            red = green = blue = 0;
            current_filterx = filter_map[name.toStdString()].second;
            ac::alphaFlame(mat);
        }
    }
    if(take_snapshot == true) {
        static int index = 0;
        QString text;
        QTextStream stream(&text);
        stream << output_directory << "/" << "AC.Snapshot." << ++index << ".png";
        cv::imwrite(text.toStdString(), mat);
        QString total;
        QTextStream stream_total(&total);
        stream_total << "Took Snapshot: " << text << "\n";
        Log(total);
        take_snapshot = false;
    }
    
    if(recording) {
        writer.write(mat);
        file_size.seekg(0, std::ios::end);
        file_pos = file_size.tellg();
    }
    
    frame_index++;
    QString frame_string;
    QTextStream frame_stream(&frame_string);
    
    frame_stream << "(Current/Total Frames/Seconds/Size) - (" << frame_index << "/" << video_frames << "/" << (frame_index/video_fps) << "/" << ((file_pos/1024)/1024) << " MB)";
     */
    
}

void AC_MainWindow::timer_Video() {
    
    /*
    if(step_frame == true  && paused == true)
        step_frame = false;
    else if(paused == true) return;

    ac::isNegative = chk_negate->isChecked();
    ac::color_order = combo_rgb->currentIndex();
    negate = ac::isNegative;
    reverse = ac::color_order;
    cv::Mat mat;
    if(capture_video.read(mat) == false) {
        controls_Stop();
        return;
    }
    ac::in_custom = true;
    for(int i = 0; i < custom_filters->count(); ++i) {
        if(i == custom_filters->count()-1)
            ac::in_custom = false;
        
        QListWidgetItem *val = custom_filters->item(i);
        QString name = val->text();
        if(filter_map[name.toStdString()].first == 0)
        	ac::draw_func[filter_map[name.toStdString()].second](mat);
        else {
            red = green = blue = 0;
            current_filterx = filter_map[name.toStdString()].second;
            ac::alphaFlame(mat);
        }
    }
    if(take_snapshot == true) {
    	static int index = 0;
    	QString text;
    	QTextStream stream(&text);
    	stream << output_directory << "/" << "AC.Snapshot." << ++index << ".png";
    	cv::imwrite(text.toStdString(), mat);
    	QString total;
    	QTextStream stream_total(&total);
    	stream_total << "Took Snapshot: " << text << "\n";
    	Log(total);
        take_snapshot = false;
    }
    if(recording) {
        writer.write(mat);
        file_size.seekg(0, std::ios::end);
        file_pos = file_size.tellg();
    }
    
    frame_index++;
    QString frame_string;
    QTextStream frame_stream(&frame_string);
    
    frame_stream << "(Current/Total Frames/Seconds/Size) - (" << frame_index << "/" << video_frames << "/" << (frame_index/video_fps) << "/" << ((file_pos/1024)/1024) << " MB)";
    
    statusBar()->showMessage(frame_string); */
}

void AC_MainWindow::buildVector(std::vector<std::pair<int,int>> &v) {
    if(!v.empty()) v.erase(v.begin(), v.end());
    for(int i = 0; i < custom_filters->count(); ++i) {
        QListWidgetItem *val = custom_filters->item(i);
        QString name = val->text();
        v.push_back(filter_map[name.toStdString()]);
    }
}


cv::Mat QImage2Mat(QImage const& src)
{
    cv::Mat tmp(src.height(),src.width(),CV_8UC3,(uchar*)src.bits(),src.bytesPerLine());
    cv::Mat result;
    cvtColor(tmp, result,CV_BGR2RGB);
    return result;
}

QImage Mat2QImage(cv::Mat const& src)
{
    cv::Mat temp;
    cvtColor(src, temp,CV_BGR2RGB);
    QImage dest((const uchar *) temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
    dest.bits();
    return dest;
}


void AC_MainWindow::updateFrame(QImage img) {
    if(playback->isStopped() == false) {
    	disp->displayImage(img);
        frame_index++;
        QString frame_string;
        QTextStream frame_stream(&frame_string);
        frame_stream.setRealNumberPrecision(2);
        
        frame_stream << "(Current/Total Frames/Seconds) - (" << frame_index << "/" << video_frames << "/" << (frame_index/video_fps) << ") ";
        if(programMode == MODE_VIDEO) {
            
            float index = frame_index;
            float max_frames = video_frames;
            
            frame_stream << " - " << (index/max_frames)*100 << "%";
        }
        statusBar()->showMessage(frame_string);
        
        if(take_snapshot == true) {
            cv::Mat mat = QImage2Mat(img);
            static int index = 0;
            QString text;
            QTextStream stream(&text);
            time_t t = time(0);
            struct tm *m;
            m = localtime(&t);
            std::ostringstream time_stream;
            time_stream << "-" << (m->tm_year + 1900) << "." << (m->tm_mon + 1) << "." << m->tm_mday << "_" << m->tm_hour << "." << m->tm_min << "." << m->tm_sec <<  "_";
            stream << output_directory << "/" << "AC2.Snapshot." << time_stream.str().c_str() << "." << ++index << ".png";
            cv::imwrite(text.toStdString(), mat);
            QString total;
            QTextStream stream_total(&total);
            stream_total << "Took Snapshot: " << text << "\n";
            Log(total);
            take_snapshot = false;
        }
    } 
}

void AC_MainWindow::stopRecording() {
    controls_Stop();
}

void AC_MainWindow::frameInc() {
    frame_index++;
    QString frame_string;
    QTextStream frame_stream(&frame_string);
    frame_stream.setRealNumberPrecision(2);
    frame_stream << "(Current/Total Frames/Seconds) - (" << frame_index << "/" << video_frames << "/" << (frame_index/video_fps) << ") ";
    if(programMode == MODE_VIDEO) {
        float index = frame_index;
        float max_frames = video_frames;
        frame_stream << " - " << (index/max_frames)*100 << "%";
    }
    statusBar()->showMessage(frame_string);
}

void AC_MainWindow::help_About() {
    QString about_str;
    QTextStream stream(&about_str);
    stream << tr("<b>Acid Cam Qt version: ") << ac_version << "</b><br><br> ";
    stream << tr("Written by <b>Jared Bruni</b><br><br><b>Social Media Accounts</b><br><br>\n\n <a href=\"http://github.com/lostjared\">GitHub</a><br>\n<a href=\"http://youtube.com/lostjared\">YouTube</a><br><a href=\"http://instagram.com/lostjared\">Instagram</a><br><a href=\"http://facebook.com/LostSideDead0x\">Facebook</a><br><a href=\"http://twitter.com/jaredbruni\">Twitter</a><br><br><br>\n");
    
    QMessageBox::information(this, tr("About Acid Cam"), about_str);
}

