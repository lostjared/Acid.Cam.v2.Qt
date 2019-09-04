#include "image_window.h"

ImageWindow::ImageWindow(QWidget *parent) : QDialog(parent) {
    setFixedSize(800, 360);
    setWindowTitle(tr("Acid Cam v2 - Image Manager"));
    createControls();
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
    image_cycle_on = new QCheckBox("Image Cycle", this);
    image_cycle_on->setGeometry(220, 325, 100, 25);
    image_cycle = new QComboBox(this);
    image_cycle->setGeometry(320, 325, 100, 25);
    image_cycle->addItem(tr("Random"));
    image_cycle->addItem(tr("In Order"));
    image_cycle->addItem(tr("Shuffle"));
    image_pic = new QLabel("", this);
    image_pic->setGeometry((770/2)+10, 20, 770/2,300);
    image_pic->setStyleSheet("QLabel{background: black; color: #000000;}");
    // connect
    connect(add_files, SIGNAL(clicked()), this, SLOT(image_AddFiles()));
    connect(rmv_file, SIGNAL(clicked()), this, SLOT(image_RmvFile()));
    connect(set_file, SIGNAL(clicked()), this, SLOT(image_SetFile()));
    //image_files->addItem("TEST!!");
}

void ImageWindow::image_AddFiles() {
    
}

void ImageWindow::image_RmvFile() {
    
}

void ImageWindow::image_SetFile() {
    
}
