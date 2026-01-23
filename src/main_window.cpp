/*
 * Acid Cam v2 - Qt/OpenCV Edition
 * written by Jared Bruni ( http://lostsidedead.com )
 * (C) 2017 GPL
 */


#include "main_window.h"
#include<mutex>
#include"plugin.h"
#include<sys/stat.h>
#include<unordered_map>
#include <opencv2/core/ocl.hpp>

std::unordered_map<std::string, FilterValue> filter_map;
std::unordered_map<std::string, int> filter_map_main;
std::vector<std::string> solo_filter;
void custom_filter(cv::Mat &);

const char *filter_names[] = { "AC Self AlphaBlend", "Reverse Self AlphaBlend",
    "Opposite Self AlphaBlend",
    "AC2 Distort",
    "Reverse Distort",
    "Opposite Distort",
    "Full Distort",
    "A New One",
    "AC NewOne",
    "AC Thought Filter",
    "Line Draw",
    "Gradient Square",
    "Color Wave",
    "Pixelated Gradient",
    "Combined Gradient",
    "Diagonal",
    "Average",
    "Average Divide",
    "Cos/Sin Multiply",
    "Modulus Multiply",
    "Positive/Negative",
    "z+1 Blend",
    "Diamond Pattern",
    "Pixelated Shift",
    "Pixelated Mix",
    "Color Accumulate",
    "Color Accumulate #2",
    "Color Accumulate #3",
    "Angle",
    "Vertical Average",
    "Circular Blend",
    "Average Blend",
    "~Divide",
    "Mix",
    "Random Number",
    "Gradient Repeat", 0 };


const char *menuNames[] = {"All Filters", "All Filters Sorted", "Blend", "Distort", "Pattern", "Gradient", "Mirror", "Strobe", "Blur", "Image/Video", "Square", "Other", "SubFilter", "Special", "User", 0};


void generate_map() {
    ac::fill_filter_map();
    for(int i = 0; i < ac::draw_max; ++i )
        filter_map[ac::draw_strings[i]] = FilterValue(0, i, -1);
    int index = 0;
    while(filter_names[index] != 0) {
        std::string filter_n = "AF_";
        filter_n += filter_names[index];
        filter_map[filter_n] = FilterValue(1, index, -1);
        ++index;
        ac::filter_menu_map["All Filters"].menu_list->push_back(filter_n);
    }
    std::cout << "index: " << index << "\n";
    for(unsigned int j = 0; j < plugins.plugin_list.size(); ++j) {
        std::string name = "plugin " + plugins.plugin_list[j]->name();
        filter_map[name] = FilterValue(2, j, -1);
    }
    
    filter_map_main = ac::filter_map;
    solo_filter = ac::solo_filter;
}


void AC_MainWindow::showRange() {
    color_range_window->show();
}

void custom_filter(cv::Mat &) {
    
}


AC_MainWindow::~AC_MainWindow() {
    controls_Stop();
#ifndef _WIN32
    delete playback;
#endif
    
}

AC_MainWindow::AC_MainWindow(QWidget *parent) : QMainWindow(parent) {
    programMode = MODE_CAMERA;

    init_plugins();
    ac::init_filter_menu_map();
    generate_map();
    draw_strings = ac::draw_strings;
    ac::SortFilters();
    ac::filter_menu_map["User"].menu_list->push_back("No Filter");
    playback = new Playback();
    settings = new QSettings("LostSideDead", "Acid Cam Qt");
    setGeometry(100, 100, 1280, 900);
    setMinimumSize(1280, 900);
    setWindowTitle(tr("Acid Cam v2 - Qt"));
    createControls();
    createMenu();
    speed_index = 0;
    loading = false;
    playback->setMaxAlloc(300);
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
    disp2 = new DisplayWindow(this);
    disp2->setFixedSize(640, 480);
    disp2->setGeometry(100, 100, 640, 480);
    disp2->hide();
    disp2->setWindowTitle("View Window");
    goto_window->setParent(this);
    goto_window->setDisplayWindow(disp);
    goto_window->setPlayback(playback);
    goto_window->setMainWindow(this);
    
    image_window = new ImageWindow(this);
    image_window->setPlayback(playback);
    
    pref_window = new OptionsWindow(this);
    pref_window->setPlayback(playback);
    
    color_range_window = new ColorRangeWindow(this);
    slit_win = new SlitScanWindow(this);
    
    QObject::connect(playback, SIGNAL(procImage(QImage)), this, SLOT(updateFrame(QImage)));
    QObject::connect(playback, SIGNAL(stopRecording()), this, SLOT(stopRecording()));
    QObject::connect(playback, SIGNAL(frameIncrement()), this, SLOT(frameInc()));
    QObject::connect(playback, SIGNAL(resetIndex()), this, SLOT(resetIndex()));
    QObject::connect(playback, SIGNAL(ffmpegFinished(QString, QString, QString)), 
                     this, SLOT(onFFmpegFinished(QString, QString, QString)));
    
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
    chroma_window->main_window = this;
    define_window = new DefineWindow(this);
    define_window->hide();
    define_window->main_window = this;
 
#ifndef DISABLE_JOYSTICK
    Controller::init();
    joy_timer = new QTimer(this);
    connect(joy_timer, SIGNAL(timeout()), this, SLOT(chk_Joystick()));
    QString out_text;
    QTextStream stream(&out_text);
    if(controller.open(0)) {
        stream << "Controller: " << controller.getControllerName() << " connected...\n";
        joy_timer->start();
    } else {
        stream << "No controller detected...\n";
    }
    Log(out_text);
#endif
    
}

bool AC_MainWindow::checkAdd(QString str) {
    const char *ex[] = { "SetCurrentFrameStateAsSource", "ExpandSquareVertical", "DistortPixelate128", "Zoom", "AcidShuffleMedian", "MatrixColorBlur", "AlphaBlendArrayExpand", "Zoom", "ImageXorSmooth", "SketchFilter", "SlideSub", "Histogram", "Desktop","MultiVideo","Solo", "Bars", "BilateralFilter", "BilateralFilterFade", "BoxFilter", "CurrentDesktopRect", "HorizontalTrailsInter", "IntertwineAlpha", "IntertwineAlphaBlend", "IntertwineVideo640", "RandomAlphaBlendFilter", "RandomOrigFrame", "RectangleGlitch", "SquareSwap64x32", "VideoColorMap", 0};

    std::string val = str.toStdString();
    for(int i = 0; ex[i] != 0; ++i)
        if(val.find(ex[i]) != std::string::npos)
            return true;
    return false;
}

