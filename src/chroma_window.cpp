
#include "chroma_window.h"
#include<cmath>
#include<cstdlib>

ChromaWindow::ChromaWindow(QWidget *parent) : QDialog(parent) {
    setFixedSize(400, 240);
    setWindowTitle(tr("Chroma Key"));
    setWindowIcon(QPixmap(":/images/icon.png"));
    createControls();
}

bool ChromaWindow::checkEdit(QLineEdit *edit) {
    QString text = edit->text();
    std::string chk_value;
    chk_value = text.toStdString();
    for(unsigned int i = 0; i < chk_value.length(); ++i) {
        if(!(chk_value[i] >= '0' && chk_value[i] <= '9'))
            return false;
    }
    return true;
}

bool ChromaWindow::checkInput(cv::Vec3b &low, cv::Vec3b &high) {
    double lo_b, lo_g, lo_r;
    double hi_b, hi_g, hi_r;
    lo_b = atof(low_b->text().toStdString().c_str());
    lo_g = atof(low_g->text().toStdString().c_str());
    lo_r = atof(low_r->text().toStdString().c_str());
    hi_b = atof(high_b->text().toStdString().c_str());
    hi_g = atof(high_g->text().toStdString().c_str());
    hi_r = atof(high_r->text().toStdString().c_str());
    low[0] = lo_b;
    low[1] = lo_g;
    low[2] = lo_r;
    high[0] = hi_b;
    high[1] = hi_g;
    high[2] = hi_r;
    
    if(button_select_range->isChecked() && (lo_b > hi_b || lo_g > hi_g || lo_r > hi_r))
            return false;
    
    if(lo_b >= 0 && lo_b <= 255 && lo_g >= 0 && lo_g <= 255 && lo_r >= 0 && lo_r <= 255 && hi_b >= 0 && hi_b <= 255 && hi_g >= 0 && hi_g <= 255 && hi_b >= 0 && hi_b <= 255)
    return true;
    return false;
}

void ChromaWindow::createControls() {
    button_select_range = new QRadioButton(tr("Color Range"), this);
    button_select_range->setGeometry(75, 25, 120, 20);
    connect(button_select_range, SIGNAL(clicked()), this, SLOT(openColorSelectRange()));
    button_select_tolerance = new QRadioButton(tr("Select Color Tolerance"), this);
    button_select_tolerance->setGeometry(75+120+10, 25, 150, 20);
    connect(button_select_tolerance, SIGNAL(clicked()), this, SLOT(openColorSelectTolerance()));
    low_b = new QLineEdit("0", this);
    low_g = new QLineEdit("0", this);
    low_r = new QLineEdit("0", this);
    high_b = new QLineEdit("0", this);
    high_g = new QLineEdit("0", this);
    high_r = new QLineEdit("0", this);
    low_b->setGeometry(95, 65, 50, 20);
    low_g->setGeometry(170, 65, 50, 20);
    low_r->setGeometry(245, 65, 50, 20);
    high_b->setGeometry(95, 90, 50, 20);
    high_g->setGeometry(170, 90, 50, 20);
    high_r->setGeometry(245, 90, 50, 20);
    string_low = new QLabel("<b>BGR Low:</b> ", this);
    string_high = new QLabel("<b>BGR High:</b> ", this);
    string_low->setGeometry(15, 65, 75, 20);
    string_high->setGeometry(15, 90, 75, 20);
    button_select_range->setChecked(true);
    color_keys = new QListWidget(this);
    color_keys->setGeometry(15, 130, 320-30, 100);
    color_add = new QPushButton(tr("Add Key"), this);
    color_add->setGeometry(320-10, 130, 75, 20);
    color_remove = new QPushButton(tr("Remove"), this);
    color_remove->setGeometry(320-10, 155, 75, 20);
    color_okay = new QPushButton(tr("Set Keys"), this);
    color_okay->setGeometry(320-10,210, 75, 20);
    connect(color_add, SIGNAL(clicked()), this, SLOT(colorAdd()));
    connect(color_remove, SIGNAL(clicked()), this, SLOT(colorRemove()));
    connect(color_okay, SIGNAL(clicked()), this, SLOT(colorSet()));
}


void ChromaWindow::openColorSelectRange() {
// set to use range
    string_low->setText(tr("<b>BGR Low:</b> "));
    string_high->setText(tr("<b>BGR High:</b>"));
}

void ChromaWindow::openColorSelectTolerance() {
// set to use tolerance
    string_low->setText(tr("<b>Tolerance -</b>"));
    string_high->setText(tr("<b>Tolerance +</b>"));
}

void ChromaWindow::colorAdd() {
    cv::Vec3b low, high;
    QLineEdit *array[] = { low_r, low_g, low_b, high_r, high_g, high_b, 0 };
    for(int i = 0; array[i] != 0; ++i) {
        if(checkEdit(array[i])==false) {
            QMessageBox::information(this, "Invalid Value", "Values must be between 0-255 no characters");
            return;
        }
    }
    if(checkInput(low, high)==false) {
        QMessageBox::information(this,"Error ","Error Color Values must be between 0-255 and a valid range");
        return;
    }
    ac::Keys key_id;
    key_id.low = low;
    key_id.high = high;
    if(button_select_range->isChecked())
        key_id.key_type = ac::KeyValueType::KEY_RANGE;
     else
        key_id.key_type = ac::KeyValueType::KEY_TOLERANCE;
    colorkeys_vec.push_back(key_id);
    QString text;
    QTextStream stream(&text);
    QString type_key = ((key_id.key_type == ac::KeyValueType::KEY_RANGE) ? "Range" : "Tolerance");
    
    stream << "Added Chroma Key " << type_key << "\n" << colorkeys_vec.size() << " Keys set.\n";
    QMessageBox::information(this, "Key Added", text);
    text = "";
    stream << type_key << " BGR(" << low[0] << "," << low[1] << "," << low[2] << ") - BGR(" << high[0] << "," << high[1] << "," << high[2] << ")\n";
    color_keys->addItem(text);
}
void ChromaWindow::colorRemove() {
    
}
void ChromaWindow::colorSet() {
    QString text;
    QTextStream stream(&text);
    stream << "Set " << colorkeys_vec.size() << " Chroma Keys.\n";
    QMessageBox::information(this, "Set Key Values", text);
    ac::setBlockedColorKeys(colorkeys_vec);
}
