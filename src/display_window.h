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
private:
    QLabel *img_label;
};

#endif
