#ifndef __GOTO_WINDOW__H_
#define __GOTO_WINDOW__H_

#include "qtheaders.h"
#include "display_window.h"
#include "playback_thread.h"

class AC_MainWindow;

class GotoWindow : public QDialog {
    Q_OBJECT
public:
    GotoWindow(QWidget *parent);
    void setVideoCapture(cv::VideoCapture *cap);
    void setDisplayWindow(DisplayWindow *win);
    void setMainWindow(AC_MainWindow *window);
    void setPlayback(Playback *playb);
    void createControls();
    void setFrameIndex(int index);
    void showImage();
    void showWindow(int frame_index, int min, int max);
private:
    int frame_index;
    DisplayWindow *disp_window;
    AC_MainWindow *main_window;
    cv::VideoCapture *capture_device;
    QSlider *goto_pos;
    QLineEdit *goto_sec, *goto_frame;
    QPushButton *goto_jump;
    Playback *playback_thread;
public slots:
    void pressedGo();
    void slideChanged(int pos);
    
};

#endif

