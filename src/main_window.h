
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
#include "search_box.h"
#include "goto_window.h"
#include "chroma_window.h"

class SearchWindow;

class AC_MainWindow : public QMainWindow {
    Q_OBJECT
public:
    AC_MainWindow(QWidget *parent = 0);
    ~AC_MainWindow();
    void Log(const QString &s);
    bool startCamera(int res, int dev, const QString &outdir, bool record, int type);
    bool startVideo(const QString &filename, const QString &outdir, bool record, int type);
    QListWidget /**filters,*/ *custom_filters;
    QPushButton *btn_add, *btn_remove, *btn_moveup, *btn_movedown, *btn_sub, *btn_clr;
    QTextEdit *log_text;
    QCheckBox *chk_negate;
    QComboBox *combo_rgb;
    QSlider *slide_r, *slide_g, *slide_b, *slide_bright, *slide_gamma, *slide_saturation;
    QProgressBar *progress_bar;
    QComboBox *color_maps, *filters;
    QMenu *file_menu, *controls_menu, *help_menu, *options, *movement, *speed_menu;
    QAction *file_exit, *file_new_capture, *file_new_video;
    QAction *controls_snapshot, *controls_pause, *controls_step, *controls_stop, *controls_setimage,*controls_setkey,*controls_showvideo, *clear_images, *reset_filters;
    QAction *help_about;
    QAction *open_search;
    QAction *in_out_increase;
    QAction *in_out;
    QAction *out_reset;
    QAction *speed_action_items[7];
    QMenu *image_menu;
    QAction *flip1, *flip2, *flip3, *noflip;
    QAction *clear_sub;
    QAction *clear_image;
    QAction *repeat_v;
    QAction *fade_on;
    QAction *select_key;
    double speed_actions[7];
    QRadioButton *filter_single, *filter_custom;
    void updateList();
    void setSubFilter(const QString &num);
    void setFrameIndex(int i);
    public slots:
    void addClicked();
    void rmvClicked();
    void upClicked();
    void setSub();
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
    void controls_SetKey();
    void controls_Reset();
    void help_About();
    void updateFrame(QImage img);
    void stopRecording();
    void resetIndex();
    void chk_Clicked();
    void cb_SetIndex(int index);
    void frameInc();
    void slideChanged(int pos);
    void colorChanged(int pos);
    void colorMapChanged(int pos);
    void comboFilterChanged(int pos);
    void setFilterSingle();
    void setFilterCustom();
    void openSearch();
    void movementOption1();
    void movementOption2();
    void movementOption3();
    void speed1();
    void speed2();
    void speed3();
    void speed4();
    void speed5();
    void speed6();
    void speed7();
    void flip1_action();
    void flip2_action();
    void flip3_action();
    void noflip_action();
    void clear_subfilter();
    void clear_img();
    void repeat_vid();
    void setFade();
    void openColorWindow();
private:
    void createControls();
    void createMenu();
    DisplayWindow *disp;
    CaptureCamera *cap_camera;
    CaptureVideo *cap_video;
    SearchWindow *search_box;
    ChromaWindow *chroma_window;
    GotoWindow *goto_window;
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
    void buildVector(std::vector<FilterValue> &v);
};

extern const char *filer_names[];
extern std::unordered_map<std::string, FilterValue> filter_map;
void generate_map();

#endif
