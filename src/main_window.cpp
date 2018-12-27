/*
 * Acid Cam v2 - Qt/OpenCV Edition
 * written by Jared Bruni ( http://lostsidedead.com )
 * (C) 2017 GPL
 */


#include "main_window.h"
#include<mutex>
#include"plugin.h"
#include<sys/stat.h>

std::unordered_map<std::string, FilterValue> filter_map;
void custom_filter(cv::Mat &);

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
        filter_map[ac::draw_strings[i]] = FilterValue(0, i, -1);
    
    int index = 0;
    while(filter_names[index] != 0) {
        std::string filter_n = "AF_";
        filter_n += filter_names[index];
        filter_map[filter_n] = FilterValue(1, index, -1);
        ++index;
    }
    for(unsigned int j = 0; j < plugins.plugin_list.size(); ++j) {
        std::string name = "plugin " + plugins.plugin_list[j]->name();
        filter_map[name] = FilterValue(2, j, -1);
    }
}

void custom_filter(cv::Mat &) {
    
}


AC_MainWindow::~AC_MainWindow() {
    controls_Stop();
    delete playback;
}

AC_MainWindow::AC_MainWindow(QWidget *parent) : QMainWindow(parent) {
    programMode = MODE_CAMERA;
    init_plugins();
    generate_map();
    setGeometry(100, 100, 800, 700);
    setFixedSize(800, 700);
    setWindowTitle(tr("Acid Cam v2 - Qt"));
    createControls();
    createMenu();
    
    cap_camera = new CaptureCamera(this);
    cap_camera->setParent(this);
    
    cap_video = new CaptureVideo(this);
    cap_video->setParent(this);
    
    search_box = new SearchWindow(this);
    search_box->setParent(this);
    search_box->setFiltersControl(filters, custom_filters);
    search_box->main_window = this;
    statusBar()->showMessage(tr("Acid Cam v2 Loaded - Use File Menu to Start"));
    take_snapshot = false;
    goto_window = new GotoWindow(this);
    disp = new DisplayWindow(this);
    playback = new Playback();
    
    goto_window->setParent(this);
    goto_window->setDisplayWindow(disp);
    goto_window->setPlayback(playback);
    goto_window->setMainWindow(this);
    
    QObject::connect(playback, SIGNAL(procImage(QImage)), this, SLOT(updateFrame(QImage)));
    QObject::connect(playback, SIGNAL(stopRecording()), this, SLOT(stopRecording()));
    QObject::connect(playback, SIGNAL(frameIncrement()), this, SLOT(frameInc()));
    QObject::connect(playback, SIGNAL(resetIndex()), this, SLOT(resetIndex()));
    
    for(unsigned int i = 0; i < plugins.plugin_list.size(); ++i) {
        QString text;
        QTextStream stream(&text);
        stream << "Loaded Plugin: " << plugins.plugin_list[i]->name().c_str() << "\n";
        Log(text);
    }
    
    int cindex = filters->findText("Self AlphaBlend");
    filters->setCurrentIndex(cindex);
    chroma_window = new ChromaWindow(this);
    chroma_window->hide();
}

