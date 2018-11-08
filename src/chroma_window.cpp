
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
    QLabel *string_low, *string_high;
    string_low = new QLabel("<b>Low BGR:</b> ", this);
    string_high = new QLabel("<b>High BGR:</b> ", this);
    string_low->setGeometry(15, 65, 75, 20);
    string_high->setGeometry(15, 90, 75, 20);
    
    button_select_range->setChecked(true);
}


void ChromaWindow::openColorSelectRange() {
// set to use range
}

void ChromaWindow::openColorSelectTolerance() {
// set to use tolerance
}
