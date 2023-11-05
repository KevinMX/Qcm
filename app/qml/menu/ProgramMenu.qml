import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qcm.App as QA
import Qcm.Material as MD

MD.Menu {
    id: root

    required property QA.t_program program

    dim: false
    font.capitalization: Font.Capitalize
    modal: true

    QA.PlatnextAction {
        song: root.program.song
    }
    QA.CommentAction {
        itemId: root.program
    }
}