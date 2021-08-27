
#ifndef __SEARCHBOX__H__
#define __SEARCHBOX__H__

#include "qtheaders.h"
#include "main_window.h"

class SearchWindow : public QDialog {
    Q_OBJECT
public:
    AC_MainWindow *main_window;
    SearchWindow(QWidget *parent = 0);
    void createControls();
    void setFiltersControl(QComboBox *filter_box, QListWidget *custom);
public slots:
    void search_filter();
    void add_current();
    void set_subf();
private:
    QListWidget *search_list,*custom_list;
    QLineEdit *search_text;
    QPushButton *search, *add, *subf;
    QComboBox *filters;
    
    bool checkAdd(QString str);
};

#endif

