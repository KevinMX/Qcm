import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import ".."

Button {
    // mirrored: root.mirrored

    id: root

    font.pointSize: Theme.ts.label_large.size
    icon.width: 18
    icon.height: 18
    Material.foreground: {
        if (!enabled)
            return Material.hintTextColor;
        else if ((flat && highlighted) || (checked && !highlighted))
            return Material.accentColor;
        else if (highlighted)
            return Theme.color.getOn(Material.accentColor);
        else
            return parent.Material.foreground;
    }

    contentItem: RowLayout {
        spacing: root.spacing

        Label {
            visible: root.display !== AbstractButton.TextOnly && root.icon.name
            text: root.icon.name
            font.family: Theme.font.icon_round.family
            font.pointSize: Theme.ic_size(root.icon.width)
            color: Material.foreground
        }

        Label {
            visible: root.display !== AbstractButton.IconOnly
            text: root.text
            font: root.font
            color: Material.foreground
        }

    }

}
