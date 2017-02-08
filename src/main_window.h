#ifndef __MAIN_WINDOW_H__
#define __MAIN_WINDOW_H__

#include "qtheaders.h"
#include "new_dialog.h"

class Playback : public QThread {
Q_OBJECT
private:
    bool stop;
    QMutex mutex;
    QWaitCondition condition;
    cv::Mat frame;
    int frame_rate;
    bool recording;
    cv::VideoCapture capture;
    cv::VideoWriter  writer;
    cv::Mat rgb_frame;
    QImage img;
    std::vector<std::pair<int, int>> current;
    bool isPaused, isStep;
public:
    Playback(QObject *parent = 0);
    ~Playback();
    void Play();
    void Stop();
    void setVideo(cv::VideoCapture cap, cv::VideoWriter writer, bool record);
    bool isStopped() const;
    void run();
    void msleep(int ms);
    void setVector(std::vector<std::pair<int, int>> s);
    void setOptions(bool n, int c);
    void setImage(const cv::Mat &image);
    void setStep();
signals:
    void procImage(const QImage &image);
    void procCameraFrame(void *frame);
    
};

class DisplayWindow : public QDialog {
    Q_OBJECT
public:
    DisplayWindow(QWidget *parent = 0);
    void createControls();
    void displayImage(const QImage &img);
    void paintEvent(QPaintEvent *paint);
private:
    QLabel *img_label;
};

class AC_MainWindow : public QMainWindow {
    Q_OBJECT
public:
    AC_MainWindow(QWidget *parent = 0);
    ~AC_MainWindow();
    void Log(const QString &s);
    bool startCamera(int res, int dev, const QString &outdir, bool record);
    bool startVideo(const QString &filename, const QString &outdir, bool record);
    QListWidget *filters, *custom_filters;
    QPushButton *btn_add, *btn_remove, *btn_moveup, *btn_movedown;
    QTextEdit *log_text;
    QCheckBox *chk_negate;
    QComboBox *combo_rgb;
    QMenu *file_menu, *controls_menu, *help_menu;
    QAction *file_exit, *file_new_capture, *file_new_video;
    QAction *controls_snapshot, *controls_pause, *controls_step, *controls_stop, *controls_setimage;
    QAction *help_about;
public slots:
    void addClicked();
    void rmvClicked();
    void upClicked();
    void downClicked();
    void file_Exit();
    void file_NewVideo();
    void file_NewCamera();
    void controls_Stop();
    void controls_Snap();
    void controls_Pause();
    void controls_Step();
    void controls_SetImage();
    void help_About();
    void timer_Camera();
    void timer_Video();
    void updateFrame(QImage img);
    void CameraFrame(void *frame);
    void chk_Clicked();
    void cb_SetIndex(int index);
private:
    void createControls();
    void createMenu();
    DisplayWindow *disp;
    CaptureCamera *cap_camera;
    CaptureVideo *cap_video;
    cv::VideoCapture capture_camera, capture_video;
    cv::VideoWriter writer;
    unsigned long video_frames;
    double video_fps;
    QTimer *timer_video, *timer_camera;
    bool paused, recording, step_frame;
    QString video_file_name;
    QString output_directory;
    bool take_snapshot;
    unsigned long file_pos, frame_index;
    Playback *playback;
    
    void buildVector(std::vector<std::pair<int,int>> &v);
};

extern const char *filer_names[];
extern std::unordered_map<std::string, std::pair<int, int>> filter_map;
void generate_map();

#endif