void AC_MainWindow::createControls() {
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    
    // ===== Filter Mode Selection =====
    QGroupBox *filterModeGroup = new QGroupBox(tr("Filter Mode"), this);
    filterModeGroup->setMaximumHeight(60);
    QHBoxLayout *filterModeLayout = new QHBoxLayout(filterModeGroup);
    filterModeLayout->setSpacing(12);
    filterModeLayout->setContentsMargins(12, 8, 12, 8);
    filter_single = new QRadioButton(tr("Single Filter"), this);
    filter_custom = new QRadioButton(tr("Custom Filter"), this);
    filter_single->setChecked(true);
    filter_single->setToolTip(tr("Apply a single filter to the video stream"));
    filter_custom->setToolTip(tr("Apply a sequence of filters in custom order"));
    filterModeLayout->addWidget(filter_single);
    filterModeLayout->addWidget(filter_custom);
    filterModeLayout->addStretch();
    connect(filter_single, SIGNAL(pressed()), this, SLOT(setFilterSingle()));
    connect(filter_custom, SIGNAL(pressed()), this, SLOT(setFilterCustom()));
    
    // ===== Filter Selection Section =====
    QGroupBox *filterSelectGroup = new QGroupBox(tr("Filter Selection"), this);
    QVBoxLayout *filterSelectLayout = new QVBoxLayout(filterSelectGroup);
    filterSelectLayout->setSpacing(12);
    filterSelectLayout->setContentsMargins(12, 12, 12, 12);
    
    QHBoxLayout *filterDropdownLayout = new QHBoxLayout();
    filterDropdownLayout->setSpacing(16);
    
    // Category selector
    QVBoxLayout *categoryLayout = new QVBoxLayout();
    categoryLayout->setSpacing(6);
    QLabel *categoryLabel = new QLabel(tr("Category:"), this);
    categoryLabel->setStyleSheet("font-weight: bold;");
    menu_cat = new QComboBox(this);
    menu_cat->setMinimumWidth(200);
    menu_cat->setMaximumHeight(24);
    menu_cat->setToolTip(tr("Select filter category"));
    for(int i = 0; menuNames[i] != 0; ++i) {
        menu_cat->addItem(menuNames[i]);
    }
    menu_cat->setCurrentIndex(0);
    categoryLayout->addWidget(categoryLabel);
    categoryLayout->addWidget(menu_cat);
    
    // Filter selector
    QVBoxLayout *filterLayout = new QVBoxLayout();
    filterLayout->setSpacing(6);
    QLabel *filterLabel = new QLabel(tr("Filter:"), this);
    filterLabel->setStyleSheet("font-weight: bold;");
    filters = new QComboBox(this);
    filters->setMinimumWidth(200);
    filters->setMaximumHeight(24);
    filters->setToolTip(tr("Select a filter to apply"));
    filterLayout->addWidget(filterLabel);
    filterLayout->addWidget(filters);
    
    filterDropdownLayout->addLayout(categoryLayout);
    filterDropdownLayout->addLayout(filterLayout);
    filterDropdownLayout->addStretch();
    
    // Subfilter controls
    QHBoxLayout *subfilterLayout = new QHBoxLayout();
    subfilterLayout->setSpacing(10);
    btn_sub = new QPushButton(tr("Set Subfilter"), this);
    btn_clr = new QPushButton(tr("Clear Subfilter"), this);
    btn_sub->setObjectName("btnPrimary");
    btn_clr->setObjectName("btnSecondary");
    btn_sub->setMinimumWidth(72);
    btn_clr->setMinimumWidth(72);
    btn_sub->setMaximumHeight(28);
    btn_clr->setMaximumHeight(28);
    btn_sub->setToolTip(tr("Configure advanced filter options"));
    btn_clr->setToolTip(tr("Clear any subfilter settings"));
    subfilterLayout->addWidget(btn_sub);
    subfilterLayout->addWidget(btn_clr);
    subfilterLayout->addStretch();
    connect(btn_sub, SIGNAL(clicked()), this, SLOT(setSub()));
    connect(btn_clr, SIGNAL(clicked()), this, SLOT(clear_subfilter()));
    
    filterSelectLayout->addLayout(filterDropdownLayout);
    filterSelectLayout->addLayout(subfilterLayout);
    
    connect(filters, SIGNAL(currentIndexChanged(int)), this, SLOT(comboFilterChanged(int)));
    connect(menu_cat, SIGNAL(currentIndexChanged(int)), this, SLOT(menuFilterChanged(int)));

    // ===== Custom Filter List Section =====
    QGroupBox *customListGroup = new QGroupBox(tr("Custom Filter List"), this);
    QVBoxLayout *customListLayout = new QVBoxLayout(customListGroup);
    customListLayout->setSpacing(12);
    customListLayout->setContentsMargins(14, 18, 14, 14);
    
    // Filter list widget
    custom_filters = new QListWidget(this);
    custom_filters->setMinimumHeight(100);
    custom_filters->setMaximumHeight(140);
    custom_filters->setToolTip(tr("List of filters in custom filter chain\nScroll to see all filters"));
    customListLayout->addWidget(custom_filters);
    
    // Add spacing between list and buttons
    customListLayout->addSpacing(8);
    
    // Buttons row (below list, above checkboxes)
    QHBoxLayout *customButtonLayout = new QHBoxLayout();
    customButtonLayout->setSpacing(10);
    btn_add = new QPushButton(tr("Add"), this);
    btn_remove = new QPushButton(tr("Remove"), this);
    btn_moveup = new QPushButton(tr("Move Up"), this);
    btn_movedown = new QPushButton(tr("Move Down"), this);
    btn_load = new QPushButton(tr("Load"), this);
    btn_save = new QPushButton(tr("Save"), this);
    btn_add->setObjectName("btnPrimary");
    btn_remove->setObjectName("btnDanger");
    btn_moveup->setObjectName("btnSecondary");
    btn_movedown->setObjectName("btnSecondary");
    btn_load->setObjectName("btnSecondary");
    btn_save->setObjectName("btnPrimary");
    
    btn_add->setMinimumWidth(60);
    btn_remove->setMinimumWidth(60);
    btn_moveup->setMinimumWidth(60);
    btn_movedown->setMinimumWidth(60);
    btn_load->setMinimumWidth(60);
    btn_save->setMinimumWidth(60);

    btn_add->setMaximumHeight(26);
    btn_remove->setMaximumHeight(26);
    btn_moveup->setMaximumHeight(26);
    btn_movedown->setMaximumHeight(26);
    btn_load->setMaximumHeight(26);
    btn_save->setMaximumHeight(26);
    
    btn_add->setToolTip(tr("Add selected filter to custom list"));
    btn_remove->setToolTip(tr("Remove selected filter from custom list"));
    btn_moveup->setToolTip(tr("Move selected filter up in the list"));
    btn_movedown->setToolTip(tr("Move selected filter down in the list"));
    btn_load->setToolTip(tr("Load filter list from file"));
    btn_save->setToolTip(tr("Save current filter list to file"));
    
    customButtonLayout->addWidget(btn_add);
    customButtonLayout->addWidget(btn_remove);
    customButtonLayout->addWidget(btn_moveup);
    customButtonLayout->addWidget(btn_movedown);
    customButtonLayout->addStretch();
    customButtonLayout->addWidget(btn_load);
    customButtonLayout->addWidget(btn_save);
    customListLayout->addLayout(customButtonLayout);
    
    // Add spacing between buttons and options
    customListLayout->addSpacing(6);
    
    // Options row (checkboxes and channel order)
    QHBoxLayout *optionsRow = new QHBoxLayout();
    optionsRow->setSpacing(14);
    chk_negate = new QCheckBox(tr("Negate Colors"), this);
    chk_negate->setCheckState(Qt::Unchecked);
    chk_negate->setToolTip(tr("Invert all colors in the output"));
    
    QLabel *channelLabel = new QLabel(tr("Channel Order:"), this);
    combo_rgb = new QComboBox(this);
    combo_rgb->addItem(tr("RGB"));
    combo_rgb->addItem(tr("BGR"));
    combo_rgb->addItem(tr("BRG"));
    combo_rgb->addItem(tr("GRB"));
    combo_rgb->setMinimumWidth(70);
    combo_rgb->setMaximumHeight(24);
    combo_rgb->setToolTip(tr("Select color channel order"));
    
    use_settings = new QCheckBox(tr("Use Settings"), this);
    use_settings->setCheckState(Qt::Checked);
    use_settings->setToolTip(tr("Apply color adjustment settings below"));
    
    optionsRow->addWidget(chk_negate);
    optionsRow->addSpacing(10);
    optionsRow->addWidget(channelLabel);
    optionsRow->addWidget(combo_rgb);
    optionsRow->addStretch(1);
    optionsRow->addWidget(use_settings);
    customListLayout->addLayout(optionsRow);
    
    connect(btn_add, SIGNAL(clicked()), this, SLOT(addClicked()));
    connect(btn_remove, SIGNAL(clicked()), this, SLOT(rmvClicked()));
    connect(btn_moveup, SIGNAL(clicked()), this, SLOT(upClicked()));
    connect(btn_movedown, SIGNAL(clicked()), this, SLOT(downClicked()));
    connect(btn_load, SIGNAL(clicked()), this, SLOT(load_CustomFile()));
    connect(btn_save, SIGNAL(clicked()), this, SLOT(save_CustomFile()));
    connect(chk_negate, SIGNAL(clicked()), this, SLOT(chk_Clicked()));
    connect(combo_rgb, SIGNAL(currentIndexChanged(int)), this, SLOT(cb_SetIndex(int)));

    // ===== Color Adjustments Section =====
    QGroupBox *colorGroup = new QGroupBox(tr("Color Adjustments"), this);
    QGridLayout *colorLayout = new QGridLayout(colorGroup);
    colorLayout->setSpacing(12);
    colorLayout->setContentsMargins(14, 18, 14, 14);
    colorLayout->setVerticalSpacing(14);
    
    // RGB sliders row 1
    QLabel *r_label = new QLabel(tr("Red:"), this);
    r_label->setStyleSheet("font-weight: bold;");
    slide_r = new QSlider(Qt::Horizontal, this);
    slide_r->setRange(0, 255);
    slide_r->setMinimumWidth(120);
    slide_r->setMaximumHeight(22);
    slide_r->setToolTip(tr("Adjust red channel intensity (0-255)"));
    
    QLabel *g_label = new QLabel(tr("Green:"), this);
    g_label->setStyleSheet("font-weight: bold;");
    slide_g = new QSlider(Qt::Horizontal, this);
    slide_g->setRange(0, 255);
    slide_g->setMinimumWidth(120);
    slide_g->setMaximumHeight(22);
    slide_g->setToolTip(tr("Adjust green channel intensity (0-255)"));
    
    QLabel *b_label = new QLabel(tr("Blue:"), this);
    b_label->setStyleSheet("font-weight: bold;");
    slide_b = new QSlider(Qt::Horizontal, this);
    slide_b->setRange(0, 255);
    slide_b->setMinimumWidth(120);
    slide_b->setMaximumHeight(22);
    slide_b->setToolTip(tr("Adjust blue channel intensity (0-255)"));
    
    colorLayout->addWidget(r_label, 0, 0, Qt::AlignRight);
    colorLayout->addWidget(slide_r, 0, 1);
    colorLayout->addWidget(g_label, 0, 2, Qt::AlignRight);
    colorLayout->addWidget(slide_g, 0, 3);
    colorLayout->addWidget(b_label, 0, 4, Qt::AlignRight);
    colorLayout->addWidget(slide_b, 0, 5);
    
    // Brightness, Gamma, Saturation row 2
    QLabel *bright_label = new QLabel(tr("Brightness:"), this);
    bright_label->setStyleSheet("font-weight: bold;");
    slide_bright = new QSlider(Qt::Horizontal, this);
    slide_bright->setRange(0, 255);
    slide_bright->setMinimumWidth(120);
    slide_bright->setMaximumHeight(22);
    slide_bright->setToolTip(tr("Adjust overall brightness (0-255)"));
    
    QLabel *gamma_label = new QLabel(tr("Gamma:"), this);
    gamma_label->setStyleSheet("font-weight: bold;");
    slide_gamma = new QSlider(Qt::Horizontal, this);
    slide_gamma->setRange(0, 255);
    slide_gamma->setMinimumWidth(120);
    slide_gamma->setMaximumHeight(22);
    slide_gamma->setToolTip(tr("Adjust gamma correction (0-255)"));
    
    QLabel *sat_label = new QLabel(tr("Saturation:"), this);
    sat_label->setStyleSheet("font-weight: bold;");
    slide_saturation = new QSlider(Qt::Horizontal, this);
    slide_saturation->setRange(0, 255);
    slide_saturation->setMinimumWidth(120);
    slide_saturation->setMaximumHeight(22);
    slide_saturation->setToolTip(tr("Adjust color saturation (0-255)"));
    
    colorLayout->addWidget(bright_label, 1, 0, Qt::AlignRight);
    colorLayout->addWidget(slide_bright, 1, 1);
    colorLayout->addWidget(gamma_label, 1, 2, Qt::AlignRight);
    colorLayout->addWidget(slide_gamma, 1, 3);
    colorLayout->addWidget(sat_label, 1, 4, Qt::AlignRight);
    colorLayout->addWidget(slide_saturation, 1, 5);
    
    // Color map row 3
    QLabel *colormap_label = new QLabel(tr("Color Map:"), this);
    colormap_label->setStyleSheet("font-weight: bold;");
    color_maps = new QComboBox(this);
    color_maps->addItem(tr("None"));
    color_maps->addItem(tr("Autumn"));
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
    color_maps->setMinimumWidth(140);
    color_maps->setMaximumHeight(22);
    color_maps->setToolTip(tr("Apply a color mapping to the output"));
    
    colorLayout->addWidget(colormap_label, 2, 0, Qt::AlignRight);
    colorLayout->addWidget(color_maps, 2, 1, 1, 2);
    colorLayout->setColumnStretch(5, 1);
    
    connect(slide_r, SIGNAL(valueChanged(int)), this, SLOT(slideChanged(int)));
    connect(slide_g, SIGNAL(valueChanged(int)), this, SLOT(slideChanged(int)));
    connect(slide_b, SIGNAL(valueChanged(int)), this, SLOT(slideChanged(int)));
    connect(slide_bright, SIGNAL(valueChanged(int)), this, SLOT(colorChanged(int)));
    connect(slide_gamma, SIGNAL(valueChanged(int)), this, SLOT(colorChanged(int)));
    connect(slide_saturation, SIGNAL(valueChanged(int)), this, SLOT(colorChanged(int)));
    connect(color_maps, SIGNAL(currentIndexChanged(int)), this, SLOT(colorMapChanged(int)));

    // ===== Log Output Section =====
    QGroupBox *logGroup = new QGroupBox(tr("Log Output"), this);
    QVBoxLayout *logLayout = new QVBoxLayout(logGroup);
    logLayout->setSpacing(8);
    
    log_text = new QTextEdit(this);
    log_text->setReadOnly(true);
    log_text->setMinimumHeight(100);
    log_text->setToolTip(tr("Application log and status messages"));
    
    QString text;
    QTextStream stream(&text);
    stream << tr("Acid Cam Filters v");
    stream << ac::version.c_str();
    stream << " loaded " << ac::getFilterCount() << " filters.\n";
    std::ostringstream s_stream;
    if(!context.create(cv::ocl::Device::TYPE_ALL))
        s_stream << "Could not create OpenCL Context\n";
    else {
        for(unsigned int i = 0; i < context.ndevices(); ++i) {
            cv::ocl::Device device = context.device(i);
            s_stream << "Name: " << device.name() << "\n"  << "OpenCL version: " << device.OpenCL_C_Version() << "\n";
        }
        cv::ocl::Device(context.device(0));
    }
    stream << s_stream.str().c_str();
    log_text->setText(text);
    
    logLayout->addWidget(log_text);

    progress_bar = new QProgressBar(this);
    progress_bar->setMinimum(0);
    progress_bar->setMaximum(100);
    progress_bar->setMinimumHeight(20);
    progress_bar->setMaximumHeight(24);
    progress_bar->hide();
    progress_bar->setToolTip(tr("Recording progress"));

    // ===== Final Layout Assembly =====
    QHBoxLayout *topRowLayout = new QHBoxLayout();
    topRowLayout->setSpacing(12);
    topRowLayout->addWidget(filterModeGroup);
    topRowLayout->addWidget(filterSelectGroup, 1);
    
    mainLayout->addLayout(topRowLayout);
    mainLayout->addWidget(customListGroup);
    mainLayout->addWidget(colorGroup);
    mainLayout->addWidget(progress_bar);
    mainLayout->addWidget(logGroup, 1);

    setWindowIcon(QPixmap(":/images/icon.png"));
    menu_cat->setCurrentIndex(1);
    Log(tr("Max frames set to 300; set accordingly based on desired RAM.\nIntertwine Rows and other filters that require more than 300 frames will not work until max is set.\n"));
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
    
    show_options_window = new QAction(tr("Show Preferences"), this);
    options->addAction(show_options_window);
    
    connect(show_options_window, SIGNAL(triggered()), this, SLOT(showPrefWindow()));
    
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
    
    cycle_custom = new QAction(tr("Cycle Custom"), this);
    cycle_custom->setCheckable(true);
    cycle_custom->setChecked(false);
    
    fade_on = new QAction(tr("Cross Fade"), this);
    fade_on->setCheckable(true);
    fade_on->setChecked(false);
    options->addAction(fade_on);
    options->addAction(repeat_v);
    options->addAction(cycle_custom);
    connect(cycle_custom, SIGNAL(triggered()), this, SLOT(setCustomCycle_Menu()));
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
    //controls_setkey = new QAction(tr("Set Color Key Image"), this);
    //controls_setkey->setShortcut(tr("Ctrl+K"));
    //controls_menu->addAction(controls_setkey);
    controls_showvideo = new QAction(tr("Hide Display Video"), this);
    controls_showvideo->setShortcut(tr("Ctrl+V"));
    controls_menu->addAction(controls_showvideo);
    
    //show_glDisplay = new QAction(tr("Show OpenGL Display"), this);
    //show_glDisplay->setShortcut(tr("Ctrl+G"));
    //connect(show_glDisplay, SIGNAL(triggered()), this, SLOT(showGLDisplay()));
    //controls_menu->addAction(show_glDisplay);
    show_fullscreen = new QAction(tr("Enter Full Screen"), this);
    show_image_window = new QAction(tr("Show Image/Video Manager"), this);
    controls_menu->addAction(show_fullscreen);
    connect(show_fullscreen, SIGNAL(triggered()), this, SLOT(showFull()));
    controls_menu->addAction(show_image_window);
    connect(show_image_window, SIGNAL(triggered()), this, SLOT(showImageWindow()));
    show_control_window = new QAction(tr("Show View Window"), this);
    connect(show_control_window, SIGNAL(triggered()), this, SLOT(controls_ShowDisp2()));
    controls_menu->addAction(show_control_window);

    show_range = new QAction(tr("Show Range"), this);
    show_range->setShortcut(tr("Ctrl+1"));
    controls_menu->addAction(show_range);
    
    show_slit = new QAction(tr("Show SlitScan"), this);
    show_slit->setShortcut(tr("Ctrl+2"));
    controls_menu->addAction(show_slit);

    connect(show_range, SIGNAL(triggered()), this, SLOT(showRange()));

    connect(show_slit, SIGNAL(triggered()), this, SLOT(showSlit()));

    
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
  //  connect(controls_setkey, SIGNAL(triggered()), this, SLOT(controls_SetKey()));
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
    
    set_newnames = new QAction(tr("Set Favorites"), this);
    connect(set_newnames, SIGNAL(triggered()), this, SLOT(show_Favorites()));
    controls_menu->addAction(set_newnames);
    
    select_random_filter = new QAction(tr("Set Random Filter"), this);
    connect(select_random_filter, SIGNAL(triggered()), this, SLOT(setRandomFilterValue()));
    select_random_filter->setShortcut(tr("Space"));
    
    controls_menu->addAction(select_random_filter);
    select_next_filter = new QAction(tr("Next Filter"), this);
    
    connect(select_next_filter, SIGNAL(triggered()), this, SLOT(next_filter()));
    
    select_next_filter->setShortcut(tr("Ctrl+K"));
    controls_menu->addAction(select_next_filter);
    
    select_prev_filter = new QAction(tr("Prev Filter"), this);
    
    connect(select_prev_filter, SIGNAL(triggered()), this, SLOT(prev_filter()));
    
    select_prev_filter->setShortcut(tr("Ctrl+L"));
    controls_menu->addAction(select_prev_filter);
    
}

