#ifndef __MAIN_WINDOW_H__
#define __MAIN_WINDOW_H__

#include "qtheaders.h"

class AC_MainWindow : public QMainWindow {
    Q_OBJECT
public:
    AC_MainWindow(QWidget *parent = 0);
    
    QListWidget *filters, *custom_filters;
    QPushButton *btn_add, *btn_remove, *btn_moveup, *btn_movedown;
    QTextEdit *log_text;
    QCheckBox *chk_negate;
    QComboBox *combo_rgb;
    
public slots:
    void addClicked();
    void rmvClicked();
    void upClicked();
    void downClicked();
    
    void Log(const QString &s);
    
private:
    void createControls();
    void createMenu();
    
};

extern std::unordered_map<std::string, int> filter_map;
void generate_map();

#endif
