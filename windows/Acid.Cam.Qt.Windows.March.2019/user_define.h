#ifndef __USER_DEFINE_H
#define __USER_DEFINE_H

#include "qtheaders.h"


class AC_MainWindow;

class DefineWindow : public QMainWindow {
Q_OBJECT
public:
    DefineWindow(QWidget *p);
    void createControls();
    AC_MainWindow *main_window;
public slots:
    void setFilterName();
    void clearFilterNames();
    void saveNames();
    void loadNames();
private:
    QComboBox *def_filters;
    QListWidget *def_list;
    QLineEdit *def_newname;
    QPushButton *def_set, *def_clear, *def_save, *def_load;
};


#endif
