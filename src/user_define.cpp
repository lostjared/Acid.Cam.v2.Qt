
#include "user_define.h"

DefineWindow::DefineWindow(QWidget *p) : QMainWindow(p) {
    setFixedSize(640, 320);
    setWindowTitle("Filter Define");
    createControls();
}

void DefineWindow::createControls() {
    def_filters = new QComboBox(this);
    def_filters->setGeometry(10, 10, 620, 30);
    def_list = new QListWidget(this);
    def_list->setGeometry(10, 50, 620, 200);
    def_newname = new QLineEdit("", this);
    def_newname->setGeometry(10, 260, 620, 25);
    def_set = new QPushButton("Set", this);
    def_set->setGeometry(10, 290, 100, 20);
    def_clear = new QPushButton("Clear", this);
    def_clear->setGeometry(120, 290, 100, 20);
    for(auto &i : ac::svAllSorted) {
        def_filters->addItem(i.c_str());
    }
    connect(def_set, SIGNAL(clicked()), this, SLOT(setFilterName()));
    connect(def_clear, SIGNAL(clicked()), this, SLOT(clearFilterNames()));
}

void DefineWindow::setFilterName() {
    
}
void DefineWindow::clearFilterNames() {
    
}
