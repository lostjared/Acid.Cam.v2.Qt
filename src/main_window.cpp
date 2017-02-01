#include "main_window.h"

void custom_filter(cv::Mat &frame) {
    
}

AC_MainWindow::AC_MainWindow(QWidget *parent) : QMainWindow(parent) {
    
    setGeometry(0, 0, 800, 600);
    setWindowTitle("Acid Cam v2 - Qt");
    
    createControls();
    createMenu();
}

void AC_MainWindow::createControls() {
    
    filters = new QListWidget(this);
    filters->setGeometry(10, 10, 390, 200);
    filters->show();
    
    custom_filters = new QListWidget(this);
    custom_filters->setGeometry(400, 10, 390, 200);
    custom_filters->show();
    
    for(int i = 0; i < ac::draw_max-4; ++i) {
        filters->addItem(ac::draw_strings[i].c_str());
    }
    
    btn_add = new QPushButton("Add", this);
    btn_remove = new QPushButton("Remove", this);
    btn_moveup = new QPushButton("Move Up", this);
    btn_movedown = new QPushButton("Move Down", this);
    
    btn_add->setGeometry(10, 215, 100, 20);
    btn_remove->setGeometry(400, 215, 100, 20);
    btn_moveup->setGeometry(500, 215, 100, 20);
    btn_movedown->setGeometry(600, 215, 100, 20);
    
    connect(btn_add, SIGNAL(clicked()), this, SLOT(addClicked()));
    connect(btn_remove, SIGNAL(clicked()), this, SLOT(rmvClicked()));
    connect(btn_moveup, SIGNAL(clicked()), this, SLOT(upClicked()));
    connect(btn_movedown, SIGNAL(clicked()), this, SLOT(downClicked()));
    
    log_text = new QTextEdit(this);
    log_text->setGeometry(10, 250, 780,310);
    log_text->setReadOnly(true);
    
    QString text = "Acid Cam v";
    text += ac::version.c_str();
    text += " loaded.\n";
    log_text->setText(text);
    
    filters->setCurrentRow(0);
    
}

void AC_MainWindow::createMenu() {
    
}

void AC_MainWindow::addClicked() {
    
    
    int row = filters->currentRow();
    if(row != -1) {
        custom_filters->addItem(ac::draw_strings[row].c_str());
    }
    
}

void AC_MainWindow::rmvClicked() {
    int item = custom_filters->currentRow();
    if(item != -1) {
        custom_filters->takeItem(item);
    }
}

void AC_MainWindow::upClicked() {
    
}

void AC_MainWindow::downClicked() {
    
}

void AC_MainWindow::Log(const QString &s) {
    QString text;
    text = log_text->toPlainText();
    text += "\n";
    text += s;
    log_text->setText(text);
    QTextCursor tmpCursor = log_text->textCursor();
    tmpCursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    log_text->setTextCursor(tmpCursor);
}



