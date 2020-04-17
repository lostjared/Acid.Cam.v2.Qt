######################################################################
# Automatically generated by qmake (2.01a) Wed Feb 1 02:31:01 2017
######################################################################

TEMPLATE = app
TARGET = Acid.Cam.v2.Qt
RC_FILE = win-icon.rc
QT += core gui widgets network
DEPENDPATH += .
#put path to OpenCV Include here
INCLUDEPATH += . C:\OpenCV_4\opencv\build\include
#INCLUDEPATH += . C:\OpenCV\opencv\build\include /usr/include/ /usr/local/include
#put path to OpenCV Librarys here
LIBS += C:\OpenCV_4\opencv\build\x64\vc15\lib/opencv_world420.lib -lopengl32
#LIBS += C:/OpenCV/opencv/build/x64/vc15/lib/opencv_world341.lib #-luser32 -lgdi32
RESOURCES += qresource.qrc
# Input
HEADERS += main_window.h new_dialog.h plugin.h qtheaders.h select_image.h ac.h fractal.h display_window.h playback_thread.h ac.h search_box.h goto_window.h chroma_window.h user_define.h ac-filtercat.h dl-man.h image_window.h options_window.h controller.h gl_display.h color_range.h slitscan_win.h
SOURCES += main.cpp main_window.cpp new_dialog.cpp plugin.cpp select_image.cpp fractal.cpp display_window.cpp playback_thread.cpp search_box.cpp user_define.cpp ac-alpha.cpp ac-obj.cpp ac-util.cpp ac-square.cpp ac-particle.cpp ac-grid.cpp ac-basic.cpp ac-filter1.cpp  ac-filter2.cpp ac-filter3.cpp ac-filter4.cpp ac-filter5.cpp ac-filter6.cpp ac-filter7.cpp ac-filter8.cpp ac-filter9.cpp ac-filter10.cpp ac-filter11.cpp ac-filter12.cpp ac-filter13.cpp ac-filter14.cpp ac-filter15.cpp ac-filter16.cpp ac-filter17.cpp ac-filter18.cpp ac-filter19.cpp ac-filter20.cpp ac-filter21.cpp ac-filter22.cpp ac-filter23.cpp ac-filter24.cpp ac-filter25.cpp ac-filter26.cpp ac-filter27.cpp ac-filter28.cpp ac-filter29.cpp ac-filter30.cpp ac-filter31.cpp ac-filter32.cpp ac-filter33.cpp ac-filter34.cpp ac-filter35.cpp ac-filter36.cpp ac-filter37.cpp ac-filter38.cpp ac-filter39.cpp ac-filter40.cpp ac-filter41.cpp ac-filter42.cpp ac-filter43.cpp ac-filter44.cpp ac-filter45.cpp ac-filter46.cpp ac-image.cpp ac-box.cpp chroma_window.cpp goto_window.cpp ac-filtercat.cpp dl-man.cpp image_window.cpp options_window.cpp controller.cpp gl_display.cpp color_range.cpp slitscan_win.cpp
