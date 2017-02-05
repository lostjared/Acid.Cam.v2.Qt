#include "main_window.h"

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
}

void custom_filter(cv::Mat &frame) {
    
}

AC_MainWindow::AC_MainWindow(QWidget *parent) : QMainWindow(parent) {
    generate_map();
    setGeometry(100, 100, 800, 600);
    setFixedSize(800, 600);
    setWindowTitle("Acid Cam v2 - Qt");
    createControls();
    createMenu();
    
    cap_camera = new CaptureCamera(this);
    cap_camera->setParent(this);
    
    cap_video = new CaptureVideo(this);
    cap_video->setParent(this);
    
    statusBar()->showMessage(tr("Acid Cam v2 Loaded - Use File Menu to Start"));
    take_snapshot = false;
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
    
    btn_add = new QPushButton("Add", this);
    btn_remove = new QPushButton("Remove", this);
    btn_moveup = new QPushButton("Move Up", this);
    btn_movedown = new QPushButton("Move Down", this);
    
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
    
    QString text = "Acid Cam v";
    text += ac::version.c_str();
    text += " loaded.\n";
    log_text->setText(text);
    
    filters->setCurrentRow(0);
    
    chk_negate = new QCheckBox("Negate", this);
    chk_negate->setGeometry(120,215,100, 20);
    chk_negate->setCheckState(Qt::Unchecked);
    
    combo_rgb = new QComboBox(this);
    combo_rgb->setGeometry(200,215, 190, 25);
    combo_rgb->addItem("RGB");
    combo_rgb->addItem("BGR");
    combo_rgb->addItem("BRG");
    combo_rgb->addItem("GRB");
    
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
    
    connect(controls_snapshot, SIGNAL(triggered()), this, SLOT(controls_Snap()));
    connect(controls_pause, SIGNAL(triggered()), this, SLOT(controls_Pause()));
    connect(controls_step, SIGNAL(triggered()), this, SLOT(controls_Step()));
    connect(controls_stop, SIGNAL(triggered()), this, SLOT(controls_Stop()));
    connect(controls_setimage, SIGNAL(triggered()), this, SLOT(controls_SetImage()));
    
    controls_pause->setCheckable(true);
    controls_pause->setText("Pause");
    controls_pause->setChecked(false);
    
    help_about = new QAction(tr("About"), this);
    help_about->setShortcut(tr("Ctrl+A"));
    help_menu->addAction(help_about);
    
    connect(help_about, SIGNAL(triggered()), this, SLOT(help_About()));
    timer_video = new QTimer(this);
    timer_camera = new QTimer(this);
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
    }
}

void AC_MainWindow::upClicked() {
    int item = custom_filters->currentRow();
    if(item > 0) {
        QListWidgetItem *i = custom_filters->takeItem(item);
        custom_filters->insertItem(item-1, i->text());
        custom_filters->setCurrentRow(item-1);
    }
}

void AC_MainWindow::downClicked() {
    int item = custom_filters->currentRow();
    if(item >= 0 && item < custom_filters->count()-1) {
    	QListWidgetItem *i = custom_filters->takeItem(item);
    	custom_filters->insertItem(item+1, i->text());
    	custom_filters->setCurrentRow(item+1);
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
    cv::namedWindow("Acid Cam v2");
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
            Log("Could not create video writer..\n");
        }
        QString out_s;
        QTextStream out_stream(&out_s);
        out_stream << "Now recording to: " << output_name << "\nResolution: " << res_w << "x" << res_h << " FPS: " << video_fps << "\n";
        Log(out_s);
        file_size.open(output_name.toStdString(), std::ios::in | std::ios::binary);
    }
    // if successful
    file_new_capture->setEnabled(false);
    file_new_video->setEnabled(false);
    controls_stop->setEnabled(true);
    connect(timer_camera, SIGNAL(timeout()), this, SLOT(timer_Camera()));
    timer_camera->setInterval(1000/24);
    timer_camera->start();
    return true;
}

