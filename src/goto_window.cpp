#include "goto_window.h"


GotoWindow::GotoWindow(QWidget *parent) : QDialog(parent) {
    
}

void GotoWindow::setVideo(cv::VideoCapture *cap) {
    capture = cap;
}
