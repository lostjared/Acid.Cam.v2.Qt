/*
 * Acid Cam v2 - Qt/OpenCV Edition
 * written by Jared Bruni ( http://lostsidedead.com )
 * (C) 2017 GPL
 */

// windows.h MUST be included before any C++ standard library or Qt headers
// to avoid the std::byte vs rpcndr.h byte ambiguity (C++17 + MinGW).
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

//#define LINUX_RELEASE

#include"qtheaders.h"
#include "main_window.h"
#ifdef LINUX_RELEASE
#include<unistd.h>
#endif

#ifdef _WIN32
// Lock DLL search order to: exe directory + System32 only.
// This prevents Windows from loading a wrong libstdc++-6.dll (or other
// MinGW runtime DLLs) from PATH entries like Git, MSYS2, Strawberry Perl, etc.
// Must be called before any DLLs are loaded (i.e. very first thing in main).
static void lockDllSearchOrder() {
    // Remove current-working-directory from DLL search (security + correctness)
    SetDllDirectoryW(L"");

    // Restrict DLL search to: application directory + system32
    // LOAD_LIBRARY_SEARCH_APPLICATION_DIR = 0x00000200
    // LOAD_LIBRARY_SEARCH_SYSTEM32        = 0x00000800
    typedef BOOL (WINAPI *SetDefaultDllDirectoriesFunc)(DWORD);
    HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
    if (kernel32) {
        auto pSetDefaultDllDirectories = (SetDefaultDllDirectoriesFunc)
            GetProcAddress(kernel32, "SetDefaultDllDirectories");
        if (pSetDefaultDllDirectories) {
            pSetDefaultDllDirectories(0x00000200 | 0x00000800);
        }
    }
}
#endif

int main(int argc, char **argv) {

#ifdef _WIN32
    lockDllSearchOrder();
#endif

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
