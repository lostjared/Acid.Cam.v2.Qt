
/*
 * Acid Cam v2 - Qt/OpenCV Edition
 * written by Jared Bruni ( http://lostsidedead.com )
 * (C) 2017 GPL
 */


#include "new_dialog.h"
#include "main_window.h"


CaptureCamera::CaptureCamera(QWidget *parent) : QDialog(parent) {
    setWindowTitle(tr("Capture from Webcam"));
    setWindowIcon(QPixmap(":/images/icon.png"));
    createControls();
    adjustSize();
    setMinimumWidth(350);
}

void CaptureCamera::createControls() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QGridLayout *deviceGrid = new QGridLayout();
    
    QLabel *res = new QLabel(tr("Resolution:"), this);
    combo_res = new QComboBox(this);
    combo_res->addItem("640x480 (SD)");
    combo_res->addItem("1280x720 (HD)");
    combo_res->addItem("1920x1080 (Full HD)");
    deviceGrid->addWidget(res, 0, 0);
    deviceGrid->addWidget(combo_res, 0, 1);
    
    QLabel *dev = new QLabel(tr("Device:"), this);
    combo_device = new QComboBox(this);
    for(int i = 0; i < 10; ++i) {
        combo_device->addItem(QString::number(i));
    }
    deviceGrid->addWidget(dev, 1, 0);
    deviceGrid->addWidget(combo_device, 1, 1);
    mainLayout->addLayout(deviceGrid);
    QHBoxLayout *dirLayout = new QHBoxLayout();
    btn_select = new QPushButton(tr("Save Directory"), this);
    output_dir = new QLineEdit("", this);
    output_dir->setReadOnly(true);
    dirLayout->addWidget(btn_select);
    dirLayout->addWidget(output_dir, 1);
    mainLayout->addLayout(dirLayout);
    QGroupBox *opencvGroup = new QGroupBox(tr("OpenCV Encoding"), this);
    QHBoxLayout *opencvLayout = new QHBoxLayout(opencvGroup);
    video_type = new QComboBox(this);
    video_type->addItem(tr("MP4 - MPEG-4"));
    video_type->addItem(tr("MP4 - AVC/H.264"));
    video_type->addItem(tr("AVI - XviD"));
    opencvLayout->addWidget(video_type);
    mainLayout->addWidget(opencvGroup);
    
    QGroupBox *ffmpegGroup = new QGroupBox(tr("FFmpeg Encoding (Recommended)"), this);
    QVBoxLayout *ffmpegLayout = new QVBoxLayout(ffmpegGroup);
    
    chk_use_ffmpeg = new QCheckBox(tr("Use FFmpeg Encoder"), this);
    chk_use_ffmpeg->setChecked(true);
    ffmpegLayout->addWidget(chk_use_ffmpeg);
    
    QGridLayout *ffmpegGrid = new QGridLayout();
    
    QLabel *codecLabel = new QLabel(tr("Codec:"), this);
    ffmpeg_codec = new QComboBox(this);
    for (int i = 0; i < static_cast<int>(FFmpegCodec::CODEC_COUNT); ++i) {
        ffmpeg_codec->addItem(getCodecDescription(static_cast<FFmpegCodec>(i)));
    }
    if (ffmpeg_check_nvenc()) {
        ffmpeg_codec->setCurrentIndex(static_cast<int>(FFmpegCodec::H264_NVENC));
    }
    ffmpegGrid->addWidget(codecLabel, 0, 0);
    ffmpegGrid->addWidget(ffmpeg_codec, 0, 1);
    
    QLabel *crfLabel = new QLabel(tr("Quality (CRF):"), this);
    spin_crf = new QSpinBox(this);
    spin_crf->setRange(0, 51);
    spin_crf->setValue(23);
    spin_crf->setToolTip(tr("Lower = better quality, larger file. 18-23 recommended."));
    ffmpegGrid->addWidget(crfLabel, 1, 0);
    ffmpegGrid->addWidget(spin_crf, 1, 1);
    
    ffmpegLayout->addLayout(ffmpegGrid);
    mainLayout->addWidget(ffmpegGroup);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    chk_record = new QCheckBox(tr("Record"), this);
    buttonLayout->addWidget(chk_record);
    buttonLayout->addStretch();
    btn_start = new QPushButton(tr("Start"), this);
    btn_start->setMinimumWidth(100);
    buttonLayout->addWidget(btn_start);
    mainLayout->addLayout(buttonLayout);
    
    connect(btn_start, SIGNAL(clicked()), this, SLOT(btn_Start()));
    connect(btn_select, SIGNAL(clicked()), this, SLOT(btn_Select()));
    connect(chk_use_ffmpeg, SIGNAL(stateChanged(int)), this, SLOT(onUseFFmpegChanged(int)));
    
    onUseFFmpegChanged(Qt::Checked);
}

