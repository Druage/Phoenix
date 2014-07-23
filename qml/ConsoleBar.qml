import QtQuick 2.3
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1
import QtGraphicalEffects 1.0

Rectangle {
    id: consoleBar;
    color: "#2b2b2b";
    height: 500;
    width: 250;

    ListView {
        id: listView;
        anchors {
            top: parent.top;
            topMargin: 25;
        }

        height: parent.height / 2;
        snapMode: ListView.SnapToItem;
        orientation: ListView.Vertical;
        interactive: true;
        spacing: 5;
        highlightFollowsCurrentItem: false;

        header: Item {
            height: 25;
            width: parent.width;
            Label {
                anchors {
                    left: parent.left;
                    leftMargin: 25;
                    horizontalCenter: parent.horizontalCenter;
                }

                text: "Consoles";
                color: "#f1f1f1";
                font {
                    family: "Sans";
                    bold: true;
                }
            }
        }

        highlight: Item {
            id: highlightItem;
            height: listView.currentItem.height;
            width: listView.width;
            anchors.verticalCenter: listView.currentItem.verticalCenter;
            y: listView.currentItem.y;
            Item {
                id: innerItem;
                height: parent.height;
                width: parent.width;
                Rectangle {
                    id: accentRectangle;
                    color: root.accentColor;
                    width: 15;
                    anchors {
                        left: parent.left;
                        top: parent.top;
                        bottom: parent.bottom;
                    }
                }


                Rectangle {
                    id: mainColor;
                    anchors {
                        left: accentRectangle.right;
                        right: parent.right;
                        top: parent.top;
                        bottom: parent.bottom;
                    }
                    color: listView.currentItem ? "#666666" : "#000000FF";
                }

                Rectangle {
                    id: triangle;
                    anchors {
                        left: mainColor.right;
                        leftMargin: -(height / 2);
                        verticalCenter: mainColor.verticalCenter;
                    }
                    rotation: 45;
                    height: highlightItem.height / Math.SQRT2;
                    width: height;
                    color: mainColor.color;
                    smooth: true;
                }

            }

            DropShadow {
                width: innerItem.width + 30;
                height: innerItem.height + 5;
                horizontalOffset: 1;
                verticalOffset: 1;
                radius: 8.0
                samples: 16
                transparentBorder: true;
                color: "#80000000"
                opacity: 0.8;
                source: innerItem;
            }
        }

        anchors {
            right: parent.right;
            left: parent.left;
            top: consoleLabel.bottom;
            topMargin: 10;

        }

        model: ListModel {
            ListElement {title: "All"; icon: "";}
            ListElement {title: "Nintendo"; icon: "/assets/consoleicons/nes.png";}
            ListElement {title: "Super Nintendo"; icon: "/assets/consoleicons/snes.png";}
            ListElement {title: "Nintendo 64"; icon: "/assets/consoleicons/mupen64plus.png";}
            ListElement {title: "FFmpeg"; icon: "/assets/consoleicons/ffmpeg.png";}
            ListElement {title: "Sony PlayStation"; icon: "/assets/consoleicons/ps1.png";}
            ListElement {title: "Game Boy Advance"; icon: "/assets/consoleicons/gba.png";}
            ListElement {title: "Game Boy Color"; icon: "/assets/consoleicons/gbcolor.png";}
            ListElement {title: "Nintendo DS"; icon: "/assets/consoleicons/nds.png";}
            ListElement {title: "DOSBox"; icon: "/assets/consoleicons/dosbox.png";}
            ListElement {title: "Dinothawr"; icon: "/assets/consoleicons/dinothawr.png";}
        }

        ExclusiveGroup {
            id: consoleGroup
        }

        delegate: Item {
            height: 33;
            width: consoleBar.width;
            Row {
                anchors.fill: parent;
                anchors.leftMargin: 50;
                spacing: 10;

                Image {
                    anchors.verticalCenter: parent.verticalCenter;
                    source: icon;
                    sourceSize {
                        height: 20;
                        width: 20;
                    }
                    height: 20;
                    width: 20;
                }

                Label {
                    id: consoleItem;
                    anchors.verticalCenter: parent.verticalCenter;
                    text: title;
                    color: "#f1f1f1";
                    font {
                        family: "Sans";
                        bold: true;
                        pixelSize: 14;
                    }
                }
            }

            MouseArea {
                anchors.fill: parent;
                onClicked: listView.currentIndex = index;
            }
        }
    }

    Label {
        id: favorites;
        z: listView.z + 1;
        text: "Favorites";

        font {
            bold: true;
            pixelSize: 18;
            family: "Sans";
        }

        color: "#f1f1f1";
        anchors {
            left: parent.left;
            leftMargin: 25;
            top: listView.bottom;
            topMargin: 40;
        }
    }

    DropShadow {
        anchors.fill: source;
        horizontalOffset: 0;
        verticalOffset: -3;
        radius: 8.0
        samples: 16
        transparentBorder: true;
        color: "#80000000"
        //opacity: 0.8;
        source: progressBar;
    }

    Rectangle {
        id: progressBar;
        z: 1;
        visible: true;
        anchors {
            bottom: parent.bottom;
            left: parent.left;
            right: parent.right;
        }

        height: 75;
        color: "#242323";

        property string showText: "Importing Games";
        onShowTextChanged: {
            if (showText !== "")
                visible = true;
        }

        Column {
            spacing: 4;
            anchors {
                horizontalCenter: parent.horizontalCenter;
                top: parent.top;
                topMargin: 15;
            }
            Label {
                anchors.horizontalCenter: parent.horizontalCenter;
                text: progressBar.showText;
                font.bold: true;
                color: "#f1f1f1";
            }

            ProgressBar {
                anchors.horizontalCenter: parent.horizontalCenter;
                maximumValue: 100;
                minimumValue: 0;
                value: 50;
                style: ProgressBarStyle {
                    background: Rectangle {
                        color: "#666666";
                        opacity: 0.8;
                        implicitWidth: 200;
                        implicitHeight: 8;
                    }
                    progress: Rectangle {
                        color: root.accentColor;
                    }
                }
            }
        }
    }
}
