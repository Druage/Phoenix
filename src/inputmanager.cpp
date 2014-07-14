#include "inputmanager.h"

InputManager::InputManager() {

    connect(&timer, SIGNAL(timeout()), this, SLOT(getDeviceEvents()));

}

InputManager::~InputManager() {

    stopTimer();

}

void append() {

    //sdl_joystick = SDLJoystick(this,)

}

Joystick * InputManager::getDevice(unsigned port) {

    current_port = port;

    return joysticks.at(port);

}

void InputManager::startTimer(int mili_secs) {

    timer.start(mili_secs);

}

void InputManager::stopTimer() {

    timer.stop();

}

// Sets how number of joysticks to iterate over to find presses
// Set to -1 to process every available joystick
void InputManager::getDeviceEvents() {

    sdl_joystick.onProcessEvent(joysticks);

}

void InputManager::scanDevices() {

    joysticks = sdl_joystick.onScan();

}
