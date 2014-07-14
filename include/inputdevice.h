#include "libretro.h"
#include "logging.h"

#include <QMap>
#include <QString>
#include <SDL2/SDL.h>

class InputDevice {

public:
    InputDevice(unsigned id);

    ~InputDevice();

    QMap<unsigned, Sint16> button_states;

    unsigned type;

    int index;

    int count;

    SDL_Joystick *joystick;

    QString name;


};
