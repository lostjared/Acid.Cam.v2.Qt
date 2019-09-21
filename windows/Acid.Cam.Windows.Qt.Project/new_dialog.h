
/*
 * Acid Cam v2 - Qt/OpenCV Edition
 * written by Jared Bruni ( http://lostsidedead.com )
 * (C) 2017 GPL
 */

#ifndef _NEW_DIALOG_H_
#define _NEW_DIALOG_H_

#include "qtheaders.h"

class AC_MainWindow;

class CaptureCamera : public QDialog {
    Q_OBJECT
public:
    CaptureCamera(QWidget *parent = 0);
    void createControls();
    void setParent(AC_MainWindow *p);
    
    QComboBox *combo_res, *combo_device;
    QLineEdit *output_dir;
    QCheckBox *chk_record;
    QPushButton *btn_start, *btn_select;
    QComboBox *video_type;
public slots:
    void btn_Select();
    void btn_Start();
    
private:
    AC_MainWindow *win_parent;
};

class CaptureVideo : public QDialog {
    Q_OBJECT
public:
    CaptureVideo(QWidget *parent = 0);
    void createControls();
    void setParent(AC_MainWindow *p);
    
    QLineEdit *edit_src, *edit_outdir;
    QPushButton *btn_setedit, *btn_setout, *btn_start;
    QCheckBox *chk_record;
    QComboBox *video_type;
    
public slots:
    void btn_SetSourceFile();
    void btn_SetOutputDir();
    void btn_Start();
private:
    AC_MainWindow *win_parent;
    
};

#endif