bool AC_MainWindow::startVideo(const QString &filename, const QString &outdir, bool record) {
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
    cv::namedWindow("Acid Cam v2");
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
        file_size.open(output_name.toStdString(), std::ios::in | std::ios::binary);
        file_size.seekg(0, std::ios::end);
    }
    connect(timer_video, SIGNAL(timeout()), this, SLOT(timer_Video()));
    
    timer_video->setInterval((1000/ac::fps));
    timer_video->start();
    
    return true;
}

void AC_MainWindow::controls_Stop() {
    if(capture_video.isOpened()) {
        timer_video->stop();
        capture_video.release();
        writer.release();
        cv::destroyWindow("Acid Cam v2");
        file_new_capture->setEnabled(true);
        file_new_video->setEnabled(true);
        if(recording) {
            QString stream_;
            QTextStream stream(&stream_);
            stream << "Wrote video file: " << video_file_name << "\n";
            Log(stream_);
            file_size.close();
        }
    }
    if(capture_camera.isOpened()) {
        timer_camera->stop();
        capture_camera.release();
        writer.release();
        cv::destroyWindow("Acid Cam v2");
        file_new_capture->setEnabled(true);
        file_new_video->setEnabled(true);
        if(recording) {
            QString stream_;
            QTextStream stream(&stream_);
            stream << "Wrote video file: " << video_file_name << "\n";
            Log(stream_);
            file_size.close();
        }
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
    } else {
        controls_pause->setText("Pause");
        controls_pause->setChecked(Qt::Unchecked);
        paused = false;
    }
}

void AC_MainWindow::controls_SetImage() {
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open Image"), "/home", tr("Image Files (*.png *.jpg)"));
    if(fileName != "") {
        blend_image = cv::imread(fileName.toStdString());
        if(!blend_image.empty()) {
            blend_set = true;
            QMessageBox::information(this, tr("Loaded Image"), tr("Image set"));
        }
    }
}

void AC_MainWindow::controls_Step() {
    step_frame = true;
}

void AC_MainWindow::timer_Camera() {
    if(step_frame == true  && paused == true) {
        step_frame = false;
    }
    else if(paused == true) return;
    ac::isNegative = chk_negate->isChecked();
    ac::color_order = combo_rgb->currentIndex();
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
    
    cv::imshow("Acid Cam v2", mat);
    if(recording) {
        writer.write(mat);
        file_size.seekg(0, std::ios::end);
        file_pos = file_size.tellg();
    }
    
    frame_index++;
    QString frame_string;
    QTextStream frame_stream(&frame_string);
    
    frame_stream << "(Current/Total Frames/Seconds/Size) - (" << frame_index << "/" << video_frames << "/" << (frame_index/video_fps) << "/" << ((file_pos/1024)/1024) << " MB)";
    
    statusBar()->showMessage(frame_string);
    
}

void AC_MainWindow::timer_Video() {
    if(step_frame == true  && paused == true)
        step_frame = false;
    else if(paused == true) return;

    ac::isNegative = chk_negate->isChecked();
    ac::color_order = combo_rgb->currentIndex();
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
    cv::imshow("Acid Cam v2", mat);
    if(recording) {
        writer.write(mat);
        file_size.seekg(0, std::ios::end);
        file_pos = file_size.tellg();
    }
    
    frame_index++;
    QString frame_string;
    QTextStream frame_stream(&frame_string);
    
    frame_stream << "(Current/Total Frames/Seconds/Size) - (" << frame_index << "/" << video_frames << "/" << (frame_index/video_fps) << "/" << ((file_pos/1024)/1024) << " MB)";
    
    statusBar()->showMessage(frame_string);
}

void AC_MainWindow::help_About() {
    QMessageBox::information(this, tr("About Acid Cam"), tr("Written by <b>Jared Bruni</b><br><br><b>Social Media Accounts</b><br><br>\n\n <a href=\"http://github.com/lostjared\">GitHub</a><br>\n<a href=\"http://youtube.com/lostjared\">YouTube</a><br><a href=\"http://instagram.com/lostjared\">Instagram</a><br><a href=\"http://facebook.com/LostSideDead0x\">Facebook</a><br><br>\n"));
}

