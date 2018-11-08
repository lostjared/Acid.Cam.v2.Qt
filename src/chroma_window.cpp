
#include "chroma_window.h"


ChromaWindow::ChromaWindow(QWidget *parent) : QDialog(parent) {
    setFixedSize(800, 600);
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
    // create 6 text input boxes.
}


void ChromaWindow::openColorSelectRange() {
// set to use range
}

void ChromaWindow::openColorSelectTolerance() {
// set to use tolerance
}