void CaptureCamera::onUseFFmpegChanged(int state) {
    bool useFFmpeg = (state == Qt::Checked);
    ffmpeg_codec->setEnabled(useFFmpeg);
    spin_crf->setEnabled(useFFmpeg);
    video_type->setEnabled(!useFFmpeg);
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
    int vtype = video_type->currentIndex();
    bool useFFmpeg = chk_use_ffmpeg->isChecked();
    FFmpegCodec codec = static_cast<FFmpegCodec>(ffmpeg_codec->currentIndex());
    int crf = spin_crf->value();
    
    if(output_dir->text().length() > 0) {
        if(win_parent->startCamera(combo_res->currentIndex(), combo_device->currentIndex(), 
                                   output_dir->text(), chk_record->isChecked(), vtype,
                                   useFFmpeg, codec, crf)) {
            hide();
        } else {
            QMessageBox::information(this, tr("Could not open Capture device"), 
                tr("Make sure your Webcam is plugged in. If you have more than one Webcam use the proper device index."));
        }
    } else {
        QMessageBox::information(this, tr("Error please fill out Save Directory"), 
            tr("Could not create Capture device requires Save Directory"));
    }
}

CaptureVideo::CaptureVideo(QWidget *parent) : QDialog(parent) {
    setWindowTitle(tr("Capture from Video"));
    setWindowIcon(QPixmap(":/images/icon.png"));
    settings = new QSettings("LostSideDead", "Acid Cam Qt");
    createControls();
    adjustSize();
    setMinimumWidth(400);
}

CaptureVideo::~CaptureVideo() {
}

