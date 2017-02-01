#include"qtheaders.h"
#include "main_window.h"


int main(int argc, char **argv) {
    QApplication app(argc, argv);
    AC_MainWindow window;
    window.show();
    return app.exec();
    
}
