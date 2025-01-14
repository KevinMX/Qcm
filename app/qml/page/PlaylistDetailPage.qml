import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qcm.App as QA
import Qcm.Material as MD

MD.Page {
    id: root

    property alias itemData: qr_pl.data
    property alias itemId: qr_pl.itemId

    padding: 0

    MD.ListView {
        id: view
        anchors.fill: parent
        reuseItems: true
        clip: true
        contentY: 0

        topMargin: 8

        model: itemData.songs

        header: ColumnLayout {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottomMargin: 8
            anchors.leftMargin: 8
            anchors.rightMargin: 8

            spacing: 4

            RowLayout {
                spacing: 16

                QA.Image {
                    MD.MatProp.elevation: MD.Token.elevation.level2
                    source: `image://ncm/${root.itemData.picUrl}`
                    radius: 16

                    Layout.preferredWidth: displaySize.width
                    Layout.preferredHeight: displaySize.height
                    displaySize: Qt.size(240, 240)
                }

                ColumnLayout {
                    Layout.alignment: Qt.AlignTop
                    spacing: 12

                    MD.Text {
                        Layout.fillWidth: true
                        maximumLineCount: 2
                        text: root.itemData.name
                        typescale: MD.Token.typescale.headline_large
                    }

                    RowLayout {
                        spacing: 12
                        MD.Text {
                            typescale: MD.Token.typescale.body_medium
                            text: `${root.itemData.songs.length} tracks`
                        }
                        MD.Text {
                            typescale: MD.Token.typescale.body_medium
                            text: Qt.formatDateTime(root.itemData.updateTime, 'yyyy.MM.dd')
                        }
                    }
                    QA.ListDescription {
                        description: root.itemData.description.trim()
                        Layout.fillWidth: true
                    }
                }
            }
            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                MD.IconButton {
                    action: Action {
                        icon.name: MD.Token.icon.playlist_add
                        // text: qsTr('add to list')
                        onTriggered: {
                            QA.Global.playlist.appendList(itemData.songs);
                        }
                    }
                }
                MD.IconButton {
                    id: btn_fav
                    action: QA.SubAction {
                        enabled: QA.Global.user_info.userId !== itemData.userId
                        liked: qr_dynamic.data.subscribed
                        querier: qr_sub
                        itemId: root.itemId
                    }
                }
                MD.IconButton {
                    id: btn_comment
                    action: QA.CommentAction {
                        itemId: root.itemId
                    }
                }
            }
        }
        delegate: QA.SongDelegate {
            count: view.count
            width: view.width

            onClicked: {
                QA.Global.playlist.switchTo(modelData);
            }
        }
        footer: MD.ListBusyFooter {
            running: qr_pl.status === QA.ApiQuerierBase.Querying
            width: ListView.view.width
        }
    }
    MD.FAB {
        action: Action {
            icon.name: MD.Token.icon.play_arrow
            onTriggered: {
                const songs = itemData.songs.filter(s => {
                        return s.canPlay;
                    });
                if (songs.length)
                    QA.Global.playlist.switchList(songs);
            }
        }
    }

    QA.PlaylistDetailQuerier {
        id: qr_pl
        autoReload: root.itemId.valid()
    }
    QA.PlaylistDetailDynamicQuerier {
        id: qr_dynamic
        autoReload: itemId.valid()
        itemId: qr_pl.itemId
    }
    QA.PlaylistSubscribeQuerier {
        id: qr_sub
        autoReload: false

        onStatusChanged: {
            if (status === QA.ApiQuerierBase.Finished)
                QA.App.playlistLiked(itemId, sub);
        }
    }
}
