#ifndef __COLOR_RANGE__H_
#define __COLOR_RANGE__H_
#include"qtheaders.h"


class ColorRangeWindow : public QDialog {
    Q_OBJECT
private:
    QLabel *color1, *color2;
    QPushButton *range_set, *setc1, *setc2;
    cv::Vec3b color1_value, color2_value;
public:
    ColorRangeWindow(QWidget *parent = 0);
    
public slots:
    void selectColor1();
    void selectColor2();
    void setValues();
    
};



#endif

