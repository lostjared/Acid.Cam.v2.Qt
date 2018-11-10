
#include "chroma_window.h"


ChromaWindow::ChromaWindow(QWidget *parent) : QDialog(parent) {
    setFixedSize(400, 240);
    setWindowTitle(tr("Chroma Key"));
    setWindowIcon(QPixmap(":/images/icon.png"));
    createControls();
}

void ChromaWindow::createControls() {
    button_select_range = new QRadioButton("Color Range", this);
    button_select_range->setGeometry(75, 25, 120, 20);
    connect(button_select_range, SIGNAL(clicked()), this, SLOT(openColorSelectRange()));
    button_select_tolerance = new QRadioButton("Select Color Tolerance", this);
    button_select_tolerance->setGeometry(75+120+10, 25, 150, 20);
    connect(button_select_tolerance, SIGNAL(clicked()), this, SLOT(openColorSelectTolerance()));
    low_b = new QLineEdit("0", this);
    low_g = new QLineEdit("0", this);
    low_r = new QLineEdit("0", this);
    high_b = new QLineEdit("0", this);
    high_g = new QLineEdit("0", this);
    high_r = new QLineEdit("0", this);
    low_b->setGeometry(95, 65, 50, 20);
    low_g->setGeometry(170, 65, 50, 20);
    low_r->setGeometry(245, 65, 50, 20);
    high_b->setGeometry(95, 90, 50, 20);
    high_g->setGeometry(170, 90, 50, 20);
    high_r->setGeometry(245, 90, 50, 20);
    string_low = new QLabel("<b>BGR Low:</b> ", this);
    string_high = new QLabel("<b>BGR High:</b> ", this);
    string_low->setGeometry(15, 65, 75, 20);
    string_high->setGeometry(15, 90, 75, 20);
    button_select_range->setChecked(true);
    color_keys = new QListWidget(this);
    color_keys->setGeometry(15, 130, 320-30, 100);
    color_add = new QPushButton("Add Key", this);
    color_add->setGeometry(320-10, 130, 75, 20);
    color_remove = new QPushButton("Remove", this);
    color_remove->setGeometry(320-10, 155, 75, 20);
}


void ChromaWindow::openColorSelectRange() {
// set to use range
    string_low->setText(tr("<b>BGR Low:</b> "));
    string_high->setText(tr("<b>BGR High:</b>"));
}

void ChromaWindow::openColorSelectTolerance() {
// set to use tolerance
    string_low->setText(tr("<b>Tolerance -</b>"));
    string_high->setText(tr("<b>Tolerance +</b>"));
}
