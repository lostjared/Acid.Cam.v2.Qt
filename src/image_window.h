#ifndef __IMAGE_WINDOW__H_
#define __IMAGE_WINDOW__H_

#include "qtheaders.h"
#include "playback_thread.h"

class ImageWindow : public QDialog {
Q_OBJECT
public:
    ImageWindow(QWidget *parent);
    void createControls();
    void setPlayback(Playback *play);
public slots:
    void image_AddFiles();
    void image_RmvFile();
    void image_SetFile();
    void image_RowChanged(int index);
    void image_SetCycle();
    void video_Set();
    void video_Clr();
private:
    QListWidget *image_files;
    QPushButton *add_files, *rmv_file, *set_file;
    QComboBox *image_cycle;
    QCheckBox *image_cycle_on;
    QLabel *image_pic;
    QLineEdit *image_frames;
    QPushButton *image_set_cycle;
    Playback *playback;
    QPushButton *btn_setvideo, *btn_clear;
    QLabel *lbl_video;
};

#endif

