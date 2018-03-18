
/*
 * Acid Cam v2 - Qt/OpenCV Edition
 * written by Jared Bruni ( http://lostsidedead.com )
 * (C) 2017 GPL
*/

#ifndef __MAIN_WINDOW_H__
#define __MAIN_WINDOW_H__

#include "qtheaders.h"
#include "new_dialog.h"
#include "display_window.h"
#include "playback_thread.h"

class AC_MainWindow : public QMainWindow {
    Q_OBJECT
public:
    AC_MainWindow(QWidget *parent = 0);
    ~AC_MainWindow();
    void Log(const QString &s);
    bool startCamera(int res, int dev, const QString &outdir, bool record, int type);
    bool startVideo(const QString &filename, const QString &outdir, bool record, int type);
    QListWidget /**filters,*/ *custom_filters;
    QPushButton *btn_add, *btn_remove, *btn_moveup, *btn_movedown;
    QTextEdit *log_text;
    QCheckBox *chk_negate;
    QComboBox *combo_rgb;
    QSlider *slide_r, *slide_g, *slide_b, *slide_bright, *slide_gamma, *slide_saturation;
    QProgressBar *progress_bar;
    QComboBox *color_maps, *filters;
    QMenu *file_menu, *controls_menu, *help_menu;
    QAction *file_exit, *file_new_capture, *file_new_video;
    QAction *controls_snapshot, *controls_pause, *controls_step, *controls_stop, *controls_setimage, *controls_showvideo;
    QAction *help_about;
    QRadioButton *filter_single, *filter_custom;
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
    void controls_ShowVideo();
    void help_About();
    void updateFrame(QImage img);
    void stopRecording();
    void chk_Clicked();
    void cb_SetIndex(int index);
    void frameInc();
    void slideChanged(int pos);
    void colorChanged(int pos);
    void colorMapChanged(int pos);
    void comboFilterChanged(int pos);
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
    bool paused, recording, step_frame;
    QString video_file_name;
    QString output_directory;
    bool take_snapshot;
    unsigned long file_pos, frame_index;
    Playback *playback;
    VideoMode programMode;
    void buildVector(std::vector<std::pair<int,int>> &v);
};

extern const char *filer_names[];
extern std::unordered_map<std::string, std::pair<int, int>> filter_map;
void generate_map();

#endif
