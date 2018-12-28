
#include "user_define.h"
#include "main_window.h"

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
    QString filter_name = def_filters->currentText();
    QString filter_text = def_newname->text();
    if(filter_text.length() > 0) {
        std::string real_final_name = "User_";
        real_final_name += filter_text.toStdString();
        std::string sval = filter_name.toStdString();
        if(sval.find("Image") != std::string::npos)
            real_final_name += "_Image";
        if(sval.find("SubFilter") != std::string::npos)
            real_final_name += "_SubFilter";
        std::vector<std::string> *v = ac::filter_menu_map["User"].menu_list;
        v->push_back(real_final_name);
        std::string ft = filter_name.toStdString();
        std::string fn = real_final_name;
        filter_map[fn].index = 0;
        filter_map[fn].filter = filter_map[ft].filter;
        filter_map[fn].subfilter = -1;
        main_window->resetMenu();
        def_list->addItem(real_final_name.c_str());
        def_newname->setText("");
    }
}
void DefineWindow::clearFilterNames() {
    std::vector<std::string> *v = ac::filter_menu_map["User"].menu_list;
    if(!v->empty()) {
    	v->erase(v->begin(), v->end());
    	v->push_back("No Filter");
        while(def_list->count() > 0) {
        	def_list->takeItem(0);
        }
        main_window->resetMenu();
    }
}
