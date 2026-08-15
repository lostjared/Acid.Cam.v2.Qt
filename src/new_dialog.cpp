
/*
 * Acid Cam v2 - Qt/OpenCV Edition
 * written by Jared Bruni ( http://lostsidedead.com )
 * (C) 2017 GPL
 */


#include "new_dialog.h"
#include "main_window.h"
#include "MXWrite/mxwrite.hpp"
#include <QHeaderView>
#include <QInputDialog>
#include <QTreeWidget>

namespace {
void populatePresetAndTuneControls(QComboBox *preset, QComboBox *tune) {
    preset->addItems({"ultrafast", "superfast", "veryfast", "faster",
                      "fast",      "medium",    "slow",     "slower",
                      "veryslow",  "p1",        "p2",       "p3",
                      "p4",        "p5",        "p6",       "p7"});
    preset->setToolTip(
        QObject::tr("Software presets run from ultrafast to veryslow. "
                    "NVENC presets run from p1 (fastest) to p7 (best quality)."));

    tune->addItems({"none",       "film", "animation", "grain",
                    "stillimage", "psnr", "ssim",      "fastdecode",
                    "zerolatency", "hq",  "uhq",       "ll",
                    "ull",        "lossless"});
    tune->setToolTip(
        QObject::tr("Content tunes apply to software encoders. hq/uhq/ll/ull "
                    "apply to NVENC; lossless is translated per codec."));
}

FFmpegEncodeOptions selectedEncodeOptions(QSpinBox *quality,
                                          QComboBox *preset,
                                          QComboBox *tune,
                                          QComboBox *codec,
                                          QLineEdit *parameters,
                                          QCheckBox *realtime) {
    FFmpegEncodeOptions options;
    options.quality = quality->value();
    options.preset = preset->currentText().toStdString();
    options.tune = tune->currentText().toStdString();
    options.codec = codec->currentData().toString().toStdString();
    options.ffmpegOptions = parameters->text().trimmed().toStdString();
    options.realtime = realtime->isChecked();
    return options;
}

QString legacyCodecName(int index) {
    static const QStringList names = {"libx264", "libx265", "h264_nvenc",
                                      "hevc_nvenc", "h264_vaapi",
                                      "hevc_vaapi"};
    return index >= 0 && index < names.size() ? names.at(index) : "auto";
}

void populateVideoEncoders(QComboBox *combo, const QString &savedEncoder) {
    combo->clear();
    combo->addItem(QObject::tr("Automatic (NVENC, then software)"), "auto");
    combo->addItem(QObject::tr("Automatic software H.264/H.265"), "software");
    combo->addItem(QObject::tr("Automatic NVIDIA NVENC"), "nvenc");

    for(const mx::EncoderInfo &encoder : mx::available_video_encoders()) {
        const QString name = QString::fromStdString(encoder.name);
        const QString longName = QString::fromStdString(encoder.long_name);
        const QString backend = encoder.hardware ? QObject::tr("hardware")
                                                 : QObject::tr("software");
        combo->addItem(QString("%1 — %2 [%3]").arg(name, longName, backend),
                       name);
        const int index = combo->count() - 1;
        combo->setItemData(index, longName, Qt::UserRole + 1);
        combo->setItemData(index, QString::fromStdString(encoder.codec_name),
                           Qt::UserRole + 2);
        combo->setItemData(index, backend, Qt::UserRole + 3);
        combo->setItemData(index,
                           encoder.experimental ? QObject::tr("experimental")
                                                : QObject::tr("stable"),
                           Qt::UserRole + 4);
        combo->setItemData(index,
                           QString::fromStdString(encoder.pixel_formats),
                           Qt::UserRole + 5);
    }

    int savedIndex = combo->findData(savedEncoder);
    if(savedIndex < 0 && !savedEncoder.isEmpty()) {
        combo->addItem(QObject::tr("%1 — unavailable in this FFmpeg build")
                           .arg(savedEncoder),
                       savedEncoder);
        savedIndex = combo->count() - 1;
    }
    combo->setCurrentIndex(savedIndex >= 0 ? savedIndex : 0);
}

bool isExactEncoder(const QComboBox *combo) {
    const QString name = combo->currentData().toString();
    return name != "auto" && name != "software" && name != "nvenc";
}

void updateEncoderDetails(QComboBox *combo, QLabel *details,
                          QPushButton *optionsButton) {
    const bool exact = isExactEncoder(combo);
    optionsButton->setEnabled(exact && combo->isEnabled());
    const QString description = combo->currentData(Qt::UserRole + 1).toString();
    if(description.isEmpty()) {
        details->setText(exact
                             ? QObject::tr("Exact FFmpeg encoder: %1")
                                   .arg(combo->currentData().toString())
                             : QObject::tr("MXWrite selects the concrete encoder at startup."));
        return;
    }
    const QString codec = combo->currentData(Qt::UserRole + 2).toString();
    const QString backend = combo->currentData(Qt::UserRole + 3).toString();
    QString formats = combo->currentData(Qt::UserRole + 5).toString();
    if(formats.isEmpty())
        formats = QObject::tr("encoder-defined");
    details->setText(QObject::tr("%1 | codec: %2 | %3 | pixel formats: %4")
                         .arg(description, codec, backend, formats));
}

void showEncoderOptions(QWidget *parent, QComboBox *combo,
                        QLineEdit *parameters) {
    const QString encoderName = combo->currentData().toString();
    if(!isExactEncoder(combo))
        return;

    QDialog dialog(parent);
    dialog.setWindowTitle(QObject::tr("%1 Encoder Options").arg(encoderName));
    dialog.resize(1000, 560);
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QLabel *summary = new QLabel(
        QObject::tr("Double-click an option to append it to Extra FFmpeg parameters."),
        &dialog);
    layout->addWidget(summary);

    QTreeWidget *tree = new QTreeWidget(&dialog);
    tree->setColumnCount(6);
    tree->setHeaderLabels({QObject::tr("Option"), QObject::tr("Type"),
                           QObject::tr("Default"), QObject::tr("Range"),
                           QObject::tr("Named values"),
                           QObject::tr("Description")});
    tree->setRootIsDecorated(false);
    tree->setWordWrap(true);
    const auto options =
        mx::video_encoder_options(encoderName.toStdString());
    for(const mx::EncoderOptionInfo &option : options) {
        QString range;
        if(!option.minimum.empty() || !option.maximum.empty()) {
            range = QString("%1 … %2")
                        .arg(QString::fromStdString(option.minimum),
                             QString::fromStdString(option.maximum));
        }
        new QTreeWidgetItem(
            tree, {"-" + QString::fromStdString(option.name),
                   QString::fromStdString(option.type),
                   QString::fromStdString(option.default_value), range,
                   QString::fromStdString(option.choices),
                   QString::fromStdString(option.help)});
    }
    if(tree->topLevelItemCount() == 0)
        new QTreeWidgetItem(tree, {QObject::tr("(No private video AVOptions reported)")});
    tree->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    tree->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    tree->header()->setSectionResizeMode(5, QHeaderView::Stretch);
    layout->addWidget(tree);

    QObject::connect(tree, &QTreeWidget::itemDoubleClicked, &dialog,
                     [parameters, &dialog](QTreeWidgetItem *item, int) {
        const QString optionName = item->text(0);
        if(!optionName.startsWith('-'))
            return;
        bool accepted = false;
        QString value = QInputDialog::getText(
            &dialog, QObject::tr("Set Encoder Option"),
            QObject::tr("%1 value:").arg(optionName), QLineEdit::Normal,
            item->text(2), &accepted);
        if(!accepted)
            return;
        if(value.contains(' ') || value.contains('\t') || value.contains('"')) {
            value.replace('\\', "\\\\");
            value.replace('"', "\\\"");
            value = '"' + value + '"';
        }
        QString current = parameters->text().trimmed();
        if(!current.isEmpty())
            current += ' ';
        parameters->setText(current + optionName + ' ' + value);
    });
    dialog.exec();
}
}


