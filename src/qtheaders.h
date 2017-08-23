/*
 * Acid Cam v2 - Qt/OpenCV Edition
 * written by Jared Bruni ( http://lostsidedead.com )
 * (C) 2017 GPL
*/

#ifndef _QT_HEADERS__
#define _QT_HEADERS__

#define ac_version "v0.8"

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
#include"ac.h"
#include"fractal.h"
#include<unordered_map>
#include<utility>

void init_plugins();
void draw_plugin(cv::Mat &frame, int filter);

#endif
