#include "new_dialog.h"

CaptureCamera::CaptureCamera(QWidget *parent) : QDialog(parent) {
    setFixedSize(640, 480);
    setWindowTitle("Capture from Webcam");
    setWindowIcon(QPixmap(":/images/icon.png"));
    createControls();
}

void CaptureCamera::createControls() {
    
}

CaptureVideo::CaptureVideo(QWidget *parent) : QDialog(parent) {
    setFixedSize(640, 480);
    setWindowTitle(("Capture from Video"));
    setWindowIcon(QPixmap(":/images/icon.png"));
    createControls();
}

void CaptureVideo::createControls() {
    
}
