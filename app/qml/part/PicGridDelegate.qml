import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import ".."
import "../component"
import "../part"

MPane {
    property alias image: image
    property alias text: label.text
    property alias subText: label_sub.text

    signal clicked()

    width: GridView.view.cellWidth
    height: GridView.view.cellHeight
    Component.onCompleted: {
        item_dg.clicked.connect(clicked);
    }

    MItemDelegate {
        id: item_dg

        anchors.horizontalCenter: parent.horizontalCenter
        clip: true
        padding: 8

        contentItem: ColumnLayout {
            Image {
                id: image

                Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
                Layout.preferredWidth: 160
                Layout.preferredHeight: 160
                sourceSize.width: 160
                sourceSize.height: 160
            }

            Label {
                id: label

                Layout.topMargin: 8
                Layout.preferredWidth: 160
                maximumLineCount: 2
                wrapMode: Text.Wrap
                elide: Text.ElideRight
            }

            Label {
                id: label_sub

                visible: !!text
                Layout.alignment: Qt.AlignHCenter
                font.pointSize: Theme.ts.label_small.size
                opacity: 0.6
            }

            Item {
                Layout.fillHeight: true
            }

        }

    }

}