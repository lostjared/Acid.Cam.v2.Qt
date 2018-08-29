#ifndef __GOTO_WINDOW__H_
#define __GOTO_WINDOW__H_


#include "qtheaders.h"

class GotoWindow : public QDialog {
    Q_OBJECT
public:
    GotoWindow(QWidget *parent);
    void setVideo(cv::VideoCapture *cap);    
private:
    cv::VideoCapture *capture;
};

#endif

