#ifndef __USER_DEFINE_H
#define __USER_DEFINE_H

#include "qtheaders.h"

class DefineWindow : public QMainWindow {
Q_OBJECT
public:
    DefineWindow(QWidget *p);
    void createControls();
private:
    QComboBox *def_filters;
    QListWidget *def_list;
    QLineEdit *def_newname;
    QPushButton *def_set, *def_clear;
};


#endif
