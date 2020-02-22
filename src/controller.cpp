
#include"controller.h"
#include<iostream>


bool Controller::open(int index) {
    stick = SDL_JoystickOpen(index);
    if(!stick)
        return false;
    std::cout << "Opened: " << SDL_JoystickName(stick) << "\n";
    return true;
}

const char *Controller::getControllerName() {
    const char *text = SDL_JoystickName(stick);
    return text;
}

void Controller::close() {
    if(stick != 0)
        SDL_JoystickClose(stick);
}

bool Controller::button(int index) {
    if(stick != 0 && SDL_JoystickGetButton(stick, index))
        return true;
    return false;
}

Uint8 Controller::hat(int h) {
    if(stick != 0)
        return SDL_JoystickGetHat(stick, h);
    return 0;
}

int Controller::axis(int index) {
    if(stick != 0)
        return SDL_JoystickGetAxis(stick, index);
    return 0;
}
