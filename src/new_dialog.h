#ifndef _NEW_DIALOG_H_
#define _NEW_DIALOG_H_

#include "qtheaders.h"


class CaptureCamera : public QDialog {
	Q_OBJECT
public:
    CaptureCamera(QWidget *parent = 0);
    void createControls();
    
    QComboBox *combo_res, *combo_device;
    QLineEdit *output_dir;
    QCheckBox *chk_record;
    QPushButton *btn_start, *btn_select;
    
public slots:
    void btn_Select();
    void btn_Start();
};

class CaptureVideo : public QDialog {
    Q_OBJECT
public:
    CaptureVideo(QWidget *parent = 0);
    void createControls();
    
};

#endif
