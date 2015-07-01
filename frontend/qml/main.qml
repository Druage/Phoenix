import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1

import vg.phoenix.backend 1.0

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: qsTr("Phoenix");


    RowLayout {
        anchors {
            fill: parent;
        }

        spacing: 0;

        SelectionArea {
            anchors {
                top: parent.top;
                bottom: parent.bottom;
            }

            width: 250;
        }

        ContentArea {
            anchors {
                top: parent.top;
                bottom: parent.bottom;
            }

            Layout.fillWidth: true;

        }
    }


}
