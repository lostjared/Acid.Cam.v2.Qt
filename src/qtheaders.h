/*
 * Acid Cam v2 - Qt/OpenCV Edition
 * written by Jared Bruni ( http://lostsidedead.com )
 * (C) 2017 GPL
*/

#ifndef _QT_HEADERS__
#define _QT_HEADERS__
#define ac_version "v1.17.3"
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
#include"ac.h"
#include"fractal.h"
#include<unordered_map>
#include<utility>
#include<vector>
#include<algorithm>

void init_plugins();
void draw_plugin(cv::Mat &frame, int filter);
extern std::unordered_map<std::string, std::pair<int, int>> filter_map;
#endif
