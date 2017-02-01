#include "main_window.h"

std::unordered_map<std::string, std::pair<int, int>> filter_map;

const char *filter_names[] = { "AC Self AlphaBlend", "Reverse Self AlphaBlend",
    "Opposite Self AlphaBlend", "AC2 Distort", "Reverse Distort", "Opposite Distort",
    "Full Distort", "A New One", "AC NewOne", "AC Thought Filter", "Line Draw",
    "Gradient Square", "Color Wave", "Pixelated Gradient", "Combined Gradient",
    "Diagonal", "Average", "Average Divide", "Cos/Sin Multiply", "Modulus Multiply",
    "Positive/Negative", "z+1 Blend", "Diamond Pattern", "Pixelated Shift","Pixelated Mix",
    "Color Accumulate", "Color Accumulate #2", "Color Accumulate #3", "Angle",
    "Vertical Average", "Circular Blend", "Average Blend", "~Divide", "Mix", "Random Number",
    "Gradient Repeat", 0 };

void generate_map() {
    for(int i = 0; i < ac::draw_max; ++i )
        filter_map[ac::draw_strings[i]] = std::make_pair(0, i);
    
    int index = 0;
    while(filter_names[index] != 0) {
        std::string filter_n = "AF_";
        filter_n += filter_names[index];
        filter_map[filter_n] = std::make_pair(1, index);
        ++index;
    }
}

void custom_filter(cv::Mat &frame) {
    
}

AC_MainWindow::AC_MainWindow(QWidget *parent) : QMainWindow(parent) {
    generate_map();
    setGeometry(0, 0, 800, 600);
    setFixedSize(800, 600);
    setWindowTitle("Acid Cam v2 - Qt");
    createControls();
    createMenu();
    statusBar()->showMessage(tr("Acid Cam v2 Loaded - Use File Menu to Start"));
}

void AC_MainWindow::createControls() {
    
    filters = new QListWidget(this);
    filters->setGeometry(10, 10, 390, 200);
    filters->show();
    
    custom_filters = new QListWidget(this);
    custom_filters->setGeometry(400, 10, 390, 200);
    custom_filters->show();
    
    for(int i = 0; i < ac::draw_max-4; ++i) {
        filters->addItem(ac::draw_strings[i].c_str());
    }
    
    for(int i = 0; filter_names[i] != 0; ++i) {
        std::string filter_n = "AF_";
        filter_n += filter_names[i];
        filters->addItem(filter_n.c_str());
    }
    
    btn_add = new QPushButton("Add", this);
    btn_remove = new QPushButton("Remove", this);
    btn_moveup = new QPushButton("Move Up", this);
    btn_movedown = new QPushButton("Move Down", this);
    
    btn_add->setGeometry(10, 215, 100, 20);
    btn_remove->setGeometry(400, 215, 100, 20);
    btn_moveup->setGeometry(500, 215, 100, 20);
    btn_movedown->setGeometry(600, 215, 100, 20);
    
    connect(btn_add, SIGNAL(clicked()), this, SLOT(addClicked()));
    connect(btn_remove, SIGNAL(clicked()), this, SLOT(rmvClicked()));
    connect(btn_moveup, SIGNAL(clicked()), this, SLOT(upClicked()));
    connect(btn_movedown, SIGNAL(clicked()), this, SLOT(downClicked()));
    
    log_text = new QTextEdit(this);
    log_text->setGeometry(10, 250, 780,310);
    log_text->setReadOnly(true);
    
    QString text = "Acid Cam v";
    text += ac::version.c_str();
    text += " loaded.\n";
    log_text->setText(text);
    
    filters->setCurrentRow(0);
    
    chk_negate = new QCheckBox("Negate", this);
    chk_negate->setGeometry(120,215,100, 20);
    chk_negate->setCheckState(Qt::Unchecked);
    
    combo_rgb = new QComboBox(this);
    combo_rgb->setGeometry(200,215, 190, 25);
    combo_rgb->addItem("RGB");
    combo_rgb->addItem("BGR");
    combo_rgb->addItem("BRG");
    combo_rgb->addItem("GRB");
    
    setWindowIcon(QPixmap(":/images/icon.png"));
    
}

