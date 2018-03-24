
/*
 * Acid Cam v2 - Qt/OpenCV Edition
 * written by Jared Bruni ( http://lostsidedead.com )
 * (C) 2017 GPL
 */

#include "plugin.h"

PluginList plugins;

void add_directory(QDir &cdir, std::vector<std::string> &files) {
    cdir.setFilter(QDir::Files | QDir::Dirs);
    QFileInfoList list = cdir.entryInfoList();
    int pos = 0;
    QString platform;
#if defined(__linux__) 
    platform = ".so";
#elif defined(__APPLE__)
    platform = ".dylib";
#else
    platform = ".dll";
#endif
   	while(pos < list.size()) {
        QFileInfo info = list.at(pos);
        if(info.isDir() && info.fileName() != "." && info.fileName() != "..") {
            QDir cdir = info.dir();
            cdir.cd(info.fileName());
            add_directory(cdir, files);
            ++pos;
            continue;
        }
        else if(info.isFile() && info.fileName() != "." && info.fileName() != ".." && info.fileName().contains(platform)) {
            files.push_back(info.filePath().toStdString());
        }
        ++pos;
    }
}

void init_plugins() {
    std::vector<std::string> files;
    QDir d("plugins");
    add_directory(d, files);
    if(files.size()>0) {
        for(unsigned int i = 0; i < files.size(); ++i) {
            Plugin *p = new Plugin();
            if(p->loadPlugin(files[i]))
            plugins.plugin_list.push_back(p);
        }
    }
}

void draw_plugin(cv::Mat &frame, int filter) {
    for(int z = 0; z < frame.rows; ++z) {
        for(int i = 0; i < frame.cols; ++i) {
            unsigned char rgb[3];
            cv::Vec3b &cpixel = frame.at<cv::Vec3b>(z, i);
            rgb[0] = cpixel[0];
            rgb[1] = cpixel[1];
            rgb[2] = cpixel[2];
            plugins.plugin_list[filter]->call_Pixel(i, z, rgb);
            cpixel[0] = rgb[0];
            cpixel[1] = rgb[1];
            cpixel[2] = rgb[2];
        }
    }
    plugins.plugin_list[filter]->call_Complete();
}


void plugin_callback(cv::Mat &) {

}

Plugin::Plugin() {
    library = 0;
}

Plugin::~Plugin() {
    if(library) delete library;
}
    
bool Plugin::loadPlugin(const std::string &text) {
    library = new QLibrary(text.c_str());
    if(!library) {
        QMessageBox::information(0, QObject::tr("Could not load Library"), text.c_str());
        return false;
    }
    
    pixel_function = (pixel) library->resolve("pixel");
    if(!pixel_function) {
        QMessageBox::information(0, text.c_str(), "Could not find pixel function");
        return false;
    }
    
    complete_function = (complete) library->resolve("complete");
    if(!complete_function) {
        QMessageBox::information(0, text.c_str(), "Could not find complete function");
        return false;
    }
    mod_name = text;
    return true;
}


void Plugin::call_Pixel(int x, int y, unsigned char *rgb) {
    
    if(pixel_function)
        pixel_function(x, y, rgb);
    
}
void Plugin::call_Complete() {
    if(complete_function)
        complete_function();
}

PluginList::PluginList() {
    
}

PluginList::~PluginList() {
    if(plugin_list.size() == 0) return;

    for(auto i = plugin_list.begin(); i != plugin_list.end(); ++i)
        delete *i;
}


