import QtQuick 2.3

Item {
    width: 5;
    MouseArea {
        anchors.fill: parent;
        hoverEnabled: true;
        onContainsMouseChanged: {
            if (containsMouse)
                cursorShape = Qt.SizeHorCursor;
            else
                cursorShape = Qt.ArrowCursor;
        }

        property int clickPos: 1;

        onPressed: {
                clickPos = mouse.x;
        }

        onPositionChanged: {
            if (pressed) {
                root.width = root.width + (mouse.x-clickPos);
            }
        }
    }
}
