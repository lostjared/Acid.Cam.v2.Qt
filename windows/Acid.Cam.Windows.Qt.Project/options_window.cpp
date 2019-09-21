#include "options_window.h"


OptionsWindow::OptionsWindow(QWidget *parent) : QDialog(parent) {
    setFixedSize(170, 175);
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
    op_setpref = new QPushButton(tr("Set"), this);
    op_setpref->setGeometry(10, 135, 100, 25);
    connect(op_setpref, SIGNAL(clicked()), this, SLOT(setValues()));
}

void OptionsWindow::setValues() {
    
    int thread_count = atoi(op_thread->text().toStdString().c_str());
    if(thread_count <= 0) {
        QMessageBox::information(this, "Error require valid thread count", "Requires Valid Thread Count...");
        return;
    }
    int intensity = atoi(op_intensity->text().toStdString().c_str());
    if(intensity <= 0) {
        QMessageBox::information(this, "Error requires valid intensity", "To set the value you must provide a valid intensity");
        return;
    }
    playback->setPref(thread_count, intensity);
    QString text;
    QTextStream stream(&text);
    stream << "Thread Count set to: " << thread_count << " and Intensity set to: " << intensity;
    QMessageBox::information(this, "Pref Value Set", text);
}

void OptionsWindow::setPlayback(Playback *p) {
    playback = p;
}
