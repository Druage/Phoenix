
#include <QVector>
#include <QTimer>
#include <QObject>

#include "joystick.h"
#include "sdljoystick.h"

class InputManager : public QObject {
    Q_OBJECT

public:
    InputManager();
    ~InputManager();

    void scanDevices();

    void startTimer(int mili_secs=20);
    void stopTimer();

    void append(InputDevice *device);

    InputDevice * getDevice(unsigned port);

public slots:
    void getDeviceEvents();

signals:

private:
    QList<InputDevice *> joysticks;
    SDLJoystick sdl_joystick;
    QTimer timer;

    unsigned current_port;

};
