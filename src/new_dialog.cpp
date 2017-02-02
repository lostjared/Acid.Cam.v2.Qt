#include "new_dialog.h"
#include "main_window.h"


CaptureCamera::CaptureCamera(QWidget *parent) : QDialog(parent) {
    setFixedSize(290, 120);
    setWindowTitle("Capture from Webcam");
    setWindowIcon(QPixmap(":/images/icon.png"));
    createControls();
}

void CaptureCamera::createControls() {
    QLabel *res = new QLabel("Resolution: ", this);
    res->setGeometry(10, 10, 75, 20);
    combo_res = new QComboBox(this);
    combo_res->setGeometry(85, 10, 200, 25);
    combo_res->addItem("640x480 (SD)");
    combo_res->addItem("1280x720 (HD)");
    combo_res->addItem("1920x1080 (Full HD)");
    QLabel *dev = new QLabel("Device: ", this);
    dev->setGeometry(10, 35, 50, 20);
    combo_device = new QComboBox(this);
    combo_device->setGeometry(85, 35, 200, 25);
    for(int i = 0; i < 10; ++i) {
        QString s;
        QTextStream stream(&s);
        stream << i;
        combo_device->addItem(*stream.string());
    }
   	btn_select = new QPushButton("Save Directory", this);
    btn_select->setGeometry(10, 65, 100, 20);
    output_dir = new QLineEdit("", this);
    output_dir->setGeometry(110, 65, 175, 20);
    output_dir->setReadOnly(true);
    chk_record = new QCheckBox("Record", this);
    chk_record->setGeometry(10, 95, 100, 20);
    btn_start = new QPushButton("Start", this);
    btn_start->setGeometry(185, 95, 100, 20);
    connect(btn_start, SIGNAL(clicked()), this, SLOT(btn_Start()));
    connect(btn_select, SIGNAL(clicked()), this, SLOT(btn_Select()));
}

void CaptureCamera::setParent(AC_MainWindow *p) {
    win_parent = p;
}

void CaptureCamera::btn_Select() {
 
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), "/home",QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    
    if(dir != "") {
        output_dir->setText(dir);
    }
}

void CaptureCamera::btn_Start() {
    if(output_dir->text().length() > 0) {
        if(win_parent->startCamera(combo_res->currentIndex(), combo_device->currentIndex(), output_dir->text(), chk_record->isChecked())) {
            hide();
            
        } else {
            QMessageBox::information(this, "Could not open Capture device", "Make sure you Webcam is pluged in. If you have more than one Webcam use the proper device index.");
        }
    }
}

CaptureVideo::CaptureVideo(QWidget *parent) : QDialog(parent) {
    setFixedSize(330, 100);
    setWindowTitle(("Capture from Video"));
    setWindowIcon(QPixmap(":/images/icon.png"));
    createControls();
}

void CaptureVideo::createControls() {
    btn_setedit = new QPushButton("Source File", this);
    btn_setedit->setGeometry(10, 10, 110, 20);
    edit_src = new QLineEdit(this);
    edit_src->setGeometry(120, 10, 200, 20);
    edit_src->setReadOnly(true);
    btn_setout = new QPushButton("Set Output", this);
    btn_setout->setGeometry(10, 30, 110, 20);
    edit_outdir = new QLineEdit(this);
    edit_outdir->setGeometry(120, 30, 200, 20);
    edit_outdir->setReadOnly(true);
    btn_start = new QPushButton("Start", this);
    btn_start->setGeometry(10, 60, 100, 20);
    chk_record = new QCheckBox("Record", this);
    chk_record->setGeometry(110, 60, 100, 20);
    
    connect(btn_setedit, SIGNAL(clicked()), this, SLOT(btn_SetSourceFile()));
    connect(btn_setout, SIGNAL(clicked()), this, SLOT(btn_SetOutputDir()));
    connect(btn_start, SIGNAL(clicked()), this, SLOT(btn_Start()));
}

void CaptureVideo::setParent(AC_MainWindow *p) {
    win_parent = p;
}

void CaptureVideo::btn_SetSourceFile() {
    
}

void CaptureVideo::btn_SetOutputDir() {
    
}

void CaptureVideo::btn_Start() {
    
}


