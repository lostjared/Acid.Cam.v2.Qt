#ifndef __IMAGE_WINDOW__H_
#define __IMAGE_WINDOW__H_

#include "qtheaders.h"

class ImageWindow : public QDialog {
Q_OBJECT
public:
    ImageWindow(QWidget *parent);
    void createControls();
    
public slots:
    void image_AddFiles();
    void image_RmvFile();
    void image_SetFile();
    void image_RowChanged(int index);
    
private:
    QListWidget *image_files;
    QPushButton *add_files, *rmv_file, *set_file;
    QComboBox *image_cycle;
    QCheckBox *image_cycle_on;
    QLabel *image_pic;
};

#endif