void AC_MainWindow::createControls() {
    custom_filters = new QListWidget(this);
    custom_filters->setGeometry(400, 30, 390, 180);
    custom_filters->show();
   
    menu_cat = new QComboBox(this);
    menu_cat->setGeometry(10, 90, 380, 30);
    
    menu_cat->addItem("All Filters");
    menu_cat->addItem("All Filters Sorted");
    menu_cat->addItem("Blend");
    menu_cat->addItem("Distort");
    menu_cat->addItem("Pattern");
    menu_cat->addItem("Gradient");
    menu_cat->addItem("Mirror");
    menu_cat->addItem("Strobe");
    menu_cat->addItem("Blur");
    menu_cat->addItem("Image");
    menu_cat->addItem("Square");
    menu_cat->addItem("Other");
    menu_cat->addItem("SubFilter");
    menu_cat->addItem("Special");
    menu_cat->addItem("User");
    
    menu_cat->setCurrentIndex(0);
    
    filters = new QComboBox(this);
    filters->setGeometry(10, 120, 380, 30);
    
    std::vector<std::string> fnames;
    for(int i = 0; i < ac::draw_max-5; ++i) {
        fnames.push_back(ac::draw_strings[i].c_str());
    }
    
    std::sort(fnames.begin(), fnames.end());
    
    for(unsigned long i = 0; i < fnames.size(); ++i) {
        filters->addItem(fnames[i].c_str());
    }
    
    for(int i = 0; filter_names[i] != 0; ++i) {
        std::string filter_n = "AF_";
        filter_n += filter_names[i];
        filters->addItem(filter_n.c_str());
    }
    
    for(unsigned int i = 0; i < plugins.plugin_list.size(); ++i) {
        std::string name = "plugin " + plugins.plugin_list[i]->name();
        filters->addItem(name.c_str());
    }
    
    connect(filters, SIGNAL(currentIndexChanged(int)), this, SLOT(comboFilterChanged(int)));
    connect(menu_cat, SIGNAL(currentIndexChanged(int)), this, SLOT(menuFilterChanged(int)));
    
    
    filter_single = new QRadioButton(tr("Single Filter"), this);
    filter_single->setGeometry(30, 40, 100, 15);
    filter_custom = new QRadioButton(tr("Custom Filter"), this);
    filter_custom->setGeometry(30, 65, 100, 15);
    
    filter_single->setChecked(true);
    
    connect(filter_single, SIGNAL(pressed()), this, SLOT(setFilterSingle()));
    connect(filter_custom, SIGNAL(pressed()), this, SLOT(setFilterCustom()));
    
    btn_add = new QPushButton(tr("Add"), this);
    btn_remove = new QPushButton(tr("Remove"), this);
    btn_moveup = new QPushButton(tr("Move Up"), this);
    btn_movedown = new QPushButton(tr("Move Down"), this);
    btn_sub = new QPushButton(tr("Subfilter"), this);
    btn_clr = new QPushButton(tr("Clear Sub"), this);
    btn_add->setGeometry(10, 215, 100, 20);
    btn_remove->setGeometry(400, 215, 100, 20);
    btn_moveup->setGeometry(500, 215, 100, 20);
    btn_movedown->setGeometry(600, 215, 100, 20);
    btn_sub->setGeometry(10, 165, 100, 20);
    btn_clr->setGeometry(115, 165, 100, 20);
    
    connect(btn_add, SIGNAL(clicked()), this, SLOT(addClicked()));
    connect(btn_remove, SIGNAL(clicked()), this, SLOT(rmvClicked()));
    connect(btn_moveup, SIGNAL(clicked()), this, SLOT(upClicked()));
    connect(btn_movedown, SIGNAL(clicked()), this, SLOT(downClicked()));
    connect(btn_sub, SIGNAL(clicked()), this, SLOT(setSub()));
    connect(btn_clr, SIGNAL(clicked()), this, SLOT(clear_subfilter()));
    
    QLabel *r_label = new QLabel(tr("Red: "), this);
    r_label->setGeometry(10, 255, 50, 20);
    slide_r = new QSlider(Qt::Horizontal,this);
    slide_r->setGeometry(40, 250, 100, 30);
    slide_r->setMaximum(255);
    slide_r->setMinimum(0);
    slide_r->setTickInterval(0);
    
    QLabel *g_label = new QLabel(tr("Green: "), this);
    g_label->setGeometry(150, 255, 50, 20);
    slide_g = new QSlider(Qt::Horizontal, this);
    slide_g->setGeometry(190, 250, 100, 30);
    slide_g->setMaximum(255);
    slide_g->setMinimum(0);
    slide_g->setTickInterval(0);
    
    QLabel *b_label = new QLabel(tr("Blue: "), this);
    b_label->setGeometry(300, 255, 50, 20);
    slide_b = new QSlider(Qt::Horizontal, this);
    slide_b->setGeometry(330, 250, 100, 30);
    slide_b->setMaximum(255);
    slide_b->setMinimum(0);
    slide_b->setTickInterval(0);
    
    connect(slide_r, SIGNAL(valueChanged(int)), this, SLOT(slideChanged(int)));
    connect(slide_g, SIGNAL(valueChanged(int)), this, SLOT(slideChanged(int)));
    connect(slide_b, SIGNAL(valueChanged(int)), this, SLOT(slideChanged(int)));
    
    QLabel *label_slide_bright = new QLabel(tr("Brightness: "), this);
    label_slide_bright->setGeometry(10, 280, 75, 20);
    slide_bright = new QSlider(Qt::Horizontal, this);
    slide_bright->setGeometry(80, 275, 100, 30);
    slide_bright->setMaximum(255);
    slide_bright->setMinimum(0);
    slide_bright->setTickInterval(0);
    
    
    QLabel *label_slide_gamma = new QLabel(tr("Gamma: "), this);
    label_slide_gamma->setGeometry(190, 280, 65, 20);
    slide_gamma = new QSlider(Qt::Horizontal, this);
    slide_gamma->setGeometry(245, 275, 100, 30);
    slide_gamma->setMaximum(255);
    slide_gamma->setMinimum(0);
    slide_gamma->setTickInterval(0);
    
    QLabel *label_sat = new QLabel(tr("Saturation: "), this);
    label_sat->setGeometry(350, 280, 100, 20);
    slide_saturation = new QSlider(Qt::Horizontal, this);
    slide_saturation->setGeometry(420, 275, 100, 30);
    slide_saturation->setMaximum(255);
    slide_saturation->setMinimum(0);
    slide_saturation->setTickInterval(0);
    
    connect(slide_bright, SIGNAL(valueChanged(int)), this, SLOT(colorChanged(int)));
    connect(slide_gamma, SIGNAL(valueChanged(int)), this, SLOT(colorChanged(int)));
    connect(slide_saturation, SIGNAL(valueChanged(int)), this, SLOT(colorChanged(int)));
    
    QLabel *color_maps_label = new QLabel("<b>Color Maps</b>", this);
    color_maps_label->setGeometry(545, 260, 75, 20);
    
    color_maps = new QComboBox(this);
    color_maps->setGeometry(540, 275, 250, 30);
    color_maps->addItem(tr("None"));
    color_maps->addItem(tr("Autum"));
    color_maps->addItem(tr("Bone"));
    color_maps->addItem(tr("Jet"));
    color_maps->addItem(tr("Winter"));
    color_maps->addItem(tr("Rainbow"));
    color_maps->addItem(tr("Ocean"));
    color_maps->addItem(tr("Summer"));
    color_maps->addItem(tr("Cool"));
    color_maps->addItem(tr("HSV"));
    color_maps->addItem(tr("Pink"));
    color_maps->addItem(tr("Hot"));
    color_maps->addItem(tr("Parula"));
    
    connect(color_maps, SIGNAL(currentIndexChanged(int)), this, SLOT(colorMapChanged(int)));
    
    log_text = new QTextEdit(this);
    log_text->setGeometry(10, 325, 780,310);
    log_text->setReadOnly(true);
    
    QString text;
    QTextStream stream(&text);
    stream << tr("Acid Cam Filters v");
    stream << ac::version.c_str();
    stream << " loaded " << filters->count() << " filters.\n";
    log_text->setText(text);
    chk_negate = new QCheckBox(tr("Negate"), this);
    chk_negate->setGeometry(120,215,100, 20);
    chk_negate->setCheckState(Qt::Unchecked);
    
    connect(chk_negate, SIGNAL(clicked()), this, SLOT(chk_Clicked()));
    
    combo_rgb = new QComboBox(this);
    combo_rgb->setGeometry(200,215, 190, 25);
    combo_rgb->addItem(tr("RGB"));
    combo_rgb->addItem(tr("BGR"));
    combo_rgb->addItem(tr("BRG"));
    combo_rgb->addItem(tr("GRB"));
    
    setWindowIcon(QPixmap(":/images/icon.png"));
    
    progress_bar = new QProgressBar(this);
    progress_bar->setGeometry(0, 640, 800, 20);
    progress_bar->setMinimum(0);
    progress_bar->setMaximum(100);
    progress_bar->hide();
}

