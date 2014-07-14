#include "inputitem.h"

InputManager::InputManager() : QObject(parent) {
    //sdl_joystick = SDLJoystick(this);

    //connect(sdl_joystick, SIGNAL(dataChanged(Joystick)), this, SLOT(setRetroPad(Joystick));

}

InputManager::~InputManager() {
    //sdl_joystick->deleteLater();
}

void InputManager::getInputDevice(unsigned port, unsigned device) {
    emit changingDevices(true);
    switch(device) {
        case RETRO_DEVICE_JOYPAD:
            return joysticks.at(port);
        default:
            return joysticks.at(port);
    }
}

void InputManager::createJoySticks(unsigned number_of_joysticks) {
    for (unsigned i=0; i < number_of_joysticks; ++i) {
        Joystick joystick;
        joystick.name = "";
        joystick.description = "";
        joysticks.push_back(new Joystick());
    }
}

Joystick InputItem::getRetroPad() {

    return retro_pad;

}
