#include "inputdevice.h"

InputDevice::InputDevice(unsigned id) {
    type = id;
    button_states.clear();
    count = 0;
    joystick = nullptr;

    switch(id) {
        case RETRO_DEVICE_ANALOG:
            name = "Analog";
            break;
        case RETRO_DEVICE_MOUSE:
            name = "Mouse";
            break;
        case RETRO_DEVICE_JOYPAD:
            name = "Joypad";
            break;
        case RETRO_DEVICE_KEYBOARD:
            name = "Keyboard";
            break;
        case RETRO_DEVICE_NONE:
            name = "None";
            break;
        default:
            qCDebug(phxInput) << "Device not handled";
            name = "None";
            break;
    }

}

InputDevice::~InputDevice() {

}
