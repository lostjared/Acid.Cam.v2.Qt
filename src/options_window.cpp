#include "options_window.h"


OptionsWindow::OptionsWindow(QWidget *parent) : QDialog(parent) {
    setFixedSize(170, 225);
    createControls();
}

void OptionsWindow::createControls() {
    QLabel *label_x = new QLabel(tr("Thread Count: "), this);
    label_x->setGeometry(10, 10, 100, 25);
    op_thread = new QLineEdit("4", this);
    op_thread->setGeometry(110, 10, 50, 25);
    QLabel *label_y = new QLabel(tr("Difference: "), this);
    label_y->setGeometry(10, 40, 100, 25);
    op_intensity = new QLineEdit("55", this);
    op_intensity->setGeometry(110, 40, 50, 25);
    QLabel *label_z = new QLabel(tr("Max Frames: "), this);
    label_z->setGeometry(10, 70, 100, 25);
    op_max_frames = new QLineEdit("1500", this);
    op_max_frames->setGeometry(110,70, 50, 25);

    QLabel *label_q = new QLabel(tr("Delay: "), this);
    label_q->setGeometry(10, 100, 100, 25);
    
    fps_delay = new QLineEdit("60", this);
    fps_delay->setGeometry(110, 100, 50, 25);

    QLabel *_fwait = new QLabel(tr("Wait:"), this);
    _fwait->setGeometry(10, 130, 50, 25);
    fwait = new QLineEdit("5", this);
    fwait->setGeometry(110, 130, 50, 25);
    
    QLabel *_level = new QLabel(tr("Level:"), this);
    _level->setGeometry(10, 160, 50, 25);
    level = new QLineEdit("125", this);
    level->setGeometry(110, 160, 50, 25);
    
    op_setpref = new QPushButton(tr("Set"), this);
    op_setpref->setGeometry(10, 190, 100, 25);
    connect(op_setpref, SIGNAL(clicked()), this, SLOT(setValues()));
    
    
}

void OptionsWindow::setValues() {
    int thread_count = atoi(op_thread->text().toStdString().c_str());
    if(thread_count <= 0) {
        QMessageBox::information(this, tr("Error require valid thread count"),tr( "Requires Valid Thread Count..."));
        return;
    }
    int intensity = atoi(op_intensity->text().toStdString().c_str());
    if(intensity <= 0) {
        QMessageBox::information(this, tr("Error requires valid intensity"), tr("To set the value you must provide a valid intensity"));
        return;
    }
    int max_frames = atoi(op_max_frames->text().toStdString().c_str());
    playback->setPref(thread_count, intensity);
    if(max_frames < 300) {
        QMessageBox::information(this, tr("Error Requires Max"), tr("Required Max Frames must be greater than 300"));
        return;
    }
    
    int delay_value = atoi(fps_delay->text().toStdString().c_str());
    if(delay_value > 0)
        playback->setCustomCycleDelay(delay_value);
    
    ac::setMaxAllocated(max_frames);
    QString text;
    
    int level_ = atoi(level->text().toStdString().c_str());
    int wait_ = atoi(fwait->text().toStdString().c_str());
    
    if(level_ < 0 || wait_ < 0) {
        QMessageBox::information(this, tr("Error requires valid level/wait"), tr("Requires Level/Intensity greater than zero value"));
        return;
    }
    ac::setVariableWait(wait_);
    ac::setColorLevel(level_);
    QTextStream stream(&text);
    stream << tr("Thread Count set to: ") << thread_count << tr(" and Intensity set to: ") << intensity << tr("\nMaximum Stored Frames: ") << max_frames << tr("\n") << tr("Delay: ") << delay_value << "\n" << tr("Wait: ") << wait_ << "\n" << tr("Level: ") << level_ << "\n";
    QMessageBox::information(this, tr("Pref Value Set"), text);
}

void OptionsWindow::setPlayback(Playback *p) {
    playback = p;
}