void CaptureVideo::createControls() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Source file selection
    QHBoxLayout *srcLayout = new QHBoxLayout();
    btn_setedit = new QPushButton(tr("Source File"), this);
    edit_src = new QLineEdit(this);
    edit_src->setReadOnly(true);
    srcLayout->addWidget(btn_setedit);
    srcLayout->addWidget(edit_src, 1);
    mainLayout->addLayout(srcLayout);
    
    QHBoxLayout *outLayout = new QHBoxLayout();
    btn_setout = new QPushButton(tr("Set Output"), this);
    edit_outdir = new QLineEdit(this);
    edit_outdir->setReadOnly(true);
    outLayout->addWidget(btn_setout);
    outLayout->addWidget(edit_outdir, 1);
    mainLayout->addLayout(outLayout);
    
    QGroupBox *opencvGroup = new QGroupBox(tr("OpenCV Encoding"), this);
    QHBoxLayout *opencvLayout = new QHBoxLayout(opencvGroup);
    video_type = new QComboBox(this);
    video_type->addItem(tr("MP4 - MPEG-4"));
    video_type->addItem(tr("MP4 - AVC/H.264"));
    video_type->addItem(tr("AVI - XviD"));
    opencvLayout->addWidget(video_type);
    mainLayout->addWidget(opencvGroup);
    
    QGroupBox *ffmpegGroup = new QGroupBox(tr("FFmpeg Encoding (Recommended)"), this);
    QVBoxLayout *ffmpegLayout = new QVBoxLayout(ffmpegGroup);
    
    chk_use_ffmpeg = new QCheckBox(tr("Use FFmpeg Encoder"), this);
    chk_use_ffmpeg->setChecked(true);
    ffmpegLayout->addWidget(chk_use_ffmpeg);
    
    QGridLayout *ffmpegGrid = new QGridLayout();
    
    QLabel *codecLabel = new QLabel(tr("Codec:"), this);
    ffmpeg_codec = new QComboBox(this);
    for (int i = 0; i < static_cast<int>(FFmpegCodec::CODEC_COUNT); ++i) {
        ffmpeg_codec->addItem(getCodecDescription(static_cast<FFmpegCodec>(i)));
    }
    if (ffmpeg_check_nvenc()) {
        ffmpeg_codec->setCurrentIndex(static_cast<int>(FFmpegCodec::H264_NVENC));
    }
    ffmpegGrid->addWidget(codecLabel, 0, 0);
    ffmpegGrid->addWidget(ffmpeg_codec, 0, 1);
    
    QLabel *crfLabel = new QLabel(tr("Quality (CRF):"), this);
    spin_crf = new QSpinBox(this);
    spin_crf->setRange(0, 51);
    spin_crf->setValue(23);
    spin_crf->setToolTip(tr("Lower = better quality, larger file. 18-23 recommended."));
    ffmpegGrid->addWidget(crfLabel, 1, 0);
    ffmpegGrid->addWidget(spin_crf, 1, 1);
    
    ffmpegLayout->addLayout(ffmpegGrid);
    
    chk_mux_audio = new QCheckBox(tr("Copy audio from source video"), this);
    chk_mux_audio->setChecked(true);
    ffmpegLayout->addWidget(chk_mux_audio);
    
    mainLayout->addWidget(ffmpegGroup);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    chk_record = new QCheckBox(tr("Record"), this);
    chk_png = new QCheckBox(tr("PNG Frames"), this);
    buttonLayout->addWidget(chk_record);
    buttonLayout->addWidget(chk_png);
    buttonLayout->addStretch();
    btn_start = new QPushButton(tr("Start"), this);
    btn_start->setMinimumWidth(100);
    buttonLayout->addWidget(btn_start);
    mainLayout->addLayout(buttonLayout);
    
    connect(btn_setedit, SIGNAL(clicked()), this, SLOT(btn_SetSourceFile()));
    connect(btn_setout, SIGNAL(clicked()), this, SLOT(btn_SetOutputDir()));
    connect(btn_start, SIGNAL(clicked()), this, SLOT(btn_Start()));
    connect(chk_use_ffmpeg, SIGNAL(stateChanged(int)), this, SLOT(onUseFFmpegChanged(int)));
    
    onUseFFmpegChanged(Qt::Checked);
}

void CaptureVideo::onUseFFmpegChanged(int state) {
    bool useFFmpeg = (state == Qt::Checked);
    ffmpeg_codec->setEnabled(useFFmpeg);
    spin_crf->setEnabled(useFFmpeg);
    chk_mux_audio->setEnabled(useFFmpeg);
    video_type->setEnabled(!useFFmpeg);
}

void CaptureVideo::setParent(AC_MainWindow *p) {
    win_parent = p;
}

void CaptureVideo::btn_SetSourceFile() {
    QString def_path = settings->value("dir_path", "").toString();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Video"), def_path, 
        tr("Video Files (*.avi *.mov *.mp4 *.mkv *.m4v *.webm *.flv)"));
    if(fileName != "") {
        edit_src->setText(fileName);
        
        std::string val = fileName.toStdString();
        auto pos = val.rfind("/");
        if(pos == std::string::npos)
            pos = val.rfind("\\");
        if(pos != std::string::npos) {
            val = val.substr(0, pos);
        }
        
        settings->setValue("dir_path", val.c_str());
    }
}

void CaptureVideo::btn_SetOutputDir() {
    QString def_path = "";
    QString dir = QFileDialog::getExistingDirectory(this, tr("Set Output Directory"), def_path,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
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
    
    int num = video_type->currentIndex();
    bool useFFmpeg = chk_use_ffmpeg->isChecked();
    FFmpegCodec codec = static_cast<FFmpegCodec>(ffmpeg_codec->currentIndex());
    int crf = spin_crf->value();
    bool muxAudio = chk_mux_audio->isChecked();
    
    if(win_parent->startVideo(edit_src->text(), edit_outdir->text(), 
                              chk_record->isChecked(), chk_png->isChecked(), num,
                              useFFmpeg, codec, crf, muxAudio)) {
        hide();
    } else {
        QMessageBox::information(this, tr("Could not open file"), 
            tr("Could not open video file, an error has occurred"));
    }
}


