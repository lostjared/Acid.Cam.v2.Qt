#include"qtheaders.h"
#include "main_window.h"

bool blend_set = false;
cv::Mat blend_image;

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    AC_MainWindow window;
    window.show();
    return app.exec();
    
}
