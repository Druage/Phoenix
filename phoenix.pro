TEMPLATE = app
TARGET = phoenix
#CONFIG +=

QT += widgets core gui multimedia qml quick
LIBS += -LC:/SFML-2.0/lib

CONFIG(release, debug|release): LIBS += -lsfml-audio -lsfml-graphics -lsfml-main -lsfml-network -lsfml-window -lsfml-system
CONFIG(debug, debug|release): LIBS += -lsfml-audio-d -lsfml-graphics-d -lsfml-main-d -lsfml-network-d -lsfml-window-d -lsfml-system-d


INCLUDEPATH += C:/SFML-2.0/include ./include ../RetroArch
DEPENDPATH += C:/Users/robert/Downloads/SFML-master/SFML-master/include

HEADERS += include/core.h       \
           include/video-gl.h   \
           include/audio.h      \
           include/audioio.h    \
           include/input-gamepad.h

SOURCES += src/main.cpp     \
           src/video-gl.cpp \
           src/core.cpp     \
           src/audio.cpp    \
           src/audioio.cpp  \
           src/input-gamepad.cpp


RESOURCES = qml.qrc