void AC_MainWindow::showFull() {
    disp->showMax();
}

void AC_MainWindow::showImageWindow() {
    image_window->show();
}

void AC_MainWindow::resetIndex() {
    frame_index = 0;
}

void AC_MainWindow::showSlit() {
    slit_win->show();
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
        playback->setVector(v);
        Log(tr("Cleared SubFilter"));
    }
}

void AC_MainWindow::clear_img() {
    playback->clearImage();
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

void AC_MainWindow::showPrefWindow() {
    pref_window->show();
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
    speed_index = 0;
    playback->setAlpha(speed_actions[0]);
    QString text;
    QTextStream stream(&text);
    stream << "Movements Speed Set to: " <<
    speed_actions[0] << "\n";
    Log(text);
    for(int i = 0; i < 7; ++i) {
        speed_action_items[i]->setChecked(false);
    }
    speed_action_items[0]->setChecked(true);
    
}
void AC_MainWindow::speed2() {
    speed_index = 1;
    QString text;
    QTextStream stream(&text);
    playback->setAlpha(speed_actions[1]);
    stream << "Movements Speed Set to: " << speed_actions[1] << "\n";
    Log(text);
    for(int i = 0; i < 7; ++i) {
        speed_action_items[i]->setChecked(false);
    }
    speed_action_items[1]->setChecked(true);
}
void AC_MainWindow::speed3() {
    speed_index = 2;
    QString text;
    QTextStream stream(&text);
    playback->setAlpha(speed_actions[2]);
    stream << "Movements Speed Set to: " << speed_actions[2] << "\n";
    Log(text);
    for(int i = 0; i < 7; ++i) {
        speed_action_items[i]->setChecked(false);
    }
    speed_action_items[2]->setChecked(true);
    
}
void AC_MainWindow::speed4() {
    speed_index = 3;
    QString text;
    QTextStream stream(&text);
    playback->setAlpha(speed_actions[3]);
    stream << "Movements Speed Set to: " << speed_actions[3] << "\n";
    
    Log(text);
    for(int i = 0; i < 7; ++i) {
        speed_action_items[i]->setChecked(false);
    }
    speed_action_items[3]->setChecked(true);
}
void AC_MainWindow::speed5() {
    speed_index = 4;
    QString text;
    QTextStream stream(&text);
    playback->setAlpha(speed_actions[4]);
    stream << "Movements Speed Set to: " << speed_actions[4] << "\n";
    
    Log(text);
    for(int i = 0; i < 7; ++i) {
        speed_action_items[i]->setChecked(false);
    }
    speed_action_items[4]->setChecked(true);
}
void AC_MainWindow::speed6() {
    speed_index = 5;
    QString text;
    QTextStream stream(&text);
    playback->setAlpha(speed_actions[5]);
    stream << "Movements Speed Set to: " << speed_actions[5] << "\n";
    Log(text);
    for(int i = 0; i < 7; ++i) {
        speed_action_items[i]->setChecked(false);
    }
    speed_action_items[5]->setChecked(true);
}

void AC_MainWindow::speed7() {
    speed_index = 6;
    QString text;
    QTextStream stream(&text);
    playback->setAlpha(speed_actions[6]);
    stream << "Movements Speed Set to: " << speed_actions[6] << "\n";
        Log(text);
    for(int i = 0; i < 7; ++i) {
        speed_action_items[i]->setChecked(false);
    }
    speed_action_items[6]->setChecked(true);
}

void AC_MainWindow::movementOption1() {
    playback->setProcMode(0);
    in_out_increase->setChecked(true);
    in_out->setChecked(false);
    out_reset->setChecked(false);
    Log(tr("Proc Mode set to: 0\n"));
}
void AC_MainWindow::movementOption2() {
    in_out_increase->setChecked(false);
    in_out->setChecked(true);
    out_reset->setChecked(false);
    playback->setProcMode(1);
    Log(tr("Proc Mode set to: 1\n"));
}

void AC_MainWindow::movementOption3() {
    in_out_increase->setChecked(false);
    in_out->setChecked(false);
    out_reset->setChecked(true);
    playback->setProcMode(2);
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
    if(loading == true) return;
    playback->setIndexChanged(filters->currentText().toStdString());
    QString str;
    QTextStream stream(&str);
    stream << "Filter changed to: " << filters->currentText() << "\n";
    Log(str);
    std::string text = filters->currentText().toStdString();
    
    if(playback->videoIsOpen() == false && text.find("Video") != std::string::npos) {
        Log(tr("Set a video file to use this filter"));
    }
    
    if(blend_set == false && text.find("Image") != std::string::npos)
        Log(tr("Set an Image to use this filter\n"));
    
    if(playback->getProgramMode() && text.find("SubFilter") != std::string::npos)
        Log(tr("Set a SubFilter to use this filter\n"));
    
    if(playback->getMaxAlloc() < 1080 && text.find("Intertwine") != std::string::npos)
        Log(tr("Set Max Frames to greater than 1080 (requires enough RAM) to use Intertwine filters\n"));
    if(playback->getMaxAlloc() < 1080 && text.find("InOrder") != std::string::npos)
        Log(tr("Set Max Frames to greater than 1080 (requires enough RAM) to use inOrder filters\n"));
    if(playback->getMaxAlloc() < 1080 && text.find("Slit") != std::string::npos)
        Log(tr("Set Max Frames to greater than 1080 (requires enough RAM) to use SlitScan filters\n"));
}

void AC_MainWindow::setFilterSingle() {
    comboFilterChanged(0);
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
    log_text->ensureCursorVisible();
}

bool AC_MainWindow::startCamera(int res, int dev, const QString &outdir, bool record, int type,
                                bool useFFmpeg, FFmpegCodec ffmpegCodec, int crf) {
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

    video_frames = 0;
    video_fps = 24;
    
    int res_w = 0;
    int res_h = 0;
    
    output_directory = outdir;
    frame_index = 0;
    paused = false;
    recording = record;
    QString output_name;
    QTextStream stream_(&output_name);
    static unsigned int index = 0;
    time_t t = time(0);
    struct tm *m;
    m = localtime(&t);
    QString ext;
    
    Log(tr("Capture Device Opened [Camera]\n"));
    std::ostringstream time_stream;
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
    
    QString out_type;
    if(useFFmpeg) {
        ext = ".mp4";
        out_type = QString("FFmpeg.%1").arg(getCodecName(ffmpegCodec));
    } else {
        switch(type) {
            case 0:
                ext = ".mp4";
                out_type = "MPEG-4";
                break;
            case 1:
                ext = ".mp4";
                out_type = "AVC";
                break;
            case 2:
                ext = ".avi";
                out_type = "XviD";
                break;
        }
    }
    
    std::ostringstream index_val;
    index_val << std::setw(4) << std::setfill('0') << (++index);
    time_stream << "-" << (m->tm_year + 1900) << "." << (m->tm_mon + 1) << "." << m->tm_mday << "_" << m->tm_hour << "." << m->tm_min << "." << m->tm_sec <<  "_";
    stream_ << outdir << "/" << "Acid.Cam.Video" << "." << out_type << time_stream.str().c_str() << "." << "AC2.Output." << index_val.str().c_str() << ext;

    bool rt_val = false;
    
    if(recording && useFFmpeg) {
        video_file_name = output_name;
        QString out_s;
        QTextStream out_stream(&out_s);
        out_stream << "Now recording with FFmpeg (" << getCodecDescription(ffmpegCodec) << ") to: " << output_name 
                   << "\nResolution: " << res_w << "x" << res_h << " CRF: " << crf << "\n";
        Log(out_s);
        
        rt_val = playback->setVideoCameraFFmpeg(output_name.toStdString(), dev, res, ffmpegCodec, crf);
    } else {
        int c_type = 0;
        if(recording) {
            video_file_name = output_name;
            switch(type) {
                case 0:
                    c_type = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
                    break;
                case 1:
                    c_type = cv::VideoWriter::fourcc('a', 'v', 'c', '1');
                    break;
                case 2:
                    c_type = cv::VideoWriter::fourcc('X', 'v', 'i', 'D');
                    break;
            }
          
            QString out_s;
            QTextStream out_stream(&out_s);
            out_stream << "Now recording to: " << output_name << "\nResolution: " << res_w << "x" << res_h << " FPS: " << video_fps << "\n";
            Log(out_s);
        } else {
            output_name = "";
        }
        rt_val = playback->setVideoCamera(output_name.toStdString(), c_type, dev, res, writer, recording);
    }
    
    // if successful
    file_new_capture->setEnabled(false);
    file_new_video->setEnabled(false);
    controls_stop->setEnabled(true);
    
    if(rt_val == false) {
        QMessageBox::information(this, tr("Error Could not create output file"), tr("Output file"));
        return false;
    }
    
    playback->Play();
    disp->show();
    QString out_text;
#ifndef DISABLE_JOYSTICK
    QTextStream stream(&out_text);
    if(controller.open(0)) {
        stream << "Controller: " << controller.getControllerName() << " connected...\n";
        joy_timer->start();
    } else {
        stream << "No controller detected...\n";
    }
    Log(out_text);
#endif
    return true;
}

bool AC_MainWindow::startVideo(const QString &filename, const QString &outdir, bool record, bool png_record, int type,
                               bool useFFmpeg, FFmpegCodec ffmpegCodec, int crf, bool muxAudio) {
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
    video_frames = capture_video.get(cv::CAP_PROP_FRAME_COUNT);
    if(video_frames <= 0) return false;
    video_fps = capture_video.get(cv::CAP_PROP_FPS);
    int res_w = capture_video.get(cv::CAP_PROP_FRAME_WIDTH);
    int res_h = capture_video.get(cv::CAP_PROP_FRAME_HEIGHT);
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
    QString ext = ".mp4";
    std::ostringstream time_stream;
    QString out_type;
    
    if(useFFmpeg) {
        ext = ".mp4";
        out_type = QString("FFmpeg.%1").arg(getCodecName(ffmpegCodec));
    } else {
        switch(type) {
            case 0:
                ext = ".mp4";
                out_type = "MPEG-4";
                break;
            case 1:
                ext = ".mp4";
                out_type = "AVC";
                break;
            case 2:
                ext = ".avi";
                out_type = "XviD";
                break;
        }
    }
    
    std::ostringstream index_val;
    index_val << std::setw(4) << std::setfill('0') << (++index);
    time_stream << "-" << (m->tm_year + 1900) << "." << (m->tm_mon + 1) << "." << m->tm_mday << "_" << m->tm_hour << "." << m->tm_min << "." << m->tm_sec <<  "_";
    
    // For FFmpeg with audio muxing, use temp_ prefix
    QString prefix = (useFFmpeg && muxAudio) ? "temp_" : "";
    stream_ << outdir << "/" << prefix << "Acid.Cam.Video" << "." << out_type << time_stream.str().c_str() << "." << res_w << "x" << res_h << "." "AC2.Output." << index_val.str().c_str() << ext;

    if(recording && useFFmpeg) {
        video_file_name = output_name;
        QString out_s;
        QTextStream out_stream(&out_s);
        out_stream << "Now recording with FFmpeg (" << getCodecDescription(ffmpegCodec) << ") to: " << output_name 
                   << "\nResolution: " << res_w << "x" << res_h << " CRF: " << crf << "\n";
        if(muxAudio) {
            out_stream << "Audio will be copied from source video.\n";
        }
        Log(out_s);
        
        playback->setVideoFFmpeg(capture_video, output_name.toStdString(),
                                 ffmpegCodec, crf, video_fps, res_w, res_h,
                                 muxAudio, filename.toStdString());
    } else if(recording) {
        video_file_name = output_name;
        int c_type = 0;
        switch(type) {
            case 0:
                c_type = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
                break;
            case 1:
                c_type = cv::VideoWriter::fourcc('a', 'v', 'c', '1');
                break;
            case 2:
                c_type = cv::VideoWriter::fourcc('X', 'v', 'i', 'D');
                break;
        }

        writer = cv::VideoWriter(output_name.toStdString(), c_type, video_fps, cv::Size(res_w, res_h), true);
        
        if(!writer.isOpened()) {
            Log("Error could not open video writer.\n");
            QMessageBox::information(this, tr("Error invalid path"), tr("Incorrect Pathname or Codec not supported/Or you do not have permission to write to the directory."));
            return false;
        }
        QString out_s;
        QTextStream out_stream(&out_s);
        out_stream << "Now recording to: " << output_name << "\nResolution: " << res_w << "x" << res_h << " FPS: " << video_fps << "\n";
        Log(out_s);
        
        playback->setVideo(capture_video, writer, recording, png_record);
    } else {
        playback->setVideo(capture_video, writer, recording, png_record);
    }
    
    static int output_index = 1;
    QString dpath;
    QTextStream stream1(&dpath);
    QString path = filename.mid(filename.lastIndexOf("/"));
    stream1 << outdir << "/" << path << "_png." << output_index;
    
    if(png_record) {
        QDir dir(dpath);
        if(!dir.exists()) {
            dir.mkpath(dpath);
            std::cout << "mkpath: " << dpath.toStdString().c_str() << "\n";
        } else {
            std::cout << "directory exisits...\n";
        }
        ++output_index;
    }
    playback->setPngPath(dpath.toStdString());
    playback->Play();
    disp->show();
#ifndef DISABLE_JOYSTICK
    QString out_text;
    QTextStream streamx(&out_text);
    if(controller.open(0)) {
        streamx << "Controller: " << controller.getControllerName() << " connected...\n";
        joy_timer->start();
    } else {
        streamx << "No controller detected...\n";
    }
    Log(out_text);
#endif
    
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

void AC_MainWindow::controls_ShowDisp2() {
    QString st = show_control_window->text();
    if(st == "Hide Control Window") {
        disp2->hide();
        show_control_window->setText("Show View Window");
    } else {
        show_control_window->setText("Hide Control Window");
        disp2->show();
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
    QString dir_path = settings->value("dir_image", "").toString();
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open Image"), dir_path, tr("Image Files (*.png *.jpg)"));
    if(fileName != "") {
        cv::Mat tblend_image = cv::imread(fileName.toStdString());
        if(!tblend_image.empty()) {
            playback->setImage(tblend_image);
            QString text;
            QTextStream stream(&text);
            stream << "Successfully Loaded Image: [" << fileName << "] Size: " << tblend_image.cols << "x" << tblend_image.rows << "\n";
            Log(text);
            ac::pix.setInit(false);
            
            std::string val = fileName.toStdString();
            auto pos = val.rfind("/");
            if(pos == std::string::npos)
                pos = val.rfind("\\");
            if(pos != std::string::npos) {
                val = val.substr(0, pos);
            }
            
            settings->setValue("dir_image", val.c_str());
        } else {
            QMessageBox::information(this, tr("Image Load failed"), tr("Could not load image"));
        }
    }
}

void AC_MainWindow::controls_SetKey() {
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open Color Key Image"), "", tr("Image Files (*.png)"));
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
        if(n.find(":") == std::string::npos && n.find("SubFilter") == std::string::npos) {
            v.push_back(FilterValue(filter_map[name.toStdString()].index, filter_map[name.toStdString()].filter, -1));
        }
        else {
            
            
            std::string namev = name.toStdString();
            
            if(namev.find("SubFilter") != std::string::npos && namev.find(":") == std::string::npos)
                continue;
            
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
    cvtColor(tmp, result,cv::COLOR_BGR2RGB);
    return result;
}

QImage Mat2QImage(cv::Mat const& src)
{
    cv::Mat temp;
    cvtColor(src, temp,cv::COLOR_BGR2RGB);
    QImage dest((const uchar *) temp.data, static_cast<int>(temp.cols), static_cast<int>(temp.rows), static_cast<int>(temp.step), QImage::Format_RGB888);
    dest.bits();
    return dest;
}

void AC_MainWindow::setFrameIndex(int index) {
    frame_index = index;
}

std::mutex mut_lock;

void AC_MainWindow::updateFrame(QImage img) {
    mut_lock.lock();
    if(playback->isStopped() == false) {
        if(disp->isVisible())
            disp->displayImage(img);
        if(disp2->isVisible())
            disp2->displayImage(img);
        frame_index++;
        QString frame_string;
        QTextStream frame_stream(&frame_string);
        if(!recording) {
            frame_stream << "(Current/Total Frames/Seconds) - (" << frame_index << "/" << video_frames << "/" <<  (unsigned long)(frame_index/video_fps) << ") - ";
            
            unsigned long mem = playback->calcMem();
                frame_stream << "Memory Allocated: " << ((mem > 0) ? (mem/1024/1024) : 0) << " MB - " << " Initalized: " << playback->getObjectSize() << " - Allocated: " << playback->allocatedFrames() << "\n";
               
        } else {
            struct stat buf;
            stat(video_file_name.toStdString().c_str(), &buf);
            frame_stream << "(Current/Total Frames/Seconds/Size) - (" << frame_index << "/" << video_frames << "/" <<  (unsigned int)(frame_index/video_fps) << "/" << ((buf.st_size/1000)/1000) <<  " MB) - ";
            unsigned long mem = playback->calcMem();
                frame_stream << "Memory Allocated: " << ((mem > 0) ? (mem/1024/1024) : 0) << " MB - " << "Initalized: " << playback->getObjectSize() << " - Allocated: " << playback->allocatedFrames() << "\n";
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
    mut_lock.unlock();
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

void AC_MainWindow::onFFmpegFinished(QString tempFile, QString sourceFile, QString outputFile) {
    Log(tr("FFmpeg encoding finished. Muxing audio...\n"));
    bool success = ffmpeg_mux_audio(tempFile.toStdString(), sourceFile.toStdString(), outputFile.toStdString());
    if(success) {
        QFile::remove(tempFile);
        QString msg;
        QTextStream stream(&msg);
        stream << "Video with audio saved to: " << outputFile << "\n";
        Log(msg);
        QMessageBox::information(this, tr("Encoding Complete"), 
            tr("Video encoding complete with audio from source."));
    } else {
        QString msg;
        QTextStream stream(&msg);
        stream << "Audio muxing failed. Video saved to: " << tempFile << "\n";
        Log(msg);
        QMessageBox::warning(this, tr("Audio Muxing Failed"), 
            tr("Could not copy audio from source. Video file saved without audio."));
    }
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

std::mutex mutex_lock_;

void AC_MainWindow::frameInc() {
    frame_index++;
    QString frame_string;
    QTextStream frame_stream(&frame_string);
    
    if(!recording) {
        frame_stream << "(Current/Total Frames/Seconds) - (" << frame_index << "/" << video_frames << "/" << (unsigned int)(frame_index/video_fps) << ") ";
    } else {
        struct stat buf;
        stat(video_file_name.toStdString().c_str(), &buf);
        frame_stream << "(Current/Total Frames/Seconds/Size) - (" << frame_index << "/" << video_frames << "/" << (unsigned int)(frame_index/video_fps) << "/" << ((buf.st_size/1000)/1000)  <<  " MB) ";
        
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
    stream << tr("Engineering by <b>Jared Bruni</b><br>Testing by <b>Boris D. S</b><br><br><b>This software is dedicated to all the people that experience mental illness. </b><br><br><a href=\"https://lostsidedead.biz/wish\">My Wish List</a><br>\n");
    
    QMessageBox::information(this, tr("About Acid Cam"), about_str);
}

void AC_MainWindow::openSearch() {
    search_box->show();
}

void AC_MainWindow::openColorWindow() {
    chroma_window->show();
}

void AC_MainWindow::menuFilterChanged(int index) {
    if(index >= 0) {
        loading = true;
        std::string menu_n = menuNames[index];
        filters->clear();
        std::vector<std::string> &v = *ac::filter_menu_map[menu_n].menu_list;
        std::unordered_map<std::string, std::string> map_values;
        for(int i = 0; i < static_cast<int>(v.size()); ++i) {
            if(map_values.find(v[i]) == map_values.end()) {
                map_values[v[i]] = v[i];
                if(checkAdd(v[i].c_str()) == false)
                filters->addItem(v[i].c_str());
            }
        }
        filters->setCurrentIndex(0);
        loading = false;
    }
}

void AC_MainWindow::show_Favorites() {
    define_window->show();
}

void AC_MainWindow::resetMenu() {
    int index = menu_cat->currentIndex();
    if(index > 0 && menuNames[index] == std::string("User")) {
    	menuFilterChanged(index);
    	filters->setCurrentIndex(0);
    	comboFilterChanged(index);
    }
}

void AC_MainWindow::setOptionString(std::string op, std::string value) {
    int num = atoi(value.c_str());
    if(op == "=red") {
        slide_r->setSliderPosition(num);
        return;
    }
    if(op == "=green") {
        slide_g->setSliderPosition(num);
        return;
    }
    if(op == "=blue") {
        slide_b->setSliderPosition(num);
        return;
    }
    if(op == "=color_map") {
        color_maps->setCurrentIndex(num);
        return;
    }
    if(op == "=brightness") {
        slide_bright->setSliderPosition(num);
        return;
    }
    if(op == "=gamma") {
        slide_gamma->setSliderPosition(num);
        return;
    }
    if(op == "=sat") {
        slide_saturation->setSliderPosition(num);
        return;
    }
    if(op == "=negate") {
        chk_negate->setChecked(num);
        return;
    }
    if(op == "=color_order") {
        combo_rgb->setCurrentIndex(num);
        return;
    }
    if(op == "=proc") {
        setProcMode(num);
        return;
    }
    if(op == "=mvmnt") {
        setSpeedIndex(num);
        return;
    }
}

void AC_MainWindow::setProcMode(int index) {
    switch(index) {
        case 0:
            movementOption1();
            break;
        case 1:
            movementOption2();
            break;
        case 2:
            movementOption3();
            break;
    }
}

void AC_MainWindow::load_CustomFile() {
    QString file_name = QFileDialog::getOpenFileName(this, tr("Open File"),"",tr("Filter Files (*.filter)"));
    if(file_name.length()==0)
        return;
    
    std::fstream file;
    file.open(file_name.toStdString(), std::ios::in);
    if(!file.is_open()) {
        QMessageBox::information(this,"Could not open file", "Could not open file do i have rights to this folder?");
        return;
    }
    std::vector<std::string> values;
    while(!file.eof()) {
        std::string item;
        std::getline(file, item);
        if(file)
            values.push_back(item);
    }
    // check if data valid
    for(unsigned int i = 0; i < values.size(); ++i ){
        std::string item = values[i];
        std::string s_left, s_right;
        auto pos = item.find(":");
        if(pos == std::string::npos) {
            QMessageBox::information(this,"Incorrect File..\n", "Values in file incorrect");
            return;
        }
        s_left = item.substr(0,pos);
        s_right = item.substr(pos+1, item.length());

        if(item[0] == '=')
            continue;
        
        if(filter_map.find(s_left) == filter_map.end()) {
            QString itext = "Filter: ";
            itext += s_left.c_str();
            itext += " Not found in this version... old version?";
            QMessageBox::information(this, "Filter Not Found", itext);
            return;
        }
        if(s_right != "None" && filter_map.find(s_right) == filter_map.end()) {
            QString itext = "Filter: ";
            itext += s_right.c_str();
            itext += " Not found in this version... old version?";
            QMessageBox::information(this, "Filter Not Found", itext);
            return;
        }
        
        int val1 = filter_map[s_left].filter;
        int val2 = filter_map[s_right].filter;
        if(!(val1 >= 0 && val1 < ac::draw_max-4)) {
            QMessageBox::information(this,"Unsupported Value","Filter value out of range... wrong program revision?");
            return;
        }
        if(!(val2 == -1 || (val2 >= 0 && val2 < ac::draw_max-4))) {
            QMessageBox::information(this, "Unsupported SubFilter value","Sub Filter value of range...");
            return;
        }
    }
    while(custom_filters->count() > 0) {
        custom_filters->takeItem(0);
    }
    for(unsigned int i = 0; i < values.size(); ++i) {
        std::string item = values[i];
        std::string s_left, s_right;
        s_left = item.substr(0, item.find(":"));
        s_right = item.substr(item.find(":")+1, item.length());
        if(s_left[0] == '=' && use_settings->isChecked() == true) {
            setOptionString(s_left, s_right);
            continue;
        } else if(s_left[0] == '=') {
            continue;
        }
        int value1 = filter_map[s_left].filter;
        int value2 = 0;
        if(s_right == "None")
            value2 = -1;
        else
        	value2 = filter_map[s_right].filter;
        //int value1 = atoi(s_left.c_str());
        //int value2 = atoi(s_right.c_str());
        std::ostringstream stream;
        stream << draw_strings[value1];
        if(value2 != -1)
            stream << ":" << draw_strings[value2];
        custom_filters->addItem(stream.str().c_str());
    }
    slideChanged(0);
    colorChanged(0);
    colorMapChanged(color_maps->currentIndex());
    std::ostringstream sval;
    sval << "Loaded Custom Filter: " << file_name.toStdString() << "\n";
    std::vector<FilterValue> v;
    buildVector(v);
    playback->setVector(v);
    file.close();
    Log(sval.str().c_str());
}

// ugly way of doing this
void AC_MainWindow::setSpeedIndex(int index) {
    switch(index) {
        case 0:
            speed1();
            break;
        case 1:
            speed2();
            break;
        case 2:
            speed3();
            break;
        case 3:
            speed4();
            break;
        case 4:
            speed5();
            break;
        case 5:
            speed6();
            break;
        case 6:
            speed7();
            break;
            
    }
}

namespace ac {
	extern int proc_mode;
}

void AC_MainWindow::save_CustomFile() {
    if(custom_filters->count() == 0) {
        QMessageBox::information(this, "Seleect Filters", "You need to adds ome filters to be able to save...");
        return;
    }
    QString file_name = QFileDialog::getSaveFileName(this, tr("Save File"),"",                                            tr("Filter Save (*.filter)"));
    
    if(file_name.length()==0)
        return;
    
    std::fstream file_n;
    file_n.open(file_name.toStdString(),std::ios::out);
    if(!file_n.is_open()) {
        QMessageBox::information(this, "Error", "File could not be opened...");
        return;
    }
    std::vector<FilterValue> v;
    buildVector(v);
    if(v.size()==0) {
        QMessageBox::information(this, "No Filters", "Please Add Filters to Save");
        return;
    }
    if(use_settings->isChecked() == 1) {
        int rgb[] = { slide_r->sliderPosition(), slide_g->sliderPosition(), slide_b->sliderPosition()};
        file_n << "=red:" << rgb[0] << "\n";
        file_n << "=green:" << rgb[1] << "\n";
        file_n << "=blue:" << rgb[2] << "\n";
        int color_m = color_maps->currentIndex();
        file_n << "=color_map:" << color_m << "\n";
        int bright = slide_bright->sliderPosition();
        file_n << "=brightness:" << bright << "\n";
        int gam = slide_gamma->sliderPosition();
        file_n << "=gamma:" << gam << "\n";
        int sat = slide_saturation->sliderPosition();
        file_n << "=sat:" << sat << "\n";
        int chkNegate = chk_negate->isChecked();
        file_n << "=negate:" << chkNegate << "\n";
        int cord = combo_rgb->currentIndex();
        file_n << "=color_order:" << cord << "\n";
        int procM = static_cast<int>(ac::getProcMode());
        file_n << "=proc:" << procM << "\n";
        int mvmnt = speed_index;
        file_n << "=mvmnt:"<< mvmnt << "\n";
    }
    for(unsigned int i = 0; i < v.size(); ++i) {
        if(v[i].index == 0) {
        	int value1 = v[i].filter;
        	int value2 = v[i].subfilter;
            std::string v1, v2;
            v1 = draw_strings[value1];
            if(value2 == -1) {
                v2 = "None";
            }
            else {
                v2 = draw_strings[value2];
            }
        	file_n << v1 << ":" << v2 << "\n";
        }
    }
    std::ostringstream stream;
    stream << "Wrote custom to: " << file_name.toStdString() << "\n";
    Log(stream.str().c_str());
    file_n.close();
}

void AC_MainWindow::setRandomFilterValue() {
    menu_cat->setCurrentIndex(0);
    resetMenu();
    int index = rand()%solo_filter.size();
    std::string filter_name = solo_filter[index];
    int filter_index = filter_map_main[filter_name];
    filters->setCurrentIndex(filter_index);
}

void AC_MainWindow::setCustomCycle_Menu() {
    bool chk = cycle_custom->isChecked();
    playback->setCustomCycle(chk);
}

void AC_MainWindow::next_filter() {
    int count = filters->count();
    int index = filters->currentIndex();
    if(index < count) {
        ++index;
        filters->setCurrentIndex(index);
    }
}

void AC_MainWindow::prev_filter() {
    int index = filters->currentIndex();
    if(index > 0) {
        --index;
        filters->setCurrentIndex(index);
    }
}

void AC_MainWindow::showGLDisplay() {
    
}

void AC_MainWindow::chk_Joystick() {
#ifndef DISABLE_JOYSTICK
    SDL_Event e;
    while(SDL_PollEvent(&e)) {
        static int speed = 0;
        if(controller.button(1)) {
            prev_filter();
        }
        if(controller.button(0)) {
            next_filter();
        }
        
        static int movement_index = 0;
        
        if(controller.button(7)) {
            if(movement_index < 2)
                movement_index++;
            
            switch(movement_index) {
                case 0:
                    movementOption1();
                    break;
                case 1:
                    movementOption2();
                    break;
                case 2:
                    movementOption3();
                    break;
            }
        }

        if(controller.button(6)) {
            if(movement_index > 0)
                --movement_index;
            
            switch(movement_index) {
                case 0:
                    movementOption1();
                    break;
                case 1:
                    movementOption2();
                    break;
                case 2:
                    movementOption3();
                    break;
            }
        }
        /*
        for(int i = 0; i < 8; ++i) {
            if(controller.button(i)) {
                std::cout << "button: " << i << "\n";
            }
        }*/
        
        auto change = [](AC_MainWindow *window, int index) {
            switch(index) {
                case 0:
                    window->speed1();
                    break;
                case 1:
                    window->speed2();
                    break;
                case 2:
                    window->speed3();
                    break;
                case 3:
                    window->speed4();
                    break;
                case 4:
                    window->speed5();
                    break;
                case 5:
                    window->speed6();
                    break;
                case 7:
                    window->speed7();
                    break;
            }
        };
        
        if(controller.button(3)) {
            if(speed < 6) {
                ++speed;
                change(this, speed);
            }
        }
        if(controller.button(4)) {
            if(speed > 0) {
                --speed;
                change(this, speed);
            }
        }
        
    }
#endif
}
