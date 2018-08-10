
#ifndef __SEARCHBOX__H__
#define __SEARCHBOX__H__

#include "qtheaders.h"

class SearchWindow : public QDialog {
    Q_OBJECT
public:
    SearchWindow(QWidget *parent = 0);
    void createControls();
    
private:
    QListWidget *search_list;
    QLineEdit *search_text;
    QPushButton *search;
};



#endif

