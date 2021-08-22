/*
 * Acid Cam v2 - Qt/OpenCV Edition
 * written by Jared Bruni ( http://lostsidedead.com )
 * (C) 2017 GPL
 */


#ifndef __DISPLAY_WINDOW_H__
#define __DISPLAY_WINDOW_H__

#include"qtheaders.h"

class DisplayWindow : public QDialog {
    Q_OBJECT
public:
    DisplayWindow(QWidget *parent = 0);
    void createControls();
    void displayImage(const QImage &img);
    void paintEvent(QPaintEvent *paint);
    void keyPressEvent(QKeyEvent *ke);
    void keyReleaseEvent(QKeyEvent *ke);
    void showMax();
    void showGL();
private:
    QLabel *img_label;
    
};

#endif
