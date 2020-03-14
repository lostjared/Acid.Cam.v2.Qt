#include"slitscan_win.h"

SlitScanWindow::SlitScanWindow(QWidget *parent) : QDialog(parent) {

    setFixedSize(400, 140);
    
    QLabel *label1 = new QLabel("Width: ", this);
    label1->setGeometry(5,5,50,25);
    s_width = new QLineEdit("640", this);
    s_width->setGeometry(60,5,100,25);
    
    QLabel *label2 = new QLabel("Height: ", this);
    label2->setGeometry(170,5,50,25);
    s_height = new QLineEdit("480", this);
    s_height->setGeometry(220,5,100,25);

    QLabel *label3 = new QLabel("Slit Height: ", this);
    label3->setGeometry(5,35,50,25);
    s_frames = new QLineEdit("480", this);
    s_frames->setGeometry(60,35,100,25);
    
    QLabel *label4 = new QLabel("Repeat: ", this);
    label4->setGeometry(170,35,50,25);
    s_repeat = new QLineEdit("1", this);
    s_repeat->setGeometry(220,35,100,25);

    QLabel *label5 = new QLabel("Delay: ", this);
    label5->setGeometry(5,65,50,25);
    s_delay = new QLineEdit("0", this);
    s_delay->setGeometry(60,65,100,25);
    
    QLabel *label6 = new QLabel("Wait: ", this);
    label6->setGeometry(170,65,50,25);
    s_wait = new QLineEdit("2", this);
    s_wait->setGeometry(220,65,100,25);

    s_set = new QPushButton("Set", this);
    s_set->setGeometry(5,90,100, 25);
    
    connect(s_set, SIGNAL(clicked()), this, SLOT(setValues()));
}

void SlitScanWindow::setValues() {
    
    int w = atoi(s_width->text().toStdString().c_str());
    int h = atoi(s_height->text().toStdString().c_str());
    int f = atoi(s_frames->text().toStdString().c_str());
    int r = atoi(s_repeat->text().toStdString().c_str());
    int d = atoi(s_delay->text().toStdString().c_str());
    int wa = atoi(s_wait->text().toStdString().c_str());
    
    if(w > 0 && h > 0 && f > 0 && r >= 0 && d >= 0 && wa >= 0) {
        ac::slitScanSet(f, w, h, r, d, wa);
        QMessageBox::information(this, "Values set", "Values set");
    
    } else {
        QMessageBox::information(this, "Requires at least greater than zero input", "Input invalid");
    }
    
    
    
    
}
