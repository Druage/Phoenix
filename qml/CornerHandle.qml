import QtQuick 2.3

Item {
    height: 5;
    width: 5;
    MouseArea {
        id: topRightHandle;
        anchors.fill: parent;

        hoverEnabled: true;
        onContainsMouseChanged: {
            if (containsMouse)
                cursorShape = Qt.SizeBDiagCursor;
            else
                cursorShape = Qt.ArrowCursor;
        }

        property variant clickPos: "1,1"

        onPressed: {
            clickPos = Qt.point(mouse.x, mouse.y);
        }

        onPositionChanged: {
            if (pressed) {
                var delta = Qt.point(mouse.x-clickPos.x, mouse.y-clickPos.y)
                //console.log("root.x : " + root.x + ", delta.x: " + delta.x + ", mouse.x: " + mouse.x + ", mouse.y: " + mouse.y + ", clickPos.x: " + clickPos.x + ", clickPos.y: " + clickPos.y);
                //if (delta.x > 0)
                    root.width = root.width + delta.x;
                //if (delta.y > 0)
                    root.height = root.height + delta.y;
                //if (root.x + delta.x > )
                //root.x = root.x + delta.x;
                //if ()
                root.y = root.y + delta.y;
            }
        }
    }
}