CaptureCamera::CaptureCamera(QWidget *parent) : QDialog(parent) {
    setWindowTitle(tr("Capture from Webcam"));
    setWindowIcon(QPixmap(":/images/icon.png"));
    settings = new QSettings("LostSideDead", "Acid Cam Qt", this);
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

    QLabel *fpsLabel = new QLabel(tr("Camera FPS:"), this);
    combo_fps = new QComboBox(this);
    combo_fps->addItems({"24", "30", "60"});
    const QString savedFps =
        settings->value("camera/capture_fps", "30").toString();
    const int savedFpsIndex = combo_fps->findText(savedFps);
    combo_fps->setCurrentIndex(savedFpsIndex >= 0 ? savedFpsIndex : 1);
    combo_fps->setToolTip(
        tr("Requested camera capture rate. The camera must support the selected "
           "resolution and FPS combination."));
    deviceGrid->addWidget(fpsLabel, 2, 0);
    deviceGrid->addWidget(combo_fps, 2, 1);
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
    const QString savedCodec = settings->contains("camera/ffmpeg_codec_name")
                                   ? settings->value("camera/ffmpeg_codec_name").toString()
                               : settings->contains("camera/ffmpeg_codec")
                                   ? legacyCodecName(settings->value(
                                         "camera/ffmpeg_codec").toInt())
                                   : QString("auto");
    populateVideoEncoders(ffmpeg_codec, savedCodec);
    ffmpegGrid->addWidget(codecLabel, 0, 0);
    ffmpegGrid->addWidget(ffmpeg_codec, 0, 1);

    encoder_details = new QLabel(this);
    encoder_details->setWordWrap(true);
    encoder_details->setTextInteractionFlags(Qt::TextSelectableByMouse);
    ffmpegGrid->addWidget(encoder_details, 1, 0, 1, 2);
    encoder_options = new QPushButton(tr("Show Encoder Options..."), this);
    ffmpegGrid->addWidget(encoder_options, 2, 1);
    
    QLabel *crfLabel = new QLabel(tr("Quality (CRF/CQ/QP):"), this);
    spin_crf = new QSpinBox(this);
    spin_crf->setRange(0, 51);
    spin_crf->setValue(settings->value("camera/ffmpeg_quality", 18).toInt());
    spin_crf->setToolTip(tr("Lower = better quality and a larger file. 18-23 is recommended."));
    ffmpegGrid->addWidget(crfLabel, 3, 0);
    ffmpegGrid->addWidget(spin_crf, 3, 1);

    ffmpeg_preset = new QComboBox(this);
    ffmpeg_tune = new QComboBox(this);
    populatePresetAndTuneControls(ffmpeg_preset, ffmpeg_tune);
    ffmpeg_preset->setCurrentText(settings->value("camera/ffmpeg_preset", "medium").toString());
    ffmpeg_tune->setCurrentText(settings->value("camera/ffmpeg_tune", "none").toString());
    ffmpegGrid->addWidget(new QLabel(tr("Preset:"), this), 4, 0);
    ffmpegGrid->addWidget(ffmpeg_preset, 4, 1);
    ffmpegGrid->addWidget(new QLabel(tr("Tune:"), this), 5, 0);
    ffmpegGrid->addWidget(ffmpeg_tune, 5, 1);

    ffmpeg_parameters = new QLineEdit(
        settings->value("camera/ffmpeg_parameters", "").toString(), this);
    ffmpeg_parameters->setPlaceholderText(
        tr("-profile:v high -level 4.1 -pix_fmt yuv420p"));
    ffmpeg_parameters->setToolTip(
        tr("Additional FFmpeg-style encoder options passed to MXWrite. "
           "Do not include input or output filenames."));
    ffmpegGrid->addWidget(new QLabel(tr("Extra FFmpeg parameters:"), this),
                          6, 0);
    ffmpegGrid->addWidget(ffmpeg_parameters, 6, 1);

    chk_realtime = new QCheckBox(tr("Realtime (low latency)"), this);
    chk_realtime->setChecked(settings->value("camera/ffmpeg_realtime", true).toBool());
    chk_realtime->setToolTip(
        tr("Reduces encoder buffering for live camera capture and overrides "
           "the selected tune with a low-latency tune."));
    ffmpegGrid->addWidget(chk_realtime, 7, 0, 1, 2);

    chk_timestamp_frames = new QCheckBox(
        tr("Timestamp frames using capture time"), this);
    chk_timestamp_frames->setChecked(
        settings->value("camera/ffmpeg_timestamp_frames", true).toBool());
    chk_timestamp_frames->setToolTip(
        tr("Use wall-clock timestamps and variable frame timing to prevent "
           "camera recordings from drifting when processing slows down."));
    ffmpegGrid->addWidget(chk_timestamp_frames, 8, 0, 1, 2);
    
    ffmpegLayout->addLayout(ffmpegGrid);
    mainLayout->addWidget(ffmpegGroup);

    chk_sync_fps = new QCheckBox(tr("Sync processing to camera FPS"), this);
    chk_sync_fps->setChecked(
        settings->value("camera/sync_processing_fps", true).toBool());
    chk_sync_fps->setToolTip(
        tr("Limit processing to the frame rate reported by the camera."));
    mainLayout->addWidget(chk_sync_fps);
    
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
    connect(ffmpeg_codec, SIGNAL(currentIndexChanged(int)), this, SLOT(onCodecChanged(int)));
    connect(encoder_options, &QPushButton::clicked, this, [this]() {
        showEncoderOptions(this, ffmpeg_codec, ffmpeg_parameters);
    });
    
    onUseFFmpegChanged(Qt::Checked);
    onCodecChanged(ffmpeg_codec->currentIndex());
}

