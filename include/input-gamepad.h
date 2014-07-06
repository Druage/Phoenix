#ifndef INPUT_GAMEPAD_H
#define INPUT_GAMEPAD_H

#include <SFML/Window/Joystick.hpp>
#include <SFML/Window.hpp>
#include <QVector>
#include "core.h"

class GamePad {

public:
    GamePad();
    //~GamePad();

    bool connect();
    void setPort(unsigned int gamepad_port);
    void handleEvent(sf::Event event);
    void handlePress(unsigned int button, unsigned &id);
    void setCore( Core *retro_core );

signals:

public slots:
    void sync(); // same as sf::Joystick::update()

private:
    sf::Window *sfml_window;
    sf::Joystick current_retropad;
    Core *core;
    int retropad_count;
    unsigned int port;


};

#endif // INPUT_GAMEPAD_H
