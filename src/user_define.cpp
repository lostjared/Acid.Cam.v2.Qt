
#include "user_define.h"
#include "main_window.h"
#include<fstream>
#include<cstdlib>
#include<cstdio>

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
    def_save = new QPushButton("Save", this);
    def_save->setGeometry(230, 290, 100, 20);
    def_load = new QPushButton("Load", this);
    def_load->setGeometry(340, 290, 100, 20);
    
    
    for(auto &i : ac::svAllSorted) {
        static int q = 0;
        if((++q)%2 == 0 && main_window->checkAdd(i.c_str()) == false && i.find("Intertwine") == std::string::npos && i.find("InOrder") == std::string::npos)
            def_filters->addItem(i.c_str());
    }
    connect(def_set, SIGNAL(clicked()), this, SLOT(setFilterName()));
    connect(def_clear, SIGNAL(clicked()), this, SLOT(clearFilterNames()));
    connect(def_save, SIGNAL(clicked()), this, SLOT(saveNames()));
    connect(def_load, SIGNAL(clicked()), this, SLOT(loadNames()));
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
        main_window->playback->setFilterMapEx(filter_map);
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

void DefineWindow::saveNames() {
     QString fileName = QFileDialog::getSaveFileName(this,tr("Save List"), "", tr("Acid Cam List (*.acl)"));
    if(fileName.length() <= 0)
        return;
    std::fstream file;
    file.open(fileName.toStdString(), std::ios::out);
    if(!file.is_open()) {
        QMessageBox::information(this, tr("Could not save file"), tr("Could not save file"));
        return;
    }
    for(int index = 0; index < def_list->count(); ++index) {
        QListWidgetItem *m = def_list->item(index);
        std::string filter_name = m->text().toStdString();
        FilterValue &v = filter_map[filter_name];
        file << filter_name << ":" << v.filter << "\n";
    }
    file.close();
}

void DefineWindow::loadNames() {
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open List"), "", tr("Acid Cam List (*.acl)"));
    if(fileName.length() <= 0)
        return;
    std::fstream file(fileName.toStdString(), std::ios::in);
    if(!file.is_open()) {
        QMessageBox::information(this, tr("Could not open file"), tr("File could not be opened"));
        return;
    }
    clearFilterNames();
    while(!file.eof()) {
        std::string line;
        std::getline(file, line);
        if(file) {
            std::string left, right;
            auto pos = line.find(":");
            if(pos == std::string::npos) {
                QMessageBox::information(this, tr("Invalid File Format"), tr("Invalid"));
                return;
            }
            left = line.substr(0, pos);
            right = line.substr(pos+1, line.length()-pos);
            std::cout << "Left: " << left << "\n";
            std::cout << "Right: " << right << "\n";
            filter_map[left].index = 0;
            filter_map[left].subfilter = -1;
            filter_map[left].filter = atoi(right.c_str());
            def_list->addItem(left.c_str());
            std::vector<std::string> *v = ac::filter_menu_map["User"].menu_list;
            v->push_back(left);
        }
    }
}

