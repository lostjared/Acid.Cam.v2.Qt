#ifndef _PLUGIN_H
#define _PLUGIN_H

#include "qtheaders.h"


typedef void (*pixel)(int x, int y, unsigned char *buf);
typedef void (*complete)();

class Plugin {
public:
    Plugin();
    ~Plugin();
    bool loadPlugin(const std::string &text);
    void call_Pixel(int x, int y, unsigned char *rgb);
    void call_Complete();
    std::string name() const { return mod_name; }
private:
    pixel pixel_function;
    complete complete_function;
    QLibrary *library;
    std::string mod_name;
};

class PluginList {
public:
    PluginList();
    ~PluginList();
    std::vector<Plugin*> plugin_list;
};

extern PluginList plugins;
#endif
