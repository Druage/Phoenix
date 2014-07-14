#ifndef INPUTITEM_H
#define INPUTITEM_H

#include <QtQuick/QQuickItem>
#include <QMap>
#include <QVector>

#include "libretro.h"
#include "sdljoystick.h"

class InputManager : QObject {
    Q_OBJECT
public:

    explicit InputManager(QObject *parent=0);
    ~InputManager();
    Joystick getInputDevice(unsigned port, unsigned device);

    void createJoySticks(unsigned number_of_joysticks);

signals:
    changingDevices(bool);

private:

    QVector<Joystick> joysticks;

};


/*class InputItem : public QObject {

    Q_OBJECT

public:

    explicit InputItem(QObject *parent=0);
    ~InputItem();

public slots:

    void setRetroPad(Joystick joy_pad);
    Joystick getRetroPad();

signals:


protected:


private:
    SDLJoystick *sdl_joystick;
    Joystick retro_pad;

};*/

#endif // INPUTITEM_H
