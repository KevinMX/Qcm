import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qcm.App as QA
import Qcm.Material as MD

MD.Menu {
    id: root

    required property QA.t_album album
    readonly property var itemId: album.itemId

    dim: false
    font.capitalization: Font.Capitalize
    modal: true

    QA.CommentAction {
        itemId: root.itemId
    }

    QA.SubAction {
        liked: root.album.subscribed
        querier: qr_sub
        itemId: root.itemId
    }

    QA.AlbumSublistQuerier {
        id: qr_sub
        autoReload: false
        onStatusChanged: {
            if (status === QA.ApiQuerierBase.Finished)
                QA.App.playlistLiked(itemId, sub);
        }
    }
}