/*
 * Acid Cam v2 - Qt/OpenCV Edition
 * written by Jared Bruni ( http://lostsidedead.com )
 * (C) 2017 GPL
 */

#ifndef _QT_HEADERS__
#define _QT_HEADERS__
//#define DISABLE_JOYSTICK
#define ac_version "v1.66.0"
#include<QApplication>
#include<QMainWindow>
#include<QDialog>
#include<QListWidget>
#include<QPushButton>
#include<QTextEdit>
#include<QTextCursor>
#include<QComboBox>
#include<QCheckBox>
#include<QMenu>
#include<QMenuBar>
#include<QAction>
#include<QStatusBar>
#include<QMessageBox>
#include<QLabel>
#include<QTextStream>
#include<QLineEdit>
#include<QFileDialog>
#include<QTimer>
#include<QMutex>
#include<QThread>
#include<QImage>
#include<QPainter>
#include<QWaitCondition>
#include<QLibrary>
#include<QDir>
#include<QFile>
#include<QProgressBar>
#include<QRadioButton>
#include<QLineEdit>
#include<QColorDialog>
#include<QDesktopServices>
#include<QKeyEvent>
#include"ac.h"
#include"fractal.h"
#include<unordered_map>
#include<utility>
#include<vector>
#include<algorithm>
#include<fstream>

struct FilterValue {
    int index, filter, subfilter;
    FilterValue() : index(0), filter(0), subfilter(-1) {}
    FilterValue(int i, int f, int s) : index(i), filter(f), subfilter(s) {}
    FilterValue &operator=(const FilterValue &v) {
        index = v.index;
        filter = v.filter;
        subfilter = v.subfilter;
        return *this;
    }
};
void init_plugins();
void draw_plugin(cv::Mat &frame, int filter);
extern std::unordered_map<std::string, FilterValue> filter_map;

#endif
