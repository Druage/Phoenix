import QtQuick 2.3
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1
import QtQuick.Dialogs 1.1
import Qt.labs.settings 1.0

Rectangle {
    id: headerBar;
    property string headerColor: "#404040";
    property int fontSize: 13;

    property real sliderValue: 0;
    property bool sliderPressed: false;

    width: 300;
    height: 50;
    color: headerColor;



    SettingsWindow {
        id: settingsWindow;
        visible: false;
        width: 500;
        height: 500;
        stackBackgroundColor: "#383838";
        textColor: "#f1f1f1";
        groupingColor: "#535353";

        Component.onDestruction: gc(); // Start garbage collector
    }

    Row {
        id: settingsRow;
        anchors {
            left: parent.left;
            leftMargin: 20;
            verticalCenter: parent.verticalCenter;
        }
        spacing: 10;

        Button {
            id: settingsBtn;
            height: 30;
            width: 30;
            property string backgroundColor: "#000000FF";
            onHoveredChanged: {
                if (hovered) {
                    backgroundColor = "#525252";
                }
                else
                    backgroundColor = "#000000FF"
            }

            style: ButtonStyle {
                background: Rectangle {
                    color: settingsBtn.backgroundColor;
                }

                label: Image{
                    source: "../assets/cog-6x.png";
                    sourceSize.height: 23;
                    sourceSize.width: 23;
                }

            }
            onPressedChanged:  {
                settingsWindow.visible = true;
            }

        }

        FileDialog {
            id: folderDialog;
            selectFolder: true;
            title: "Add Folder to Library";
            visible: false;

        }

        Button {
            id: folderBtn;
            property string backgroundColor: "#000000FF";
            height: 30;
            width: 30;
            onHoveredChanged: {
                if (hovered) {
                    backgroundColor = "#525252";
                }
                else
                    backgroundColor = "#000000FF"
            }
            style: ButtonStyle {
                background: Rectangle {
                    color: folderBtn.backgroundColor;
                }

                label: Image{
                    source: "../assets/folder-8x.png";
                    //opacity: 0.85;
                    sourceSize.height: 25;
                    sourceSize.width: 25;
                }

            }
            onPressedChanged: {
                if (pressed) {
                    folderDialog.visible = true;
                }
            }

        }

        Button {
            id: viewBtn;
            height: 30;
            width: 30;
            property string backgroundColor: "#000000FF";
            property string imageSource: "../assets/grid-three-up-8x.png";
            onHoveredChanged: {
                if (hovered) {
                    backgroundColor = "#525252";
                }
                else
                    backgroundColor = "#000000FF"
            }
            style: ButtonStyle {
                background: Rectangle {
                    color: viewBtn.backgroundColor;
                }

                label: Image{
                    source: viewBtn.imageSource;
                    //opacity: 0.85;
                    sourceSize.height: 25;
                    sourceSize.width: 25;
                }

            }
            onPressedChanged: {
                if (pressed) {
                    if (gameStack.get(0, false).itemName === "grid") {
                        imageSource = "../assets/list-8x.png";
                        gameStack.clear();
                        gameStack.push(gameTable);
                    }
                    else {
                        imageSource = "../assets/grid-three-up-8x.png";
                        gameStack.clear();
                        gameStack.push(gameGrid);
                    }


                }
                else
                    backgroundColor = "#000000FF";
            }

        }

        Slider {
            id: zoomSlider;
            width: 150;
            height: 5;
            anchors {
                verticalCenter: parent.verticalCenter;
            }
            stepSize: 1;
            minimumValue: 1;
            maximumValue: 10;
            value: 5;


            Settings {
                category: "UI";
                property alias zoomFactor: zoomSlider.value;
            }

            onPressedChanged: {
                if (pressed)
                    headerBar.sliderPressed = true;
                else
                    headerBar.sliderPressed = false;
            }

            onValueChanged: {
                var prev = headerBar.sliderValue;
                headerBar.sliderValue = value;

            }

            style: SliderStyle {
                handle: Rectangle {
                    height: 18;
                    width: 18;
                    color: "#f1f1f1";
                    radius: 8;
                }

                groove: Rectangle {
                    implicitHeight: 5;
                    implicitWidth: 200;
                    radius: 2;
                    opacity: 0.8;
                    color: "#333333";
                }



            }

        }

    }

    MouseArea {
        property variant clickPos: "1,1"
        anchors {
            left: settingsRow.right;
            top: headerBar.top;
            bottom: headerBar.bottom;
            right: searchBar.left;
        }

        onPressed: {
            clickPos = Qt.point(mouse.x, mouse.y);
        }

        onPositionChanged: {
            var delta = Qt.point(mouse.x-clickPos.x, mouse.y-clickPos.y)
            root.x = root.x+delta.x;
            root.y = root.y+delta.y;
        }

        onDoubleClicked: {
            if (root.visibility === 5)
                root.visibility = "Windowed";
            else
                root.visibility = 5;
        }
    }

    /*Label {
        anchors.centerIn: parent;
        text: "Phoenix";
        color: "#f1f1f1";
        font.bold: true;
        font.pixelSize: headerBar.fontSize;
    }*/

    TextField {
        id: searchBar;
        width: 175;
        placeholderText: "Search";
        font {
            bold: true;
            pixelSize: 14;
        }

        textColor: "#f1f1f1";
        height: 25;

        anchors {
            right: menuRow.left;
            rightMargin: 20;
            verticalCenter: parent.verticalCenter;
        }

        Timer {
            id: searchTimer;
            interval: 300;
            running: false;
            repeat: false;
            onTriggered: gamelibrary.setFilter(searchBar.text);
        }

        onTextChanged: {
            searchTimer.restart();
        }

        style: TextFieldStyle {
            placeholderTextColor: "#f1f1f1";

            background: Rectangle {
                radius: 2;
                opacity: 0.8;
                color: "#333333";

            }

        }

        Image {
            id: image;
            focus: true;
            anchors {
                verticalCenter: parent.verticalCenter;
                right: parent.right;
                margins: 5;
            }
            visible: (searchBar.displayText == "") ? false : true;
            source: "../assets/delete-4x.png"
            sourceSize.height: 15;
            sourceSize.width: 15;
            MouseArea {
                anchors.fill: parent;
                onClicked: searchBar.text = "";
            }

        }
    }

    Row {
        id: menuRow;
        anchors {
            right: parent.right;
            //top: parent.top;
            rightMargin: 5;
            //topMargin:  5;
            verticalCenter: parent.verticalCenter;
        }

        Button {
            height: 32;
            width: 32;
            onClicked: {
                if (root.visibility === 5)
                    root.visibility = "Windowed";
                else
                    root.visibility = 5;
            }

            style: ButtonStyle {
                background: Rectangle {
                    color: "#000000FF";
                }
                label: Image {
                    source: "../assets/minimize.png";
                    sourceSize {
                        height: height;
                        width: width;
                    }
                    height: closeButton.height;
                    width: closeButton.width;
                }
            }
        }
        Button {
            height: 32;
            width: 32;
            onClicked: {
                if (root.visibility === 5)
                    root.visibility = "Windowed";
                else
                    root.visibility = 5;
            }

            style: ButtonStyle {
                background: Rectangle {
                    color: "#000000FF";
                }
                label: Image {
                    source: "../assets/maximize.png";
                    sourceSize {
                        height: height;
                        width: width;
                    }
                    height: closeButton.height;
                    width: closeButton.width;
                }
            }
        }
        Button {
            id: closeButton;
            onClicked: Qt.quit();
            height: 32;
            width: 32;
            style: ButtonStyle {
                background: Rectangle {
                    color: "#000000FF";
                }
                label: Image {
                    source: "../assets/close.png";
                    sourceSize {
                        height: height;
                        width: width;
                    }
                    height: closeButton.height;
                    width: closeButton.width;
                }
            }
        }
    }

}