void AC_MainWindow::createMenu() {
    
    file_menu = menuBar()->addMenu(tr("&File"));
    controls_menu = menuBar()->addMenu(tr("&Controls"));
    options = menuBar()->addMenu(tr("&Options"));
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
    
    movement = options->addMenu(tr("Movement"));
    in_out_increase = new QAction(tr("Move In, Move Out, Increase"), this);
    in_out_increase->setCheckable(true);
    in_out_increase->setChecked(true);
    movement->addAction(in_out_increase);
    
    in_out = new QAction(tr("Move in, Move Out"), this);
    in_out->setCheckable(true);
    movement->addAction(in_out);
    
    out_reset = new QAction(tr("Move Out, Reset"), this);
    out_reset->setCheckable(true);
    movement->addAction(out_reset);
    speed_actions[0] = 0.001;
    speed_actions[1] = 0.05;
    speed_actions[2] = 0.01;
    speed_actions[3] = 0.1;
    speed_actions[4] = 0.5;
    speed_actions[5] = 1.0;
    speed_actions[6] = 3.0;
    const QString act_val[] = { "0.001 (Very Slow)", "0.05 (Slow)", "0.01 (Normal)", "0.1 (Regular)", "0.5 (Fast)", "1.0 (Faster)", "3.0 (Very Fast)"};
    speed_menu = options->addMenu("Movement Speed");
    for(int i = 0; i < 7; ++i) {
        speed_action_items[i] = new QAction(act_val[i], this);
        speed_action_items[i]->setCheckable(true);
        speed_menu->addAction(speed_action_items[i]);
    }
    image_menu = options->addMenu(tr("Image"));
    noflip = new QAction(tr("Normal"), this);
    noflip->setCheckable(true);
    noflip->setChecked(true);
    image_menu->addAction(noflip);
    flip1 = new QAction(tr("Flip Vertical"), this);
    flip1->setCheckable(true);
    flip1->setChecked(false);
    image_menu->addAction(flip1);
    flip2 = new QAction(tr("Flip Horizontal"), this);
    flip2->setCheckable(true);
    flip2->setChecked(false);
    image_menu->addAction(flip2);
    flip3 = new QAction(tr("Flip Vertical and Horizontal"), this);
    flip3->setCheckable(true);
    flip3->setChecked(false);
    image_menu->addAction(flip3);
    options->addSeparator();
    clear_sub = new QAction(tr("Clear SubFilter"), this);
    options->addAction(clear_sub);
    clear_sub->setShortcut(tr("Ctrl+F"));
    clear_image = new QAction(tr("Clear Image"), this);
    options->addAction(clear_image);
    options->addSeparator();
    repeat_v = new QAction(tr("Repeat"), this);
    repeat_v->setCheckable(true);
    repeat_v->setChecked(false);
    
    fade_on = new QAction(tr("Fade"), this);
    fade_on->setCheckable(true);
    fade_on->setChecked(true);
    options->addAction(fade_on);
    options->addAction(repeat_v);
    
    connect(fade_on, SIGNAL(triggered()), this, SLOT(setFade()));
    connect(repeat_v, SIGNAL(triggered()), this, SLOT(repeat_vid()));
    connect(clear_image, SIGNAL(triggered()), this, SLOT(clear_img()));
    connect(clear_sub, SIGNAL(triggered()), this, SLOT(clear_subfilter()));
    connect(flip1, SIGNAL(triggered()), this, SLOT(flip1_action()));
    connect(flip2, SIGNAL(triggered()), this, SLOT(flip2_action()));
    connect(flip3, SIGNAL(triggered()), this, SLOT(flip3_action()));
    connect(noflip, SIGNAL(triggered()), this, SLOT(noflip_action()));
    connect(speed_action_items[0], SIGNAL(triggered()), this, SLOT(speed1()));
    connect(speed_action_items[1], SIGNAL(triggered()), this, SLOT(speed2()));
    connect(speed_action_items[2], SIGNAL(triggered()), this, SLOT(speed3()));
    connect(speed_action_items[3], SIGNAL(triggered()), this, SLOT(speed4()));
    connect(speed_action_items[4], SIGNAL(triggered()), this, SLOT(speed5()));
    connect(speed_action_items[5], SIGNAL(triggered()), this, SLOT(speed6()));
    connect(speed_action_items[6], SIGNAL(triggered()), this, SLOT(speed7()));
    speed2();
    connect(file_new_capture, SIGNAL(triggered()), this, SLOT(file_NewCamera()));
    connect(file_new_video, SIGNAL(triggered()), this, SLOT(file_NewVideo()));
    connect(file_exit, SIGNAL(triggered()), this, SLOT(file_Exit()));
    connect(in_out_increase, SIGNAL(triggered()), this, SLOT(movementOption1()));
    connect(in_out, SIGNAL(triggered()), this, SLOT(movementOption2()));
    connect(out_reset, SIGNAL(triggered()), this, SLOT(movementOption3()));
    controls_stop = new QAction(tr("Sto&p"), this);
    controls_stop->setShortcut(tr("Ctrl+C"));
    controls_menu->addAction(controls_stop);
    controls_stop->setEnabled(false);
    controls_snapshot = new QAction(tr("Take &Snapshot"), this);
    controls_snapshot->setShortcut(tr("Ctrl+A"));
    controls_menu->addAction(controls_snapshot);
    controls_pause = new QAction(tr("&Pause"), this);
    controls_pause->setShortcut(tr("Ctrl+P"));
    controls_menu->addAction(controls_pause);
    controls_step = new QAction(tr("Step"), this);
    controls_step->setShortcut(tr("Ctrl+I"));
    controls_menu->addAction(controls_step);
    controls_setimage = new QAction(tr("Set Image"), this);
    //controls_setimage->setShortcut(tr("Ctrl+Q"));
    controls_menu->addAction(controls_setimage);
    controls_setkey = new QAction(tr("Set Color Key Image"), this);
    //controls_setkey->setShortcut(tr("Ctrl+K"));
    controls_menu->addAction(controls_setkey);
    controls_showvideo = new QAction(tr("Hide Display Video"), this);
    controls_showvideo->setShortcut(tr("Ctrl+V"));
    controls_menu->addAction(controls_showvideo);
    reset_filters = new QAction(tr("Reset Filters"), this);
    reset_filters->setShortcut(tr("Ctrl+R"));
    controls_menu->addAction(reset_filters);
    controls_showvideo->setEnabled(false);
    controls_showvideo->setCheckable(true);
    open_search = new QAction(tr("Search Filters"), this);
    open_search->setShortcut(tr("Ctrl+S"));
    controls_menu->addAction(open_search);
    select_key = new QAction(tr("Set Chroma Key"), this);
    controls_menu->addAction(select_key);
    connect(select_key, SIGNAL(triggered()), this, SLOT(openColorWindow()));
    connect(open_search,SIGNAL(triggered()), this, SLOT(openSearch()));
    connect(controls_snapshot, SIGNAL(triggered()), this, SLOT(controls_Snap()));
    connect(controls_pause, SIGNAL(triggered()), this, SLOT(controls_Pause()));
    connect(controls_step, SIGNAL(triggered()), this, SLOT(controls_Step()));
    connect(controls_stop, SIGNAL(triggered()), this, SLOT(controls_Stop()));
    connect(controls_setimage, SIGNAL(triggered()), this, SLOT(controls_SetImage()));
    connect(controls_setkey, SIGNAL(triggered()), this, SLOT(controls_SetKey()));
    connect(controls_showvideo, SIGNAL(triggered()), this, SLOT(controls_ShowVideo()));
    connect(reset_filters, SIGNAL(triggered()), this, SLOT(controls_Reset()));
    connect(combo_rgb, SIGNAL(currentIndexChanged(int)), this, SLOT(cb_SetIndex(int)));
    controls_pause->setText(tr("Pause"));
    help_about = new QAction(tr("About"), this);
    help_about->setShortcut(tr("Ctrl+A"));
    help_menu->addAction(help_about);
    connect(help_about, SIGNAL(triggered()), this, SLOT(help_About()));
    controls_stop->setEnabled(false);
    controls_pause->setEnabled(false);
    controls_step->setEnabled(false);
    controls_snapshot->setEnabled(false);
}