void CaptureCamera::onUseFFmpegChanged(int state) {
    bool useFFmpeg = (state == Qt::Checked);
    ffmpeg_codec->setEnabled(useFFmpeg);
    spin_crf->setEnabled(useFFmpeg);
    ffmpeg_preset->setEnabled(useFFmpeg);
    ffmpeg_tune->setEnabled(useFFmpeg);
    ffmpeg_parameters->setEnabled(useFFmpeg);
    encoder_details->setEnabled(useFFmpeg);
    updateEncoderDetails(ffmpeg_codec, encoder_details, encoder_options);
    chk_realtime->setEnabled(useFFmpeg);
    chk_timestamp_frames->setEnabled(useFFmpeg);
    video_type->setEnabled(!useFFmpeg);
}

void CaptureCamera::onCodecChanged(int index) {
    (void)index;
    updateEncoderDetails(ffmpeg_codec, encoder_details, encoder_options);
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
    const FFmpegEncodeOptions options = selectedEncodeOptions(
        spin_crf, ffmpeg_preset, ffmpeg_tune, ffmpeg_codec,
        ffmpeg_parameters, chk_realtime);
    FFmpegEncodeOptions cameraOptions = options;
    cameraOptions.timestampInput = chk_timestamp_frames->isChecked();
    settings->setValue("camera/ffmpeg_codec_name", ffmpeg_codec->currentData());
    settings->setValue("camera/ffmpeg_quality", options.quality);
    settings->setValue("camera/ffmpeg_preset", ffmpeg_preset->currentText());
    settings->setValue("camera/ffmpeg_tune", ffmpeg_tune->currentText());
    settings->setValue("camera/ffmpeg_parameters", ffmpeg_parameters->text().trimmed());
    settings->setValue("camera/ffmpeg_realtime", options.realtime);
    settings->setValue("camera/ffmpeg_timestamp_frames",
                       cameraOptions.timestampInput);
    settings->setValue("camera/sync_processing_fps", chk_sync_fps->isChecked());
    settings->setValue("camera/capture_fps", combo_fps->currentText());
    
    if(output_dir->text().length() > 0) {
        if(win_parent->startCamera(combo_res->currentIndex(), combo_device->currentIndex(), 
                                   output_dir->text(), chk_record->isChecked(), vtype,
                                   useFFmpeg, cameraOptions,
                                   chk_sync_fps->isChecked(),
                                   combo_fps->currentText().toInt())) {
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
    settings = new QSettings("LostSideDead", "Acid Cam Qt", this);
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
    const QString savedCodec = settings->contains("video/ffmpeg_codec_name")
                                   ? settings->value("video/ffmpeg_codec_name").toString()
                               : settings->contains("video/ffmpeg_codec")
                                   ? legacyCodecName(settings->value(
                                         "video/ffmpeg_codec").toInt())
                                   : QString("auto");
    populateVideoEncoders(ffmpeg_codec, savedCodec);
    ffmpegGrid->addWidget(codecLabel, 0, 0);
    ffmpegGrid->addWidget(ffmpeg_codec, 0, 1);

    encoder_details = new QLabel(this);
    encoder_details->setWordWrap(true);
    encoder_details->setTextInteractionFlags(Qt::TextSelectableByMouse);
    ffmpegGrid->addWidget(encoder_details, 1, 0, 1, 2);
    encoder_options = new QPushButton(tr("Show Encoder Options..."), this);
    ffmpegGrid->addWidget(encoder_options, 2, 1);
    
    QLabel *crfLabel = new QLabel(tr("Quality (CRF/CQ/QP):"), this);
    spin_crf = new QSpinBox(this);
    spin_crf->setRange(0, 51);
    spin_crf->setValue(settings->value("video/ffmpeg_quality", 18).toInt());
    spin_crf->setToolTip(tr("Lower = better quality, larger file. 18-23 recommended."));
    ffmpegGrid->addWidget(crfLabel, 3, 0);
    ffmpegGrid->addWidget(spin_crf, 3, 1);

    ffmpeg_preset = new QComboBox(this);
    ffmpeg_tune = new QComboBox(this);
    populatePresetAndTuneControls(ffmpeg_preset, ffmpeg_tune);
    ffmpeg_preset->setCurrentText(settings->value("video/ffmpeg_preset", "medium").toString());
    ffmpeg_tune->setCurrentText(settings->value("video/ffmpeg_tune", "none").toString());
    ffmpegGrid->addWidget(new QLabel(tr("Preset:"), this), 4, 0);
    ffmpegGrid->addWidget(ffmpeg_preset, 4, 1);
    ffmpegGrid->addWidget(new QLabel(tr("Tune:"), this), 5, 0);
    ffmpegGrid->addWidget(ffmpeg_tune, 5, 1);

    ffmpeg_parameters = new QLineEdit(
        settings->value("video/ffmpeg_parameters", "").toString(), this);
    ffmpeg_parameters->setPlaceholderText(
        tr("-profile:v high -level 4.1 -pix_fmt yuv420p"));
    ffmpeg_parameters->setToolTip(
        tr("Additional FFmpeg-style encoder options passed to MXWrite. "
           "Do not include input or output filenames."));
    ffmpegGrid->addWidget(new QLabel(tr("Extra FFmpeg parameters:"), this),
                          6, 0);
    ffmpegGrid->addWidget(ffmpeg_parameters, 6, 1);

    chk_realtime = new QCheckBox(tr("Realtime (low latency)"), this);
    chk_realtime->setChecked(settings->value("video/ffmpeg_realtime", false).toBool());
    chk_realtime->setToolTip(
        tr("Reduces buffering and overrides the selected tune with a "
           "low-latency tune; normally only needed for live input."));
    ffmpegGrid->addWidget(chk_realtime, 7, 0, 1, 2);

    chk_no_drop = new QCheckBox(tr("No Drop (pace processing to encoder)"),
                                this);
    chk_no_drop->setChecked(
        settings->value("video/ffmpeg_no_drop", false).toBool());
    chk_no_drop->setToolTip(
        tr("Block file processing when the encoder is full so every processed "
           "frame is written. This can make processing slower than realtime."));
    ffmpegGrid->addWidget(chk_no_drop, 8, 0, 1, 2);
    
    ffmpegLayout->addLayout(ffmpegGrid);
    
    chk_mux_audio = new QCheckBox(tr("Copy audio from source video"), this);
    chk_mux_audio->setChecked(true);
    ffmpegLayout->addWidget(chk_mux_audio);
    
    mainLayout->addWidget(ffmpegGroup);

    chk_sync_fps = new QCheckBox(tr("Sync processing to video FPS"), this);
    chk_sync_fps->setChecked(
        settings->value("video/sync_processing_fps", true).toBool());
    chk_sync_fps->setToolTip(
        tr("Limit processing to the source video's reported frame rate."));
    mainLayout->addWidget(chk_sync_fps);
    
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
    connect(ffmpeg_codec, SIGNAL(currentIndexChanged(int)), this, SLOT(onCodecChanged(int)));
    connect(encoder_options, &QPushButton::clicked, this, [this]() {
        showEncoderOptions(this, ffmpeg_codec, ffmpeg_parameters);
    });
    
    onUseFFmpegChanged(Qt::Checked);
    onCodecChanged(ffmpeg_codec->currentIndex());
}

void CaptureVideo::onUseFFmpegChanged(int state) {
    bool useFFmpeg = (state == Qt::Checked);
    ffmpeg_codec->setEnabled(useFFmpeg);
    spin_crf->setEnabled(useFFmpeg);
    ffmpeg_preset->setEnabled(useFFmpeg);
    ffmpeg_tune->setEnabled(useFFmpeg);
    ffmpeg_parameters->setEnabled(useFFmpeg);
    encoder_details->setEnabled(useFFmpeg);
    chk_no_drop->setEnabled(useFFmpeg);
    updateEncoderDetails(ffmpeg_codec, encoder_details, encoder_options);
    chk_realtime->setEnabled(useFFmpeg);
    chk_mux_audio->setEnabled(useFFmpeg);
    video_type->setEnabled(!useFFmpeg);
}

void CaptureVideo::onCodecChanged(int index) {
    (void)index;
    updateEncoderDetails(ffmpeg_codec, encoder_details, encoder_options);
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
    FFmpegEncodeOptions options = selectedEncodeOptions(
        spin_crf, ffmpeg_preset, ffmpeg_tune, ffmpeg_codec,
        ffmpeg_parameters, chk_realtime);
    options.blockWhenFull = chk_no_drop->isChecked();
    bool muxAudio = chk_mux_audio->isChecked();
    settings->setValue("video/ffmpeg_codec_name", ffmpeg_codec->currentData());
    settings->setValue("video/ffmpeg_quality", options.quality);
    settings->setValue("video/ffmpeg_preset", ffmpeg_preset->currentText());
    settings->setValue("video/ffmpeg_tune", ffmpeg_tune->currentText());
    settings->setValue("video/ffmpeg_parameters", ffmpeg_parameters->text().trimmed());
    settings->setValue("video/ffmpeg_realtime", options.realtime);
    settings->setValue("video/ffmpeg_no_drop", options.blockWhenFull);
    settings->setValue("video/sync_processing_fps", chk_sync_fps->isChecked());
    
    if(win_parent->startVideo(edit_src->text(), edit_outdir->text(), 
                              chk_record->isChecked(), chk_png->isChecked(), num,
                              useFFmpeg, options, muxAudio,
                              chk_sync_fps->isChecked())) {
        hide();
    } else {
        QMessageBox::information(this, tr("Could not open file"), 
            tr("Could not open video file, an error has occurred"));
    }
}
