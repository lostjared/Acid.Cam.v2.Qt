/*
 * Acid Cam v2 - Qt/OpenCV Edition
 * written by Jared Bruni ( http://lostsidedead.com )
 * (C) 2017 GPL
 */


//#define LINUX_RELEASE

#include"qtheaders.h"
#include "main_window.h"
#ifdef LINUX_RELEASE
#include<unistd.h>
#endif

int main(int argc, char **argv) {
    
#ifdef LINUX_RELEASE
    if(chdir("/usr/share/acidcam") == 0) {
        std::cout << "Changed directory to: /usr/share/acidcam\n";
    }
#endif
    
    QApplication app(argc, argv);
    
    // Load and apply professional stylesheet
    QFile styleFile(":/stylesheet.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = QLatin1String(styleFile.readAll());
        app.setStyle("Fusion");
        app.setStyleSheet(style);
        styleFile.close();
    }
    
    AC_MainWindow window;
    window.show();
    return app.exec();
    
}
