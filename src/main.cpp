#include"qtheaders.h"
#include "main_window.h"
#include<unistd.h>

bool blend_set = false;
cv::Mat blend_image;

int main(int argc, char **argv) {
    

    /*
    if(chdir("/usr/share/acidcam") == 0) {
        std::cout << "Changed directory to: /usr/share/acidcam\n";
    }
    */
    
    QApplication app(argc, argv);
    AC_MainWindow window;
    window.show();
    return app.exec();
    
}
