
#include "input-gamepad.h"

GamePad::GamePad() {

    // SFML needs a window to be made to take in events, sf::Style::None makes window not render,
    // setVisible(false) hides the invisible window from stealing focus
    sfml_window = new sf::Window(sf::VideoMode(0,0), "", sf::Style::None);
    sfml_window->setVisible(false);

    current_retropad = sf::Joystick();
    core = NULL;
    retropad_count = 0;
    port = 0;
}

//GamePad::~GamePad() {
    //if (sfml_window->isOpen())
        //sfml_window->close();
//}

bool GamePad::connect() {
    return current_retropad.isConnected(port);
}

void GamePad::handlePress(unsigned int button, unsigned &id) {
    switch (button) {
        case 0:
            id = RETRO_DEVICE_ID_JOYPAD_A;
            break;
        case 1:
            id = RETRO_DEVICE_ID_JOYPAD_B;
            break;
        case 2:
            id = RETRO_DEVICE_ID_JOYPAD_X;
            break;
        case 3:
            id = RETRO_DEVICE_ID_JOYPAD_Y;
            break;
        case 4:
            id = RETRO_DEVICE_ID_JOYPAD_START;
            break;
        case 5:
            id = RETRO_DEVICE_ID_JOYPAD_SELECT;
            break;
        case 6:
            id = RETRO_DEVICE_ID_JOYPAD_LEFT;
            break;
        case 7:
            id = RETRO_DEVICE_ID_JOYPAD_RIGHT;
            break;
        case 8:
            id = RETRO_DEVICE_ID_JOYPAD_UP;
            break;
        case 9:
            id = RETRO_DEVICE_ID_JOYPAD_DOWN;
            break;
        default:
            break;
    }
}

void GamePad::handleEvent(sf::Event event) {
    unsigned id = 16;
    bool is_pressed = false;
    unsigned device = 0;
    unsigned index = 0;
    switch(event.type) {
        case sf::Event::JoystickButtonPressed:
            handlePress(event.joystickButton.button, id);
            is_pressed = true;
            break;
        case sf::Event::JoystickButtonReleased:
            handlePress(event.joystickButton.button, id);
            is_pressed = false;
            break;
        default:
            break;
    }
    core->setInputStateCallBack(is_pressed, port, device, index, id);
}

void GamePad::sync() {
    sf::Event event;
    while (sfml_window->pollEvent(event))
        handleEvent(event);
}

void GamePad::setPort(unsigned int gamepad_port) {
    port = gamepad_port;
}

void GamePad::setCore(Core *retro_core) {
    core = retro_core;
}

