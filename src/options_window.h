#ifndef __OPTIONS_WINDOW_H__
#define __OPTIONS_WINDOW_H__

#include "qtheaders.h"
#include "playback_thread.h"

class OptionsWindow : public QDialog {
Q_OBJECT

public:
    OptionsWindow(QWidget *parent);
    void createControls();
    void setPlayback(Playback *p);
public slots:
    void setValues();
private:
    QLineEdit *op_thread;
    QLineEdit *op_intensity;
    QPushButton *op_setpref;
    Playback *playback;
    QLineEdit *op_max_frames;
    QLineEdit *fps_delay;
};

#endif
