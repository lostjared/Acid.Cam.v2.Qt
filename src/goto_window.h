#ifndef __GOTO_WINDOW__H_
#define __GOTO_WINDOW__H_

#include "qtheaders.h"
#include "display_window.h"

class GotoWindow : public QDialog {
    Q_OBJECT
public:
    GotoWindow(QWidget *parent);
    void setVideoCapture(cv::VideoCapture *cap);
    void setDisplayWindow(DisplayWindow *win);
    void createControls();
    void setFrameIndex(const long &index);
    void showImage();
    void showWindow(int min, int max);
private:
    long index;
    DisplayWindow *disp_window;
    cv::VideoCapture *capture_device;
    QSlider *goto_pos;
    QLineEdit *goto_sec, *goto_frame;
    QPushButton *goto_jump;
public slots:
    void pressedGo();
    void slideChanged(int pos);
    
};

#endif

