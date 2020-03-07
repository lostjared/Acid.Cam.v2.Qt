
#include"color_range.h"

ColorRangeWindow::ColorRangeWindow(QWidget *parent) : QDialog(parent) {
    setFixedSize(280, 200);
    color1 = new QLabel("", this);
    color2 = new QLabel("", this);
    color1->setGeometry(25, 25, 100, 25);
    
    color2->setGeometry(135, 25, 100, 25);
    
    QString color_var = "#000000";
    
    color1->setStyleSheet("QLabel { background-color :" + color_var + " ; }");
    color2->setStyleSheet("QLabel { background-color :" + color_var + " ; }");
    
    setc1 = new QPushButton("Set", this);
    setc1->setGeometry(25, 75, 100, 25);
    
    setc2 = new QPushButton("Set", this);
    setc2->setGeometry(135, 75, 100, 25);
    
    connect(setc1, SIGNAL(clicked()), this, SLOT(selectColor1()));
    connect(setc2, SIGNAL(clicked()), this, SLOT(selectColor2()));
    
    range_set = new QPushButton("Enable", this);
    range_set->setGeometry(75, 125, 100, 25);
    connect(range_set, SIGNAL(clicked()), this, SLOT(setValues()));

}

void ColorRangeWindow::selectColor1() {
    QColorDialog *dialog = new QColorDialog(this);
    QColor color=  dialog->getColor();
    QVariant variant = color;
    QString color_var = variant.toString();
    //set_low_color = color;
    color1_value[0] = color.blue();
    color1_value[1] = color.green();
    color2_value[2] = color.blue();
    color1->setStyleSheet("QLabel { background-color :" + color_var + " ; }");
    color1->setText("");
}

void ColorRangeWindow::selectColor2() {
    QColorDialog *dialog = new QColorDialog(this);
    QColor color=  dialog->getColor();
    QVariant variant = color;
    QString color_var = variant.toString();
    //set_low_color = color;
    color2_value[0] = color.blue();
    color2_value[1] = color.green();
    color2_value[2] = color.blue();
    color2->setStyleSheet("QLabel { background-color :" + color_var + " ; }");
    color2->setText("");
}

void ColorRangeWindow::setValues() {
    ac::setColorRangeLowToHigh(color1_value, color2_value);
    ac::setColorRangeEnabled(true);
    QMessageBox::information(this, "Color Range Enabled...", "Enabled Color Range");
}
