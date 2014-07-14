#include "sdljoystick.h"
#include <QDebug>
#include <QProcess>

#define JOYSTICK_DEADZONE_LOW 2000
#define JOYSTICK_DEADZONE_HIGH 50000

SDLJoystick::SDLJoystick(QObject *parent)
    : QObject(parent) {


    qCDebug(phxInput) << this << "SDLJoystick";
}

SDLJoystick::~SDLJoystick() {
    qCDebug(phxInput) << this << "~SDLJoystick";
    if (SDL_WasInit(SDL_INIT_JOYSTICK) != 0)
        SDL_Quit();
}

/* public slots */

void SDLJoystick::setDeadZone(Uint16 &axis_value) {

    if (axis_value > JOYSTICK_DEADZONE_HIGH || axis_value < JOYSTICK_DEADZONE_LOW)
        axis_value = 0;

}

QList<Joystick *> SDLJoystick::onScan() {

    qDeleteAll(joys.begin(), joys.end());
    joys.clear();
    qCDebug(phxInput) << "scanning";
    if (SDL_WasInit(SDL_INIT_JOYSTICK) != 0) {
        qCDebug(phxInput) << "SDL_Quit";
        SDL_Quit();
    }

    if (SDL_Init(SDL_INIT_JOYSTICK) != 0) {
        qCCritical(phxInput) << "SDL_INIT_JOYSTICK failed";
        //return;
    }

    int count = SDL_NumJoysticks();
    Joystick *j;
    for (int i = 0; i < count; ++i) {
        j = new Joystick();

        j->index = i;

        j->name = SDL_JoystickNameForIndex(i);

        j->joy = SDL_JoystickOpen(i);
        if (!j->joy) {
            qCCritical(phxInput) << "SDL_JoystickOpen failed, index:" << i;
            //return;
        }

        quint8 k;

        j->numAxes = SDL_JoystickNumAxes(j->joy);
        for (k = 0; k < j->numAxes; ++k)
            j->axes[k] = 0;

        j->numButtons = SDL_JoystickNumButtons(j->joy);
        for (k = 0; k < j->numButtons; ++k)
            j->buttons[k] = 0;

        j->numHats = SDL_JoystickNumHats(j->joy);
        for (k = 0; k < j->numHats; ++k)
            j->hats[k] = 0;

        joys.append(j);

        qDebug("idx: %d, name: %s (axes: %d, buttons: %d, hats: %d)",
               j->index,
               j->name.toUtf8().data(),
               j->numAxes,
               j->numButtons,
               j->numHats);
    }

    qCDebug(phxInput) << "joy count: " << count;
    return joys;

}

void SDLJoystick::convertToRetroDevice(unsigned &id, int button) {
    switch (button) {
        case 0:
            id = RETRO_DEVICE_ID_JOYPAD_UP;
            break;
        case 1:
            id = RETRO_DEVICE_ID_JOYPAD_DOWN;
            break;
        case 2:
            id = RETRO_DEVICE_ID_JOYPAD_LEFT;
            break;
        case 3:
            id = RETRO_DEVICE_ID_JOYPAD_RIGHT;
            break;
        case 4:
            id = RETRO_DEVICE_ID_JOYPAD_START;
            break;
        case 5:
            id = RETRO_DEVICE_ID_JOYPAD_SELECT;
            break;
        case 6:
            id = RETRO_DEVICE_ID_JOYPAD_L3;
            break;
        case 7:
            id = RETRO_DEVICE_ID_JOYPAD_R3;
            break;
        case 8:
            id = RETRO_DEVICE_ID_JOYPAD_L;
            break;
        case 9:
            id = RETRO_DEVICE_ID_JOYPAD_R;
            break;
        case 10:
            id = RETRO_DEVICE_ID_JOYPAD_B;
            break;
        case 11:
            id = RETRO_DEVICE_ID_JOYPAD_A;
            break;
        case 12:
            id = RETRO_DEVICE_ID_JOYPAD_Y;
            break;
        case 13:
            id = RETRO_DEVICE_ID_JOYPAD_X;
            break;
        case 14:
            // id = it's the home button on xbox controller
            break;

        default:
            qCDebug(phxInput) << "Input was not handled";
            break;
    }
}

void SDLJoystick::onProcessEvent(QList<Joystick *> &joysticks) {

    Joystick *joy = nullptr;
    unsigned id = 16;
    unsigned index = 0;
    unsigned device = RETRO_DEVICE_JOYPAD;

    for (int j = 0; j < joysticks.count(); ++j) {

        int i;
        joy = joysticks.at(j);

        SDL_JoystickUpdate();

        for (i = 0; i < joy->numAxes; ++i) {

            Uint16 new_axis_motion = SDL_JoystickGetAxis(joy->joy, i);
            joy->axes[i] = new_axis_motion;
            device = RETRO_DEVICE_ANALOG;

            switch(i) {
                case 0:
                    id = RETRO_DEVICE_ID_ANALOG_X;
                    index = RETRO_DEVICE_INDEX_ANALOG_LEFT;

                    setDeadZone(new_axis_motion);

                    break;
                case 1:
                    id = RETRO_DEVICE_ID_ANALOG_Y;
                    index = RETRO_DEVICE_INDEX_ANALOG_LEFT;

                    setDeadZone(new_axis_motion);

                    break;
                case 2:
                    id = RETRO_DEVICE_ID_ANALOG_X;
                    index = RETRO_DEVICE_INDEX_ANALOG_RIGHT;

                    setDeadZone(new_axis_motion);

                    break;
                case 3:
                    id = RETRO_DEVICE_ID_ANALOG_Y;
                    index = RETRO_DEVICE_INDEX_ANALOG_RIGHT;

                    setDeadZone(new_axis_motion);

                    break;
                case 4:
                    id = RETRO_DEVICE_ID_JOYPAD_L2;
                    device = RETRO_DEVICE_JOYPAD;
                    break;
                case 5:
                    id = RETRO_DEVICE_ID_JOYPAD_R2;
                    device = RETRO_DEVICE_JOYPAD;
                    break;
                default:
                    qCDebug(phxInput) << "not handled: " << i << "   ==>>  " << new_axis_motion;
                    break;
            }
        }


        for (i = 0; i < joy->numButtons; ++i) {
            Uint8 new_button_press = SDL_JoystickGetButton(joy->joy, i);
            convertToRetroDevice(id, i);
            joy->buttons[id] = new_button_press;
        }

        for (i = 0; i < joy->numHats; ++i) {
            joy->hats[i] = SDL_JoystickGetHat(joy->joy, i);
            qCDebug(phxInput) << joy->hats[i];
        }

    }

}
