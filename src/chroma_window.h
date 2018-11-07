#ifndef __CHROMAKEY__H_
#define __CHROMAKEY__H_

#include "qtheaders.h"


class ChromaWindow : public QDialog {
    Q_OBJECT
public:
    ChromaWindow(QWidget *parent);
    
public slots:
    void openColorSelect();
    
private:
    void createControls();
    QPushButton *button_select;
};









#endif

