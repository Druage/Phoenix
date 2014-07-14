#ifndef INPUTSDLJoystick_H
#define INPUTSDLJoystick_H

#include <QObject>
#include <QTimer>
#include <QList>
#include <QListIterator>
#include "joystick.h"
#include "logging.h"
#include "libretro.h"

class SDLJoystick : public QObject
{
    Q_OBJECT

public:
    QList<Joystick *> joys;

    explicit SDLJoystick(QObject *parent = 0);
    ~SDLJoystick();
    void convertToRetroDevice(unsigned &id, int button);
    void setDeadZone(Uint16 &axis_value);

signals:
    void dataChanged(bool is_pressed, unsigned port, unsigned device, unsigned index, unsigned id);
    void dataChanged(unsigned port, unsigned device, unsigned index, unsigned id);

public slots:
    QList<Joystick *> onScan();
    void onProcessEvent(QList<Joystick *> &joysticks);

private:
    unsigned port;
    unsigned device;

};

#endif // INPUTSDLJoystick_H
