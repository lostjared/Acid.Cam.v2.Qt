#ifndef __GOTO_WINDOW__H_
#define __GOTO_WINDOW__H_

#include "qtheaders.h"
#include "display_window.h"

class GotoWindow : public QDialog {
    Q_OBJECT
public:
    GotoWindow(QWidget *parent);
    void setDisplayWindow(DisplayWindow *win);
    void createControls();
    void setFrameIndex(const long &index);
    void ShowImage();
private:
    long index;
    DisplayWindow *disp_window;
};

#endif

