
#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include"qtheaders.h"

#ifndef DISABLE_JOYSTICK

#include "SDL.h"

class Controller {
public:
    static void init() {
        SDL_Init(SDL_INIT_JOYSTICK);
        atexit(SDL_Quit);
    }
    
    bool open(int index);
    void close();
    bool button(int index);
    Uint8 hat(int h);
    int axis(int index);
    const char *getControllerName();
    
private:
    SDL_Joystick *stick;

};

#endif


#endif

