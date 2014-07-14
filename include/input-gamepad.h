#ifndef INPUT_GAMEPAD_H
#define INPUT_GAMEPAD_H

#include <SFML/Window/Joystick.hpp>
#include <SFML/Window.hpp>
#include <QThread>
#include <QEvent>
#include <QTimer>
#include <QMap>
#include <QQueue>

#include "core.h"

class ButtonPress : public QEvent {
public:
    ButtonPress() :
        QEvent(ButtonPress::type()) {
    }

    static QEvent::Type type() {
        return static_cast<QEvent::Type>(2000);
    }

};

class ButtonRelease : public QEvent {
public:
    ButtonRelease() :
        QEvent(ButtonRelease::type()) {
    }

    static QEvent::Type type() {
        return static_cast<QEvent::Type>(2001);
    }

};

/*

class GamePadEventHandler : QObject {
    Q_OBJECT
public:
    explicit GamePadEventHandler(QObject *parent = 0);
    ~GamePadEventHandler();
    bool popEvent(QEvent &event, bool block);
    void pushEvent(QEvent &event);
    void processGamePadEvents();
    void start();

private:
    sf::Event top_event;
    sf::Joystick *gamepad;

    QQueue<QEvent::Type> events;
    QTimer timer;
}
*/

#endif // INPUT_GAMEPAD_H
