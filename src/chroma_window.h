#ifndef __CHROMAKEY__H_
#define __CHROMAKEY__H_

#include "qtheaders.h"
#include "main_window.h"

class ChromaWindow : public QDialog {
    Q_OBJECT
public:
    ChromaWindow(QWidget *parent);
    bool checkInput(cv::Vec3b &low, cv::Vec3b &high);
    bool checkEdit(QLineEdit *edit);
    void setEditFromColor(int val, QColor color);
    void enableKey(bool op);
    void showGL();
    AC_MainWindow *main_window;
  
public slots:
    void openColorSelectRange();
    void openColorSelectTolerance();
    void colorAdd();
    void colorRemove();
    void colorSet();
    void setColorLow();
    void setColorHigh();
    void setImage();
    void toggleKey();
    void editSetLowB(const QString &text);
    void editSetLowG(const QString &text);
    void editSetLowR(const QString &text);
    void editSetHighB(const QString &text);
    void editSetHighG(const QString &text);
    void editSetHighR(const QString &text);
private:
    void createControls();
    QRadioButton *button_select_range, *button_select_tolerance;
    QLineEdit *low_b, *low_g, *low_r, *high_b, *high_g, *high_r;
    QLabel *string_low, *string_high;
    QListWidget *color_keys;
    QPushButton *color_add, *color_remove, *color_okay;
    QLabel *lowColor, *highColor;
    std::vector<ac::Keys> colorkeys_vec;
    QPushButton *lowButton, *highButton;
    QColor set_low_color, set_high_color;
    QComboBox *select_mode;
    QPushButton *select_setimage;
    QLabel *select_image_path;
    QCheckBox *keys_enabled;
    bool color1_set, color2_set;
};


#endif

