import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qcm.App as QA
import Qcm.Material as MD

MD.Page {
    id: root

    property alias itemData: qr_al.data
    property alias itemId: qr_al.itemId

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
                            text: `${root.itemData.size} tracks`
                        }
                        MD.Text {
                            typescale: MD.Token.typescale.body_medium
                            text: Qt.formatDateTime(root.itemData.publishTime, 'yyyy.MM')
                        }
                    }
                    MD.Text {
                        typescale: MD.Token.typescale.body_medium
                        text: QA.Global.join_name(root.itemData.artists, '/')
                        /*
                        onClicked: {
                            const artists = root.itemData.artists;
                            if (artists.length === 1)
                                QA.Global.route(artists[0].itemId);
                            else
                                QA.Global.show_popup('qrc:/Qcm/App/qml/component/ArtistsPopup.qml', {
                                        "model": artists
                                    });
                        }
                        */
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
                        liked: qr_dynamic.data.isSub
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
            subtitle: QA.Global.join_name(modelData.artists, '/')

            onClicked: {
                QA.Global.playlist.switchTo(modelData);
            }
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

    QA.AlbumDetailQuerier {
        id: qr_al
        autoReload: root.itemId.valid()
    }
    QA.AlbumDetailDynamicQuerier {
        id: qr_dynamic
        autoReload: itemId.valid()
        itemId: qr_al.itemId
    }
    QA.AlbumSubQuerier {
        id: qr_sub
        autoReload: false

        onStatusChanged: {
            if (status === QA.ApiQuerierBase.Finished) {
                QA.App.albumLiked(itemId, sub);
            }
        }
    }
}
