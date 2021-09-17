#include"search_box.h"
#include"tokenize.h"

std::string lowerString(std::string text) {
    std::string new_text;
    for(unsigned long i = 0; i < text.length(); ++i)
        new_text += tolower(text[i]);
    return new_text;
}

SearchWindow::SearchWindow(QWidget *parent) : QDialog(parent) {
    draw_strings = ac::draw_strings;
    setFixedSize(640, 480);
    setWindowTitle(tr("Search Filters"));
    setWindowIcon(QPixmap(":/images/icon.png"));
    createControls();
}


bool SearchWindow::checkAdd(QString str) {
    const char *ex[] = { "Zoom", "AcidShuffleMedian", "MatrixColorBlur", "AlphaBlendArrayExpand", "Zoom", "ImageXorSmooth", "SketchFilter", "SlideSub", "Histogram", "Desktop","MultiVideo","Solo", "Bars", "BilateralFilter", "BilateralFilterFade", "BoxFilter", "CurrentDesktopRect", "HorizontalTrailsInter", "IntertwineAlpha", "IntertwineAlphaBlend", "IntertwineVideo640", "RandomAlphaBlendFilter", "RandomOrigFrame", "RectangleGlitch", "SquareSwap64x32", "VideoColorMap", 0};

    std::string val = str.toStdString();
    for(int i = 0; ex[i] != 0; ++i)
        if(val.find(ex[i]) != std::string::npos)
            return true;
    return false;
}

void SearchWindow::createControls() {
    search_list = new QListWidget(this);
    search_list->setGeometry(25, 25, 595, 400);
    search_list->show();
    search_text = new QLineEdit(this);
    search_text->setGeometry(25, 430, 290, 30);
    search_text->show();
    search = new QPushButton(this);
    search->setGeometry(325, 430, 100, 30);
    search->setText(tr("Search"));
    subf = new QPushButton(this);
    subf->setGeometry(490+25+10,430, 100, 30);
    subf->setText(tr("SubFilter"));
    add = new QPushButton(this);
    add->setText(tr("Add"));
    add->setGeometry((490+25+10)-100, 430, 100, 30);
    connect(search, SIGNAL(pressed()), this, SLOT(search_filter()));
    connect(add, SIGNAL(pressed()), this, SLOT(add_current()));
    connect(subf, SIGNAL(pressed()), this, SLOT(set_subf()));
}

void SearchWindow::search_filter() {
    while(search_list->count() > 0) {
        search_list->takeItem(0);
    }
    std::string search = lowerString(search_text->text().toStdString());
    std::vector<std::string> tokens;
    token::tokenize(search, std::string(" "), tokens);
    for(int i = 0; i < ac::draw_max-6; ++i) {
        std::string search_value = lowerString(draw_strings[i]);
        for(unsigned int q = 0; q < tokens.size(); ++q) {
            if(search_value.find(tokens[q]) != std::string::npos && checkAdd(draw_strings[i].c_str()) == false) {
                search_list->addItem(draw_strings[i].c_str());
                break;
            }
        }
    }
}

void SearchWindow::add_current() {
    int index = search_list->currentRow();
    if(index >= 0) {
        QListWidgetItem *in = search_list->item(index);
        custom_list->addItem(in->text());
        main_window->updateList();
        std::string text = in->text().toStdString();
        if(blend_set == false && text.find("Image") != std::string::npos)
            main_window->Log(tr("Set an Image to use this filter\n"));
        else if(text.find("SubFilter") != std::string::npos)
            main_window->Log(tr("Set a SubFilter to use this filter\n"));
        QString qtext;
        QTextStream stream(&qtext);
        stream << "Filter set to: " << in->text() << "\n";
        main_window->Log(qtext);
    }
}

void SearchWindow::setFiltersControl(QComboBox *filter_box, QListWidget *custombox) {
    filters = filter_box;
    custom_list = custombox;
}

void SearchWindow::set_subf() {
    int index = search_list->currentRow();
    if(index >= 0) {
        QListWidgetItem *in = search_list->item(index);
        main_window->setSubFilter(in->text());
        main_window->updateList();
    }
}
