import QtQuick 2.3
import QtQuick.Controls 1.2

import vg.phoenix.backend 1.0

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: qsTr("Phoenix");

    InputManager {
        id: backendInputManager;
    }

    VideoItem {
        inputManager: backendInputManager;
        anchors {
            top: parent.top;
            bottom: parent.bottom;
        }

        width: height * ( 4 / 3 );
    }

}
