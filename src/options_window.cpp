#include "options_window.h"


OptionsWindow::OptionsWindow(QWidget *parent) : QDialog(parent) {
    setFixedSize(170, 150);
    createControls();
}

void OptionsWindow::createControls() {
    QLabel *label_x = new QLabel(tr("Thread Count: "), this);
    label_x->setGeometry(10, 10, 100, 25);
    op_thread = new QLineEdit("4", this);
    op_thread->setGeometry(110, 10, 50, 25);
    QLabel *label_y = new QLabel(tr("Intensity: "), this);
    label_y->setGeometry(10, 40, 100, 25);
    op_intensity = new QLineEdit("55", this);
    op_intensity->setGeometry(110, 40, 50, 25);
    QLabel *label_z = new QLabel(tr("Max Frames: "), this);
    label_z->setGeometry(10, 70, 100, 25);
    op_max_frames = new QLineEdit("1500", this);
    op_max_frames->setGeometry(110,70, 50, 25);
    op_setpref = new QPushButton(tr("Set"), this);
    op_setpref->setGeometry(10, 110, 100, 25);
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
    ac::setMaxAllocated(max_frames);
    QString text;
    QTextStream stream(&text);
    stream << tr("Thread Count set to: ") << thread_count << tr(" and Intensity set to: ") << intensity << tr("\nMaximum Stored Frames: ") << max_frames << tr("\n");
    QMessageBox::information(this, tr("Pref Value Set"), text);
}

void OptionsWindow::setPlayback(Playback *p) {
    playback = p;
}