void AC_MainWindow::resetIndex() {
    frame_index = 0;
}

void AC_MainWindow::clear_subfilter() {
    int crow = custom_filters->currentRow();
    if(crow >= 0) {
        QListWidgetItem *item = custom_filters->item(crow);
        std::string text = item->text().toStdString();
        if(text.find(":") == std::string::npos)
            return;
        std::string val = text.substr(0, text.find(":"));
        item->setText(val.c_str());
        std::vector<FilterValue> v;
        buildVector(v);
        Log(tr("Cleared SubFilter"));
    }
    ac::setSubFilter(-1);
}

void AC_MainWindow::clear_img() {
    blend_set = false;
    blend_image.release();
    Log(tr("Cleared Image\n"));
}

void AC_MainWindow::flip1_action() {
    flip1->setChecked(true);
    flip2->setChecked(false);
    flip3->setChecked(false);
    noflip->setChecked(false);
    playback->SetFlip(false, true);
    Log(tr("Flipped Image\n"));
}

void AC_MainWindow::flip2_action() {
    flip1->setChecked(false);
    flip2->setChecked(true);
    flip3->setChecked(false);
    noflip->setChecked(false);
    playback->SetFlip(true, false);
    Log(tr("Flipped Image\n"));
}

void AC_MainWindow::flip3_action() {
    flip1->setChecked(false);
    flip2->setChecked(false);
    flip3->setChecked(true);
    noflip->setChecked(false);
    playback->SetFlip(true, true);
    Log(tr("Flipped Image\n"));
}

void AC_MainWindow::noflip_action() {
    flip1->setChecked(false);
    flip2->setChecked(false);
    flip3->setChecked(false);
    noflip->setChecked(true);
    playback->SetFlip(false, false);
    Log(tr("Removed Flip Action\n"));
    
}

void AC_MainWindow::repeat_vid() {
    bool val = repeat_v->isChecked();
    playback->enableRepeat(val);
    if(val == true) {
        Log(tr("Repeat Enabled\n"));
    } else {
        Log(tr("Repeat Disabled\n"));
    }
}

void AC_MainWindow::speed1() {
    ac::alpha_increase = speed_actions[0];
    QString text;
    QTextStream stream(&text);
    stream << "Movements Speed Set to: " << ac::alpha_increase << "\n";
    Log(text);
    for(int i = 0; i < 7; ++i) {
        speed_action_items[i]->setChecked(false);
    }
    speed_action_items[0]->setChecked(true);
    
}
void AC_MainWindow::speed2() {
    QString text;
    QTextStream stream(&text);
    ac::alpha_increase = speed_actions[1];
    stream << "Movements Speed Set to: " << ac::alpha_increase << "\n";
    Log(text);
    for(int i = 0; i < 7; ++i) {
        speed_action_items[i]->setChecked(false);
    }
    speed_action_items[1]->setChecked(true);
    
}
void AC_MainWindow::speed3() {
    ac::alpha_increase = speed_actions[2];
    QString text;
    QTextStream stream(&text);
    stream << "Movements Speed Set to: " << ac::alpha_increase << "\n";
    Log(text);
    for(int i = 0; i < 7; ++i) {
        speed_action_items[i]->setChecked(false);
    }
    speed_action_items[2]->setChecked(true);
    
}
void AC_MainWindow::speed4() {
    ac::alpha_increase = speed_actions[3];
    QString text;
    QTextStream stream(&text);
    stream << "Movements Speed Set to: " << ac::alpha_increase << "\n";
    Log(text);
    for(int i = 0; i < 7; ++i) {
        speed_action_items[i]->setChecked(false);
    }
    speed_action_items[3]->setChecked(true);
}
void AC_MainWindow::speed5() {
    ac::alpha_increase = speed_actions[4];
    QString text;
    QTextStream stream(&text);
    stream << "Movements Speed Set to: " << ac::alpha_increase << "\n";
    Log(text);
    for(int i = 0; i < 7; ++i) {
        speed_action_items[i]->setChecked(false);
    }
    speed_action_items[4]->setChecked(true);
}
void AC_MainWindow::speed6() {
    ac::alpha_increase = speed_actions[5];
    QString text;
    QTextStream stream(&text);
    stream << "Movements Speed Set to: " << ac::alpha_increase << "\n";
    Log(text);
    for(int i = 0; i < 7; ++i) {
        speed_action_items[i]->setChecked(false);
    }
    speed_action_items[5]->setChecked(true);
}

