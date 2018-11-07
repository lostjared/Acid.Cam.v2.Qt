
#include "chroma_window.h"


ChromaWindow::ChromaWindow(QWidget *parent) : QDialog(parent) {
    setFixedSize(800, 600);
    setWindowTitle(tr("Chroma Key"));
    setWindowIcon(QPixmap(":/images/icon.png"));
    createControls();
    color_dialog = new QColorDialog(this);
}

void ChromaWindow::createControls() {
    button_select = new QPushButton("Select Color", this);
    connect(button_select, SIGNAL(clicked()), this, SLOT(openColorSelect()));
}


void ChromaWindow::openColorSelect() {
    
}
