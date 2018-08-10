#include "search_box.h"

SearchWindow::SearchWindow(QWidget *parent) : QDialog(parent) {
    setFixedSize(640, 480);
    setWindowTitle("Search Filters");
    setWindowIcon(QPixmap(":/images/icon.png"));
    createControls();
}


void SearchWindow::createControls() {
    
}