void AC_MainWindow::speed7() {
    QString text;
    QTextStream stream(&text);
    ac::alpha_increase = speed_actions[6];
    stream << "Movements Speed Set to: " << ac::alpha_increase << "\n";
    Log(text);
    for(int i = 0; i < 7; ++i) {
        speed_action_items[i]->setChecked(false);
    }
    speed_action_items[6]->setChecked(true);
}

void AC_MainWindow::movementOption1() {
    ac::setProcMode(0);
    in_out_increase->setChecked(true);
    in_out->setChecked(false);
    out_reset->setChecked(false);
    Log(tr("Proc Mode set to: 0\n"));
}
void AC_MainWindow::movementOption2() {
    in_out_increase->setChecked(false);
    in_out->setChecked(true);
    out_reset->setChecked(false);
    ac::setProcMode(1);
    Log(tr("Proc Mode set to: 1\n"));
}

void AC_MainWindow::movementOption3() {
    in_out_increase->setChecked(false);
    in_out->setChecked(false);
    out_reset->setChecked(true);
    ac::setProcMode(2);
    Log(tr("Proc Mode set to: 2\n"));
}

void AC_MainWindow::chk_Clicked() {
    playback->setOptions(chk_negate->isChecked(), combo_rgb->currentIndex());
}
void AC_MainWindow::cb_SetIndex(int index) {
    playback->setOptions(chk_negate->isChecked(), index);
}

void AC_MainWindow::slideChanged(int) {
    playback->setRGB(slide_r->sliderPosition(), slide_g->sliderPosition(), slide_b->sliderPosition());
}

void AC_MainWindow::colorChanged(int) {
    playback->setColorOptions(slide_bright->sliderPosition(), slide_gamma->sliderPosition(), slide_saturation->sliderPosition());
}

void AC_MainWindow::colorMapChanged(int pos) {
    playback->setColorMap(pos);
    Log("Changed Color Map\n");
}

void AC_MainWindow::comboFilterChanged(int) {
    playback->setIndexChanged(filters->currentText().toStdString());
    QString str;
    QTextStream stream(&str);
    stream << "Filter changed to: " << filters->currentText() << "\n";
    Log(str);
    std::string text = filters->currentText().toStdString();
    if(blend_set == false && text.find("Image") != std::string::npos)
        Log(tr("Set an Image to use this filter\n"));
    else if(ac::subfilter == -1 && text.find("SubFilter") != std::string::npos)
        Log(tr("Set a SubFilter to use this filter\n"));
    
}

void AC_MainWindow::setFilterSingle() {
    playback->setSingleMode(true);
    Log("Set to Single Filter Mode\n");
}
void AC_MainWindow::setFilterCustom() {
    playback->setSingleMode(false);
    Log("Set to Custom Filter Mode\n");
}

void AC_MainWindow::addClicked() {
    int row = filters->currentIndex();
    if(row != -1) {
        //QListWidgetItem *item = filters->item(row);
        
        std::string sub_str = filters->currentText().toStdString();
        custom_filters->addItem(filters->currentText());
        //custom_filters->addItem(sub_str.c_str());
        QString qs;
        QTextStream stream(&qs);
        stream << "Added Filter: " << filters->currentText() << "\n";
        Log(qs);
        std::vector<FilterValue> v;
        buildVector(v);
        playback->setVector(v);
    }
}
void AC_MainWindow::updateList() {
    std::vector<FilterValue> v;
    buildVector(v);
    playback->setVector(v);
}

void AC_MainWindow::rmvClicked() {
    int item = custom_filters->currentRow();
    if(item != -1) {
        QListWidgetItem *i = custom_filters->takeItem(item);
        QString qs;
        QTextStream stream(&qs);
        stream << "Removed Filter: " << i->text() << "\n";
        Log(qs);
        std::vector<FilterValue> v;
        buildVector(v);
        playback->setVector(v);
    }
}

void AC_MainWindow::upClicked() {
    int item = custom_filters->currentRow();
    if(item > 0) {
        QListWidgetItem *i = custom_filters->takeItem(item);
        custom_filters->insertItem(item-1, i->text());
        custom_filters->setCurrentRow(item-1);
        std::vector<FilterValue> v;
        buildVector(v);
        playback->setVector(v);
    }
}

void AC_MainWindow::downClicked() {
    int item = custom_filters->currentRow();
    if(item >= 0 && item < custom_filters->count()-1) {
        QListWidgetItem *i = custom_filters->takeItem(item);
        custom_filters->insertItem(item+1, i->text());
        custom_filters->setCurrentRow(item+1);
        std::vector<FilterValue> v;
        buildVector(v);
        playback->setVector(v);
    }
}

void AC_MainWindow::setSub() {
    int row = filters->currentIndex();
    int crow = custom_filters->currentRow();
    if(row != -1 && crow != -1) {
        std::ostringstream stream;
        QListWidgetItem *item = custom_filters->item(crow);
        QString filter_num = filters->currentText();
        int value_index = filter_map[filter_num.toStdString()].index;
        int filter_index = filter_map[filter_num.toStdString()].filter;
        if(value_index == 0) {
            std::string fname = filters->currentText().toStdString();
            std::string filter_val = item->text().toStdString();
            if(filter_val.find(":") != std::string::npos)
                filter_val = filter_val.substr(0, filter_val.find(":"));
            
            if(!(fname.find("SubFilter") == std::string::npos && filter_val.find("SubFilter") != std::string::npos)) {
                stream << filter_val << " does not support a subfilter.\n";
                Log(stream.str().c_str());
                return;
            }
            stream << "SubFilter set to: " << filter_num.toStdString() << "\n";
            stream << "SubFilter index: " << filter_index << "\n";
            std::ostringstream stream1;
            stream1 << filter_val << ":" << fname;
            item->setText(stream1.str().c_str());
            std::vector<FilterValue> v;
            buildVector(v);
            playback->setVector(v);
            QString l = stream.str().c_str();
            Log(l);
        }
    }
}
void AC_MainWindow::Log(const QString &s) {
    QString text;
    text = log_text->toPlainText();
    text += s;
    log_text->setText(text);
    QTextCursor tmpCursor = log_text->textCursor();
    tmpCursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    log_text->setTextCursor(tmpCursor);
}

