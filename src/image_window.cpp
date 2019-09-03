#include "image_window.h"

ImageWindow::ImageWindow(QWidget *parent) : QDialog(parent) {
    setFixedSize(800, 360);
    setWindowTitle(tr("Acid Cam v2 - Image Manager"));
    createControls();
}

void ImageWindow::createControls() {
    image_files = new QListWidget(this);
    image_files->setGeometry(20,20,770,300);
    add_files = new QPushButton(tr("Add"), this);
    add_files->setGeometry(15,325,100,25);
    rmv_file = new QPushButton(tr("Remove"), this);
    rmv_file->setGeometry(115,325,100,25);
    set_file = new QPushButton(tr("Set Image"), this);
    set_file->setGeometry(690,325,100,25);
    image_cycle_on = new QCheckBox("Image Cycle", this);
    image_cycle_on->setGeometry(220, 325, 100, 25);
    image_cycle = new QComboBox(this);
    image_cycle->setGeometry(320, 325, 100, 25);
    image_cycle->addItem("Random");
    image_cycle->addItem("In Order");
    image_cycle->addItem("Shuffle");
}
