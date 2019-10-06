
/*
 * Acid Cam v2 - Qt/OpenCV Edition
 * written by Jared Bruni ( http://lostsidedead.com )
 * (C) 2017 GPL
 */


#include "new_dialog.h"
#include "main_window.h"


CaptureCamera::CaptureCamera(QWidget *parent) : QDialog(parent) {
    setGeometry(100, 100, 290, 120);
    setFixedSize(290, 120);
    setWindowTitle(tr("Capture from Webcam"));
    setWindowIcon(QPixmap(":/images/icon.png"));
    createControls();
}

void CaptureCamera::createControls() {
    QLabel *res = new QLabel(tr("Resolution: "), this);
    res->setGeometry(10, 10, 75, 20);
    combo_res = new QComboBox(this);
    combo_res->setGeometry(85, 10, 200, 25);
    combo_res->addItem("640x480 (SD)");
    combo_res->addItem("1280x720 (HD)");
    combo_res->addItem("1920x1080 (Full HD)");
    QLabel *dev = new QLabel(tr("Device: "), this);
    dev->setGeometry(10, 35, 50, 20);
    combo_device = new QComboBox(this);
    combo_device->setGeometry(85, 35, 200, 25);
    for(int i = 0; i < 10; ++i) {
        QString s;
        QTextStream stream(&s);
        stream << i;
        combo_device->addItem(*stream.string());
    }
    btn_select = new QPushButton(tr("Save Directory"), this);
    btn_select->setGeometry(10, 65, 100, 20);
    output_dir = new QLineEdit("", this);
    output_dir->setGeometry(110, 65, 175, 20);
    output_dir->setReadOnly(true);
    chk_record = new QCheckBox(tr("Record"), this);
    chk_record->setGeometry(10, 95, 70, 20);
    btn_start = new QPushButton(tr("Start"), this);
    btn_start->setGeometry(185, 95, 100, 20);
    connect(btn_start, SIGNAL(clicked()), this, SLOT(btn_Start()));
    connect(btn_select, SIGNAL(clicked()), this, SLOT(btn_Select()));
    
    video_type = new QComboBox(this);
    video_type->setGeometry(80, 90, 90, 25);
    video_type->addItem("MP4 - MPEG-4");
    video_type->addItem("MP4 - AVC");
    video_type->addItem("MP4 - HEVC");
    video_type->addItem("AVI - XviD");
}

void CaptureCamera::setParent(AC_MainWindow *p) {
    win_parent = p;
}

void CaptureCamera::btn_Select() {
    
    QString def_path = "";
#if defined(__linux__)
    def_path = "";
#elif defined(__APPLE__)
    def_path = "/Users";
#elif defined(_WIN32)
    def_path = "C:\\";
#endif
    
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),def_path,QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    
    if(dir != "") {
        output_dir->setText(dir);
    }
}

void CaptureCamera::btn_Start() {
    
    int vtype;
    vtype = video_type->currentIndex();
    
    if(output_dir->text().length() > 0) {
        if(win_parent->startCamera(combo_res->currentIndex(), combo_device->currentIndex(), output_dir->text(), chk_record->isChecked(), vtype)) {
            hide();
            
        } else {
            QMessageBox::information(this, tr("Could not open Capture device"), tr("Make sure you Webcam is pluged in. If you have more than one Webcam use the proper device index."));
        }
    } else {
        QMessageBox::information(this, tr("Error please fill out Save Directory"), tr("Could not create Capture device requires Save Directory"));
    }
}

CaptureVideo::CaptureVideo(QWidget *parent) : QDialog(parent) {
    setGeometry(100, 100, 330, 100);
    setFixedSize(330, 100);
    setWindowTitle(tr("Capture from Video"));
    setWindowIcon(QPixmap(":/images/icon.png"));
    createControls();
}

void CaptureVideo::createControls() {
    btn_setedit = new QPushButton(tr("Source File"), this);
    btn_setedit->setGeometry(10, 10, 110, 20);
    edit_src = new QLineEdit(this);
    edit_src->setGeometry(120, 10, 200, 20);
    edit_src->setReadOnly(true);
    btn_setout = new QPushButton(tr("Set Output"), this);
    btn_setout->setGeometry(10, 30, 110, 20);
    edit_outdir = new QLineEdit(this);
    edit_outdir->setGeometry(120, 30, 200, 20);
    edit_outdir->setReadOnly(true);
    btn_start = new QPushButton(tr("Start"), this);
    btn_start->setGeometry(10, 60, 100, 20);
    chk_record = new QCheckBox(tr("Record"), this);
    chk_record->setGeometry(110, 60, 80, 20);
    
    video_type = new QComboBox(this);
    video_type->setGeometry(180, 55, 120, 25);
    video_type->addItem("MP4 - MPEG-4");
    video_type->addItem("MP4 - AVC");
    video_type->addItem("MP4 - HEVC");
    video_type->addItem("AVI - XviD");
    connect(btn_setedit, SIGNAL(clicked()), this, SLOT(btn_SetSourceFile()));
    connect(btn_setout, SIGNAL(clicked()), this, SLOT(btn_SetOutputDir()));
    connect(btn_start, SIGNAL(clicked()), this, SLOT(btn_Start()));
}

void CaptureVideo::setParent(AC_MainWindow *p) {
    win_parent = p;
}

void CaptureVideo::btn_SetSourceFile() {
    QString def_path = "";
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open Video"), def_path, tr("Video Files (*.avi *.mov *.mp4 *.mkv)"));
    if(fileName != "")
        edit_src->setText(fileName);
}

void CaptureVideo::btn_SetOutputDir() {
    QString def_path = "";
    QString dir = QFileDialog::getExistingDirectory(this, tr("Set Output Directory"),def_path,QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(dir != "")
        edit_outdir->setText(dir);
}

void CaptureVideo::btn_Start() {
    if(edit_src->text().length() <= 0) {
        QMessageBox::information(this, tr("No Input"), tr("Please Select a Video File"));
        return;
    }
    if(edit_outdir->text().length() <= 0) {
        QMessageBox::information(this, tr("No Output"), tr("Please Select Output Directory"));
        return;
    }
    
    int num;
    num = video_type->currentIndex();
    
    if(win_parent->startVideo(edit_src->text(), edit_outdir->text(), chk_record->isChecked(), num)) {
        hide();
    } else {
        QMessageBox::information(this, tr("Could not open file"), tr("Could not open video file, an error has occured"));
    }
}