bool AC_MainWindow::startCamera(int res, int dev, const QString &outdir, bool record, int type) {
    programMode = MODE_CAMERA;
    progress_bar->hide();
    controls_showvideo->setEnabled(false);
    playback->setDisplayed(true);
    controls_stop->setEnabled(true);
    controls_pause->setEnabled(true);
    controls_step->setEnabled(false);
    controls_snapshot->setEnabled(true);
    // setup device
    step_frame = false;
    video_file_name = "";
    frame_index = 0;
    /*
     capture_camera.open(dev);
     if(!capture_camera.isOpened()) {
     return false;
     }*/
    video_frames = 0;
    video_fps = 24; /*
                     int ores_w = capture_camera.get(CV_CAP_PROP_FRAME_WIDTH);
                     int ores_h = capture_camera.get(CV_CAP_PROP_FRAME_HEIGHT);
                     */
    int res_w = 0;
    int res_h = 0;
    /*QString str;
     QTextStream stream(&str);
     stream << "Opened capture device " << res_w << "x" << res_h << "\n";
     stream << "FPS: " << video_fps << "\n";*/
    output_directory = outdir;
    frame_index = 0;
    //Log(str);
    paused = false;
    recording = record;
    QString output_name;
    QTextStream stream_(&output_name);
    static unsigned int index = 0;
    time_t t = time(0);
    struct tm *m;
    m = localtime(&t);
    QString ext;
    ext = (type == 0) ? ".mov" : ".avi";
    Log(tr("Capture Device Opened [Camera]\n"));
    std::ostringstream time_stream;
    time_stream << "-" << (m->tm_year + 1900) << "." << (m->tm_mon + 1) << "." << m->tm_mday << "_" << m->tm_hour << "." << m->tm_min << "." << m->tm_sec <<  "_";
    stream_ << outdir << "/" << "Video" << time_stream.str().c_str() << "AC2.Output." << (++index) << ext;
    switch(res) {
        case 0:
            res_w = 640;
            res_h = 480;
            break;
        case 1:
            res_w = 1280;
            res_h = 720;
            break;
        case 2:
            res_w = 1920;
            res_h = 1080;
            
            break;
    }
    
    /*
     bool cw = capture_camera.set(CV_CAP_PROP_FRAME_WIDTH, res_w);
     bool ch = capture_camera.set(CV_CAP_PROP_FRAME_HEIGHT, res_h);
     
     if(cw == false || ch == false) {
     QMessageBox::information(this, tr("Info"), tr("Could not set resolution reverting to default .."));
     res_w = ores_w;
     res_h = ores_h;
     capture_camera.set(CV_CAP_PROP_FRAME_WIDTH, res_w);
     capture_camera.set(CV_CAP_PROP_FRAME_HEIGHT, res_h);
     } */
    
    if(recording) {
        video_file_name = output_name;
        writer = cv::VideoWriter(output_name.toStdString(), (type == 0) ? CV_FOURCC('m', 'p', '4', 'v') : CV_FOURCC('X','V','I','D'), video_fps, cv::Size(res_w, res_h), true);
        
        if(!writer.isOpened()) {
            Log(tr("Could not create video writer..\n"));
            QMessageBox::information(this, tr("Error"), tr("Incorrect Pathname/Or you do not have permission to write to the directory."));
            return false;
        }
        QString out_s;
        QTextStream out_stream(&out_s);
        out_stream << "Now recording to: " << output_name << "\nResolution: " << res_w << "x" << res_h << " FPS: " << video_fps << "\n";
        Log(out_s);
    }
    // if successful
    file_new_capture->setEnabled(false);
    file_new_video->setEnabled(false);
    controls_stop->setEnabled(true);
    bool rt_val = playback->setVideoCamera(dev, res, writer, recording);
    if(rt_val == false) return false;
    playback->Play();
    disp->show();
    return true;
}

bool AC_MainWindow::startVideo(const QString &filename, const QString &outdir, bool record, int type) {
    programMode = MODE_VIDEO;
    controls_stop->setEnabled(true);
    controls_pause->setEnabled(true);
    controls_step->setEnabled(true);
    controls_snapshot->setEnabled(true);
    if(record == true)
        controls_showvideo->setEnabled(true);
    
    progress_bar->show();
    playback->setDisplayed(true);
    video_file_name = "";
    step_frame = false;
    capture_video.open(filename.toStdString());
    if(!capture_video.isOpened()) {
        return false;
    }
    video_frames = capture_video.get(CV_CAP_PROP_FRAME_COUNT);
    if(video_frames <= 0) return false;
    video_fps = capture_video.get(CV_CAP_PROP_FPS);
    int res_w = capture_video.get(CV_CAP_PROP_FRAME_WIDTH);
    int res_h = capture_video.get(CV_CAP_PROP_FRAME_HEIGHT);
    QString str;
    QTextStream stream(&str);
    stream << "Opened capture device [Video] " << res_w << "x" << res_h << "\n";
    stream << "Video File: " << filename << "\n";
    stream << "FPS: " << video_fps << "\n";
    stream << "Frame Count: " << video_frames << "\n";
    output_directory = outdir;
    frame_index = 0;
    Log(str);
    // if successful
    file_new_capture->setEnabled(false);
    file_new_video->setEnabled(false);
    controls_stop->setEnabled(true);
    paused = false;
    recording = record;
    QString output_name;
    QTextStream stream_(&output_name);
    static unsigned int index = 0;
    time_t t = time(0);
    struct tm *m;
    m = localtime(&t);
    QString ext;
    ext = (type == 0) ? ".mov" : ".avi";
    std::ostringstream time_stream;
    time_stream << "-" << (m->tm_year + 1900) << "." << (m->tm_mon + 1) << "." << m->tm_mday << "_" << m->tm_hour << "." << m->tm_min << "." << m->tm_sec <<  "_";
    stream_ << outdir << "/" << "Video" << time_stream.str().c_str() << "AC2.Output." << (++index) << ext;
    
    
    if(recording) {
        video_file_name = output_name;
        writer = cv::VideoWriter(output_name.toStdString(), (type == 0) ? CV_FOURCC('m', 'p', '4', 'v') : CV_FOURCC('X','V','I','D'), video_fps, cv::Size(res_w, res_h), true);
        
        if(!writer.isOpened()) {
            Log("Error could not open video writer.\n");
            QMessageBox::information(this, tr("Error invalid path"), tr("Incorrect Pathname/Or you do not have permission to write to the directory."));
            return false;
        }
        QString out_s;
        QTextStream out_stream(&out_s);
        out_stream << "Now recording to: " << output_name << "\nResolution: " << res_w << "x" << res_h << " FPS: " << video_fps << "\n";
        Log(out_s);
    }
    playback->setVideo(capture_video,writer,recording);
    playback->Play();
    disp->show();
    return true;
}

