import QtQuick 2.3

Rectangle {
    width: 100;
    height: 62;

    GridView {
        id: gridView;
        model: libraryModel;

        // The max height and width of the grid's cells. This can be tweaked
        // to change the default size of the boxart.
        property int maxCellHeight: 400;
        property bool clampEdges: parent.width >= maxCellHeight;

        anchors {
            top: parent.top;
            topMargin: 24;
            left: parent.left;
            right: parent.right;
            bottom: parent.bottom;


            leftMargin: gridView.clampEdges ? ( ( parent.width % cellWidth ) / 2 ) : 0;
            rightMargin: leftMargin;
            bottomMargin: 40;
        }


        // If the grid's width is less than the maxCellWidth, get
        // the grid to scale the size of the grid items, so that the transition looks really
        // seamless.
        cellHeight: clampEdges ? maxCellHeight : parent.width;
        cellWidth: cellHeight;

        boundsBehavior: Flickable.StopAtBounds;

        Component.onCompleted: libraryModel.updateCount();

        delegate: Rectangle {
            // This needs to be equal to the cellHeight / Width
            // or else grid won't be aligned.

            height: gridView.cellHeight//gridView.clampEdges ? gridView.cellHeight : gridView.width * 0.75;
            width: gridView.cellWidth;

            color: "blue"


            Rectangle {
                anchors {
                    top: parent.top;
                    bottom: parent.bottom;
                    bottomMargin: 24;
                    left: parent.left;
                    right: parent.right;
                    leftMargin: 24
                    rightMargin: 24;
                }


                color: "#474747";

                Text {
                    anchors.centerIn: parent;
                    text: title;
                    color: "white";
                }

                MouseArea {
                    anchors.fill: parent;
                    onClicked: {
                        libraryModel.setFilter( "title = ?", [ title ], true );
                    }
                }
            }
        }
    }
}

