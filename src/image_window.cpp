#include "image_window.h"

ImageWindow::ImageWindow(QWidget *parent) : QDialog(parent) {
    setFixedSize(800, 400);
    setWindowTitle(tr("Acid Cam v2 - Image Manager"));
    createControls();
    settings = new QSettings();
}

void ImageWindow::createControls() {
    // create
    image_files = new QListWidget(this);
    image_files->setGeometry(20,20,(770/2)-10,300);
    add_files = new QPushButton(tr("Add"), this);
    add_files->setGeometry(15,325,100,25);
    rmv_file = new QPushButton(tr("Remove"), this);
    rmv_file->setGeometry(115,325,100,25);
    set_file = new QPushButton(tr("Set Image"), this);
    set_file->setGeometry(685,325,100,25);
    image_set_cycle = new QPushButton(tr("Set Cycle"), this);
    image_set_cycle->setGeometry(590, 325, 100, 25);
    image_cycle_on = new QCheckBox("Image Cycle", this);
    image_cycle_on->setGeometry(220, 325, 100, 25);
    image_cycle = new QComboBox(this);
    image_cycle->setGeometry(320, 325, 100, 25);
    image_cycle->addItem(tr("Random"));
    image_cycle->addItem(tr("In Order"));
    image_cycle->addItem(tr("Shuffle"));
    image_frames = new QLineEdit("120", this);
    image_frames->setGeometry(425, 325, 100, 25);
    image_pic = new QLabel("", this);
    image_pic->setGeometry((770/2)+10, 20, 770/2,300);
    image_pic->setStyleSheet("QLabel{background: black; color: #000000;}");
    // connect
    connect(add_files, SIGNAL(clicked()), this, SLOT(image_AddFiles()));
    connect(rmv_file, SIGNAL(clicked()), this, SLOT(image_RmvFile()));
    connect(set_file, SIGNAL(clicked()), this, SLOT(image_SetFile()));
    connect(image_set_cycle, SIGNAL(clicked()), this, SLOT(image_SetCycle()));
    connect(image_files, SIGNAL(currentRowChanged(int)), this, SLOT(image_RowChanged(int)));
    //image_files->addItem("TEST!!");
    btn_setvideo = new QPushButton(tr("Set Video"), this);
    btn_setvideo->setGeometry(15, 360, 100, 25);
    
    btn_clear = new QPushButton(tr("Clear Video"), this);
    btn_clear->setGeometry(115, 360, 100, 25);
    
    lbl_video = new QLabel(tr("Video Not Set "), this);
    lbl_video->setGeometry(215, 360, 800-360, 25);
    
    connect(btn_setvideo, SIGNAL(clicked()), this, SLOT(video_Set()));
    connect(btn_clear, SIGNAL(clicked()), this, SLOT(video_Clr()));
    
}

void ImageWindow::image_AddFiles() {
    QString dir_path = settings->value("dir_path_image", "").toString();
    
    QStringList files = QFileDialog::getOpenFileNames(this,"Select one or more files to open",dir_path,"Images (*.png *.jpg)");
    QString value;
    for(int i = 0; i < files.size(); ++i) {
        image_files->addItem(files.at(i));
        value = files.at(i);
    }
    settings->setValue("dir_path_image", value);
}

void ImageWindow::image_RmvFile() {
    int index = image_files->currentRow();
    if(index >= 0) {
        image_files->takeItem(index);
    }
}

void ImageWindow::image_SetFile() {
    int index = image_files->currentRow();
    if(index >= 0) {
        QListWidgetItem *i = image_files->item(index);
        std::string file_name = i->text().toStdString();
        cv::Mat image_value;
        image_value = cv::imread(file_name);
        if(!image_value.empty()) {
            playback->setImage(image_value);
        }
    }
}

void ImageWindow::image_RowChanged(int index) {
    if(index >= 0) {
        QListWidgetItem *i = image_files->item(index);
        QPixmap p(i->text());
        int w = image_pic->width();
        int h = image_pic->height();
        image_pic->setPixmap(p.scaled(w,h,Qt::KeepAspectRatio));
    }
}

void ImageWindow::image_SetCycle() {
    QString text_value;
    QTextStream stream(&text_value);
    if(image_files->count() < 2 || !image_cycle_on->isChecked()) {
        playback->setCycle(0);
        stream << "Cycle Turned Off.\n";
        blend_set = false;
    } else {
        std::vector<std::string> text_items;
        for(int i = 0; i < image_files->count(); ++i) {
            text_items.push_back(image_files->item(i)->text().toStdString());
        }
        int type = image_cycle->currentIndex();
        QString fvalue = image_frames->text();
        int val = atoi(fvalue.toStdString().c_str());
        if(val <= 0 || type < 0) {
            stream << "Invalid Frame Count/Type Index\n";
        } else {
            QString im_cycle = image_cycle->itemText(type);
            stream << "Image Frames: " << text_items.size() << " Cycle Type: " << im_cycle << " every " << val << " frames.\n";
            playback->setCycle(type+1, val, text_items);
        }
    }
    QMessageBox::information(this, "Image Cycle", text_value);
}

void ImageWindow::setPlayback(Playback *play) {
    playback = play;
}

void ImageWindow::video_Set() {
    
    QString dir_path = settings->value("dir_vid_path", "").toString();
    
    
    QString file_name = QFileDialog::getOpenFileName(this,"Select A video file to open",dir_path,"Video (*.avi *.mov *.mp4 *.mkv *.m4v)");
    
    if(file_name != "")
        settings->setValue("dir_vid_path",file_name);
    else return;
    
    
    std::string vname = file_name.toStdString();
    ac::v_cap.open(vname);
    if(ac::v_cap.isOpened() == false) {
        QMessageBox::information(this, "Error could not open file", "File not supported");
        return;
    }
    lbl_video->setText("File Opened and Active");
}

void ImageWindow::video_Clr() {
    if(ac::v_cap.isOpened()) {
        ac::v_cap.release();
        lbl_video->setText("Video closed..");
    }
}