void AC_MainWindow::controls_Stop() {
    playback->Stop();
    goto_window->hide();
    progress_bar->hide();
    controls_showvideo->setEnabled(false);
    controls_stop->setEnabled(false);
    controls_pause->setEnabled(false);
    controls_step->setEnabled(false);
    controls_snapshot->setEnabled(false);
    if(capture_video.isOpened()) {
        capture_video.release();
        if(recording == true) writer.release();
        file_new_capture->setEnabled(true);
        file_new_video->setEnabled(true);
        if(recording) {
            QString stream_;
            QTextStream stream(&stream_);
            stream << "Wrote video file: " << video_file_name << "\n";
            Log(stream_);
        }
        disp->hide();
        playback->Release();
    }
    if(programMode == MODE_CAMERA) {
        //capture_camera.release();
        if(recording == true) writer.release();
        file_new_capture->setEnabled(true);
        file_new_video->setEnabled(true);
        if(recording) {
            QString stream_;
            QTextStream stream(&stream_);
            stream << "Wrote video file: " << video_file_name << "\n";
            Log(stream_);
        }
        disp->hide();
        playback->Release();
    }
    Log(tr("Capture device [Closed]\n"));;
}

void AC_MainWindow::controls_ShowVideo() {
    QString st = controls_showvideo->text();
    
    if(st == "Hide Display Video") {
        playback->setDisplayed(Qt::Unchecked);
        disp->hide();
        controls_showvideo->setText("Show Display Video");
    } else {
        controls_showvideo->setText("Hide Display Video");
        playback->setDisplayed(true);
        disp->show();
    }
}

void AC_MainWindow::controls_Reset() {
    playback->reset_filters();
}

void AC_MainWindow::file_Exit() {
    QApplication::exit(0);
}

void AC_MainWindow::file_NewVideo() {
    cap_video->show();
    goto_window->hide();
}

void AC_MainWindow::file_NewCamera() {
    cap_camera->show();
    goto_window->hide();
}

void AC_MainWindow::controls_Snap() {
    take_snapshot = true;
}

void AC_MainWindow::controls_Pause() {
    QString p = controls_pause->text();
    if(p == "Pause") {
        controls_pause->setText("Paused");
        controls_pause->setChecked(true);
        paused = true;
        if(programMode == MODE_VIDEO) {
            goto_window->showWindow(frame_index, 0, video_frames);
        }
        playback->Stop();
    } else {
        controls_pause->setText("Pause");
        controls_pause->setChecked(false);
        goto_window->hide();
        playback->Play();
        paused = false;
    }
}

void AC_MainWindow::controls_SetImage() {
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open Image"), "/home", tr("Image Files (*.png *.jpg)"));
    if(fileName != "") {
        cv::Mat tblend_image = cv::imread(fileName.toStdString());
        if(!tblend_image.empty()) {
            playback->setImage(tblend_image);
            QString text;
            QTextStream stream(&text);
            stream << "Successfully Loaded Image: [" << fileName << "] Size: " << tblend_image.cols << "x" << tblend_image.rows << "\n";
            Log(text);
        } else {
            QMessageBox::information(this, tr("Image Load failed"), tr("Could not load image"));
        }
    }
}

void AC_MainWindow::controls_SetKey() {
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open Color Key Image"), "/home", tr("Image Files (*.png)"));
    if(fileName != "") {
        cv::Mat tblend_image = cv::imread(fileName.toStdString());
        if(!tblend_image.empty()) {
            playback->setColorKey(tblend_image);
            QString str_value;
            QTextStream stream(&str_value);
            stream << "ColorKey is (255,0,255)\n Image Set: " << fileName;
            QMessageBox::information(this, tr("Loaded Image"), tr(str_value.toStdString().c_str()));
        } else {
            QMessageBox::information(this, tr("Image Load failed"), tr("Could not load ColorKey image"));
        }
    }
}

void AC_MainWindow::controls_Step() {
    playback->setStep();
    playback->Play();
    step_frame = true;
}

void AC_MainWindow::buildVector(std::vector<FilterValue> &v) {
    if(!v.empty()) v.erase(v.begin(), v.end());
    for(int i = 0; i < custom_filters->count(); ++i) {
        QListWidgetItem *val = custom_filters->item(i);
        QString name = val->text();
        std::string n = name.toStdString();
        if(n.find(":") == std::string::npos)
            v.push_back(filter_map[name.toStdString()]);
        else {
            std::string namev = name.toStdString();
            std::string left_str = namev.substr(0, namev.find(":"));
            std::string right_str = namev.substr(namev.find(":")+1,namev.length()-left_str.length()-1);
            int index_val = filter_map[right_str].filter;
            FilterValue fv(filter_map[left_str].index, filter_map[left_str].filter, index_val);
            v.push_back(fv);
        }
    }
}

cv::Mat QImage2Mat(QImage const& src)
{
    cv::Mat tmp(src.height(),src.width(),CV_8UC3,(uchar*)src.bits(),src.bytesPerLine());
    cv::Mat result;
    cvtColor(tmp, result,CV_BGR2RGB);
    return result;
}

