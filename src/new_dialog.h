#ifndef _NEW_DIALOG_H_
#define _NEW_DIALOG_H_

#include "qtheaders.h"


class CaptureCamera : public QDialog {
	Q_OBJECT
public:
    CaptureCamera(QWidget *parent = 0);
};

class CaptureVideo : public QDialog {
    Q_OBJECT
public:
    CaptureVideo(QWidget *parent = 0);
    
};

#endif
