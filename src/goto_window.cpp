#include "goto_window.h"


GotoWindow::GotoWindow(QWidget *parent) : QDialog(parent) {
    createControls();
    setFixedSize(400, 70);
    setWindowTitle("Jump to Frame");
    setGeometry(300, 50, 400, 70);
}


void GotoWindow::setVideoCapture(cv::VideoCapture *cap) {
    capture_device = cap;
}
void GotoWindow::setDisplayWindow(DisplayWindow *win) {
    disp_window = win;
}

void GotoWindow::createControls() {
    goto_pos = new QSlider(Qt::Horizontal, this);
    goto_pos->setGeometry(10, 10, 380, 20);
    goto_pos->setMaximum(1);
    goto_pos->setMinimum(0);
    goto_pos->setTickInterval(0);
    
    QLabel *lbl_sec = new QLabel("Second: ", this);
    QLabel *lbl_frame = new QLabel("Frame: ", this);
    lbl_sec->setGeometry(10, 30, 50, 20);
    lbl_frame->setGeometry(180, 30, 50, 20);
    goto_sec = new QLineEdit("0", this);
    goto_sec->setGeometry(70,30,100, 20);
    goto_frame = new QLineEdit("0", this);
    goto_frame->setGeometry(230,30,100,20);
    goto_jump = new QPushButton("Go", this);
    goto_jump->setGeometry(340, 30, 45, 20);

    connect(goto_jump, SIGNAL(clicked()), this, SLOT(pressedGo()));
    connect(goto_pos, SIGNAL(valueChanged(int)), this, SLOT(slideChanged(int)));
    
}

void GotoWindow::setFrameIndex(const long &i) {
    index = i;
}

void GotoWindow::showImage() {
    
}

void GotoWindow::showWindow(int min, int max) {
    goto_pos->setMaximum(min);
    goto_pos->setMaximum(max);
    show();
}

void GotoWindow::pressedGo() {
    QString fpos = goto_frame->text();
    int f_pos = atoi(fpos.toStdString().c_str());
    if(f_pos != goto_pos->sliderPosition()) {
        goto_pos->setSliderPosition(f_pos);
    }
}

void GotoWindow::slideChanged(int pos) {
    QString text;
    QTextStream stream(&text);
    stream << static_cast<int>(pos/ac::fps);
    goto_sec->setText(text);
    text = "";
    stream << (pos);
    goto_frame->setText(text);
}