QImage Mat2QImage(cv::Mat const& src)
{
    cv::Mat temp;
    cvtColor(src, temp,CV_BGR2RGB);
    QImage dest((const uchar *) temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
    dest.bits();
    return dest;
}

void AC_MainWindow::setFrameIndex(int index) {
    frame_index = index;
}

void AC_MainWindow::updateFrame(QImage img) {
    if(playback->isStopped() == false) {
        disp->displayImage(img);
        frame_index++;
        QString frame_string;
        QTextStream frame_stream(&frame_string);
        if(!recording) {
            frame_stream << "(Current/Total Frames/Seconds) - (" << frame_index << "/" << video_frames << "/" <<  (unsigned long)(frame_index/video_fps) << ") ";
        } else {
            struct stat buf;
            stat(video_file_name.toStdString().c_str(), &buf);
            frame_stream << "(Current/Total Frames/Seconds/Size) - (" << frame_index << "/" << video_frames << "/" <<  (unsigned int)(frame_index/video_fps) << "/" << ((buf.st_size/1024)/1024) <<  " MB) ";
        }
        if(programMode == MODE_VIDEO) {
            
            float index = frame_index;
            float max_frames = video_frames;
            float value = (index/max_frames)*100;
            unsigned int val = static_cast<unsigned int>(value);
            progress_bar->setValue(val);
            frame_stream << " - " << val << "%";
        }
        statusBar()->showMessage(frame_string);
        
        if(take_snapshot == true) {
            cv::Mat mat = QImage2Mat(img);
            static int index = 0;
            QString text;
            QTextStream stream(&text);
            time_t t = time(0);
            struct tm *m;
            m = localtime(&t);
            std::ostringstream time_stream;
            time_stream << "-" << (m->tm_year + 1900) << "." << (m->tm_mon + 1) << "." << m->tm_mday << "_" << m->tm_hour << "." << m->tm_min << "." << m->tm_sec <<  "_";
            stream << output_directory << "/" << "AC2.Snapshot." << time_stream.str().c_str() << "." << ++index << ".png";
            cv::imwrite(text.toStdString(), mat);
            QString total;
            QTextStream stream_total(&total);
            stream_total << "Took Snapshot: " << text << "\n";
            Log(total);
            take_snapshot = false;
        }
    }
}

void AC_MainWindow::stopRecording() {
    controls_Stop();
    frame_index = video_frames;
    controls_stop->setEnabled(false);
    controls_pause->setEnabled(false);
    controls_step->setEnabled(false);
    controls_snapshot->setEnabled(false);
    progress_bar->hide();
}


void AC_MainWindow::setSubFilter(const QString &filter_num) {
    int value_index = filter_map[filter_num.toStdString()].index;
    int filter_index = filter_map[filter_num.toStdString()].filter;
    int crow = custom_filters->currentRow();
    if(value_index == 0 && crow >= 0) {
        std::ostringstream stream;
        QListWidgetItem *item = custom_filters->item(crow);
        std::string fname = filter_num.toStdString();
        std::string filter_val = item->text().toStdString();
        if(filter_val.find(":") != std::string::npos)
            filter_val = filter_val.substr(0, filter_val.find(":"));
        
        if(!(fname.find("SubFilter") == std::string::npos && filter_val.find("SubFilter") != std::string::npos)) {
            stream << filter_val << " does not support a subfilter.\n";
            Log(stream.str().c_str());
            return;
        }
        stream << "SubFilter set to: " << filter_num.toStdString() << "\n";
        stream << "SubFilter index: " << filter_index << "\n";
        std::ostringstream stream1;
        stream1 << filter_val << ":" << fname;
        item->setText(stream1.str().c_str());
        std::vector<FilterValue> v;
        buildVector(v);
        playback->setVector(v);
        QString l = stream.str().c_str();
        Log(l);
    } else {
        QString txt;
        QTextStream stream(&txt);
        stream << "Only Regular Filters can be used as a SubFilter not AF\n";
        Log(txt);
    }
}

void AC_MainWindow::setFade() {
    bool fc = fade_on->isChecked();
    playback->setFadeFilter(fc);
}

void AC_MainWindow::frameInc() {
    frame_index++;
    QString frame_string;
    QTextStream frame_stream(&frame_string);
    
    if(!recording) {
        frame_stream << "(Current/Total Frames/Seconds) - (" << frame_index << "/" << video_frames << "/" << (unsigned int)(frame_index/video_fps) << ") ";
    } else {
        struct stat buf;
        stat(video_file_name.toStdString().c_str(), &buf);
        frame_stream << "(Current/Total Frames/Seconds/Size) - (" << frame_index << "/" << video_frames << "/" << (unsigned int)(frame_index/video_fps) << "/" << ((buf.st_size/1024)/1024)  <<  " MB) ";
        
    }
    if(programMode == MODE_VIDEO) {
        float index = frame_index;
        float max_frames = video_frames;
        float value = (index/max_frames)*100;
        unsigned int val = static_cast<unsigned int>(value);
        if(frame_index <= video_frames)
            frame_stream << " - " << val << "%";
        progress_bar->setValue(val);
    }
    statusBar()->showMessage(frame_string);
}

void AC_MainWindow::help_About() {
    QString about_str;
    QTextStream stream(&about_str);
    stream << tr("<b>Acid Cam Qt version: ") << ac_version << " filters: " << ac::version.c_str() << "</b><br><br> ";
    stream << tr("Engineering by <b>Jared Bruni</b><br><br><b>This software is dedicated to all the people that experience mental illness. </b><br><br><b>My Social Media Accounts</b><br><br>\n\n <a href=\"http://github.com/lostjared\">GitHub</a><br>\n<a href=\"http://youtube.com/lostjared\">YouTube</a><br><a href=\"http://instagram.com/lostjared\">Instagram</a><br><a href=\"http://facebook.com/LostSideDead0x\">LostSideDead Facebook</a><br><a href=\"http://facebook.com/lostsidedead\">My Facebook</a><br><a href=\"http://twitter.com/jaredbruni\">Twitter</a><br><br><br>\n");
    
    QMessageBox::information(this, tr("About Acid Cam"), about_str);
}

void AC_MainWindow::openSearch() {
    search_box->show();
}

void AC_MainWindow::openColorWindow() {
    chroma_window->show();
}

void AC_MainWindow::menuFilterChanged(int index) {
    
}
