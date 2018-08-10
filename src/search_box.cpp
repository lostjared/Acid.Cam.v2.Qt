#include "search_box.h"

SearchWindow::SearchWindow(QWidget *parent) : QDialog(parent) {
    setFixedSize(640, 480);
    setWindowTitle(tr("Search Filters"));
    setWindowIcon(QPixmap(":/images/icon.png"));
    createControls();
}


void SearchWindow::createControls() {
    search_list = new QListWidget(this);
    search_list->setGeometry(25, 25, 595, 400);
    search_list->show();
    search_text = new QLineEdit(this);
    search_text->setGeometry(25, 430, 390, 25);
    search_text->show();
    search = new QPushButton(this);
    search->setGeometry(490+25+10,430, 100, 35);
    search->setText(tr("Search"));
    add = new QPushButton(this);
    add->setText(tr("Add"));
    add->setGeometry((490+25+10)-100, 430, 100, 35);
    
    connect(search, SIGNAL(pressed()), this, SLOT(search_filter()));
}

void SearchWindow::search_filter() {
   
    while(search_list->count() > 0) {
        search_list->takeItem(0);
    }
}
