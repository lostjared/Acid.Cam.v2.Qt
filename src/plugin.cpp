#include "plugin.h"
#if defined(__linux__) || defined(__APPLE__)
#include<dirent.h>
#include<sys/stat.h>
#else

#endif

PluginList plugins;

#if defined(__linux__) || defined(__APPLE__)
void add_directory(std::string path, std::vector<std::string> &files) {
    DIR *dir = opendir(path.c_str());
    if(dir == NULL) {
        std::cerr << "Error could not open directory: " << path << "\n";
        return;
    }
    dirent *file_info;
    while( (file_info = readdir(dir)) != 0 ) {
        std::string f_info = file_info->d_name;
        if(f_info == "." || f_info == "..")  continue;
        std::string fullpath=path+"/"+f_info;
        struct stat s;
        
        lstat(fullpath.c_str(), &s);
        if(S_ISDIR(s.st_mode)) {
            if(f_info.length()>0 && f_info[0] != '.')
                add_directory(path+"/"+f_info, files);
            
            continue;
        }
        if(f_info.length()>0 && f_info[0] != '.' && fullpath.rfind(".so") != std::string::npos) {
            files.push_back(fullpath);
            std::cout << "found: " << fullpath << "\n";
        }
    }
    closedir(dir);
}
#else
void add_directory(std::string path, std::vector<std::string> &files) {
    
}
#endif

void init_plugins() {
    std::vector<std::string> files;
    add_directory("plugins", files);
    if(files.size()>0) {
        for(unsigned int i = 0; i < files.size(); ++i) {
            Plugin *p = new Plugin();
            if(p->loadPlugin(files[i]))
            plugins.plugin_list.push_back(p);
        }
    }
}


void ac::plugin(cv::Mat &frame) {
    for(int z = 0; z < frame.rows; ++z) {
        for(int i = 0; i < frame.cols; ++i) {
            unsigned char rgb[3];
            cv::Vec3b &cpixel = frame.at<cv::Vec3b>(z, i);
            rgb[0] = cpixel[0];
            rgb[1] = cpixel[1];
            rgb[2] = cpixel[2];
            //plugins.plugin_list[0]->call_Pixel(i, z, rgb);
            cpixel[0] = rgb[0];
            cpixel[1] = rgb[1];
            cpixel[2] = rgb[2];
        }
    }
   //plugins.plugin_list[0]->call_Complete();
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
        QMessageBox::information(0, "Could not find pixel function", text.c_str());
        return false;
    }
    
    complete_function = (complete) library->resolve("complete");
    if(!complete_function) {
        QMessageBox::information(0,  "Could not find complete function", text.c_str());
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


