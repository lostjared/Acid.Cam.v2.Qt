#ifndef __CHROMAKEY__H_
#define __CHROMAKEY__H_

#include "qtheaders.h"


class ChromaWindow : public QDialog {
    Q_OBJECT
public:
    ChromaWindow(QWidget *parent);
    bool checkInput(cv::Vec3b &low, cv::Vec3b &high);
    bool checkEdit(QLineEdit *edit);
public slots:
    void openColorSelectRange();
    void openColorSelectTolerance();
    void colorAdd();
    void colorRemove();
    void colorSet();
    void setColorLow();
    void setColorHigh();
private:
    void createControls();
    QRadioButton *button_select_range, *button_select_tolerance;
    QLineEdit *low_b, *low_g, *low_r, *high_b, *high_g, *high_r;
    QLabel *string_low, *string_high;
    QListWidget *color_keys;
    QPushButton *color_add, *color_remove, *color_okay;
    QLabel *lowColor, *highColor;
    std::vector<ac::Keys> colorkeys_vec;
    QPushButton *lowButton, *highButton;
    QColor set_low_color, set_high_color;
};


#endif

