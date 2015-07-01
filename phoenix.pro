TEMPLATE = subdirs

SUBDIRS += phoenix-backend
SUBDIRS += build

phoenix-backend.file = phoenix-backend/phoenix-backend.pro
build.file = build/build.pro

CONFIG += ordered

#Include the frontend code.
SOURCES += main.cpp

RESOURCES += qml/qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(common.pri)
