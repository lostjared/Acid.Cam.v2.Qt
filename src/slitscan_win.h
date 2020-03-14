#ifndef __SLITSCAN_H__
#define __SLITSCAN_H__

#include"qtheaders.h"

class SlitScanWindow : public QDialog {
Q_OBJECT
    
public:
    SlitScanWindow(QWidget *parent = 0);
    
    QLineEdit *s_width, *s_height, *s_frames, *s_repeat, *s_delay, *s_wait;
    QPushButton *s_set;
    
public slots:
    void setValues();
    
    
};
#endif

