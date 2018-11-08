#ifndef __CHROMAKEY__H_
#define __CHROMAKEY__H_

#include "qtheaders.h"


class ChromaWindow : public QDialog {
    Q_OBJECT
public:
    ChromaWindow(QWidget *parent);
public slots:
    void openColorSelectRange();
    void openColorSelectTolerance();
private:
    void createControls();
    QRadioButton *button_select_range, *button_select_tolerance;
    QLineEdit *low_b, *low_g, *low_r, *high_b, *high_g, *high_r;
};


#endif