void AC_MainWindow::createMenu() {
    
    file_menu = menuBar()->addMenu(tr("&File"));
    controls_menu = menuBar()->addMenu(tr("&Controls"));
    help_menu = menuBar()->addMenu(tr("Help"));
    
    file_new_capture = new QAction(tr("Capture from Webcam"),this);
    file_new_capture->setShortcut(tr("Ctrl+N"));
    file_menu->addAction(file_new_capture);
    
    file_new_video = new QAction(tr("Capture from Video"), this);
    file_new_video->setShortcut(tr("Ctrl+V"));
    file_menu->addAction(file_new_video);
    
    file_exit = new QAction(tr("E&xit"), this);
    file_exit->setShortcut(tr("Ctrl+X"));
    file_menu->addAction(file_exit);
    
    connect(file_new_capture, SIGNAL(triggered()), this, SLOT(file_NewCamera()));
    connect(file_new_video, SIGNAL(triggered()), this, SLOT(file_NewVideo()));
    connect(file_exit, SIGNAL(triggered()), this, SLOT(file_Exit()));
    
    controls_stop = new QAction(tr("Sto&p"), this);
    controls_stop->setShortcut(tr("Ctrl+C"));
    controls_menu->addAction(controls_stop);
    
    controls_snapshot = new QAction(tr("&Snap"), this);
    controls_snapshot->setShortcut(tr("Ctrl+S"));
    controls_menu->addAction(controls_snapshot);
    
    controls_pause = new QAction(tr("&Pause"), this);
    controls_pause->setShortcut(tr("Ctrl+P"));
    controls_menu->addAction(controls_pause);
    
    controls_step = new QAction(tr("Step"), this);
    controls_step->setShortcut(tr("Controls+I"));
    controls_menu->addAction(controls_step);
    
    connect(controls_snapshot, SIGNAL(triggered()), this, SLOT(controls_Snap()));
    connect(controls_pause, SIGNAL(triggered()), this, SLOT(controls_Pause()));
    connect(controls_step, SIGNAL(triggered()), this, SLOT(controls_Step()));
    connect(controls_stop, SIGNAL(triggered()), this, SLOT(controls_Stop()));
    
    help_about = new QAction(tr("About"), this);
    help_about->setShortcut(tr("Ctrl+A"));
    help_menu->addAction(help_about);
    
    connect(help_about, SIGNAL(triggered()), this, SLOT(help_About()));
}

void AC_MainWindow::addClicked() {
    
    int row = filters->currentRow();
    if(row != -1) {
        custom_filters->addItem(ac::draw_strings[row].c_str());
    }
    
}

void AC_MainWindow::rmvClicked() {
    int item = custom_filters->currentRow();
    if(item != -1) {
        custom_filters->takeItem(item);
    }
}

void AC_MainWindow::upClicked() {
    int item = custom_filters->currentRow();
    if(item > 0) {
        QListWidgetItem *i = custom_filters->takeItem(item);
        custom_filters->insertItem(item-1, i->text());
        custom_filters->setCurrentRow(item-1);
    }
}

void AC_MainWindow::downClicked() {
    int item = custom_filters->currentRow();
    if(item >= 0 && item < custom_filters->count()-1) {
    	QListWidgetItem *i = custom_filters->takeItem(item);
    	custom_filters->insertItem(item+1, i->text());
    	custom_filters->setCurrentRow(item+1);
    }
  
}

void AC_MainWindow::Log(const QString &s) {
    QString text;
    text = log_text->toPlainText();
    text += "\n";
    text += s;
    log_text->setText(text);
    QTextCursor tmpCursor = log_text->textCursor();
    tmpCursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    log_text->setTextCursor(tmpCursor);
}

void AC_MainWindow::controls_Stop() {
    
}

void AC_MainWindow::file_Exit() {
    QApplication::exit(0);
}

void AC_MainWindow::file_NewVideo() {
    
}

void AC_MainWindow::file_NewCamera() {
    
}

void AC_MainWindow::controls_Snap() {
    
}

void AC_MainWindow::controls_Pause() {
    
}

void AC_MainWindow::controls_Step() {
    
}

void AC_MainWindow::help_About() {
    
}

