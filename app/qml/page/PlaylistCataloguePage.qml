import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import Qt.labs.settings
import QcmApp
import ".."
import "../component"
import "../part"

Page {
    property list<string> mine_cats: []

    id: root

    padding: 0
    title: qsTr('Playlist Tags')

    Flickable {
        id: flick

        anchors.fill: parent
        topMargin: 16
        bottomMargin: 16
        contentWidth: width
        contentHeight: content.implicitHeight
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        ColumnLayout {
            id: content

            signal catClicked(string group, string cat)

            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 12
            spacing: 12

            CatalogueSection {
                id: mine_section

                property var set: new Set(settings.value('cat_list', []))

                function add(cat) {
                    if (set.has(set))
                        return ;

                    set.add(cat);
                    setChanged();
                    root.mine_cats = root.mine_cats.concat([cat]);
                }

                function remove(cat) {
                    if (!set.has(cat))
                        return ;

                    set.delete(cat);
                    setChanged();
                    root.mine_cats = root.mine_cats.filter((el) => {
                        return set.has(el);
                    });
                }
                is_mine: true
                text: 'Mine'
                model: root.mine_cats.map((s) => {
                    return {
                        "name": s
                    };
                })
            }

            Repeater {
                model: qr_cat.data.categories

                delegate: CatalogueSection {
                    text: modelData
                    model: qr_cat.data.category(modelData)
                }

            }

            component CatalogueSection: ColumnLayout {
                id: cat_section

                property alias text: label.text
                property alias model: flow_repeater.model
                property bool is_mine: false

                Label {
                    id: label

                    font.pointSize: Theme.ts.label_large.size
                }

                Flow {
                    Layout.fillWidth: true
                    spacing: 8

                    Repeater {
                        id: flow_repeater

                        delegate: MButton {
                            enabled: cat_section.is_mine || !mine_section.set.has(modelData.name)
                            Material.elevation: 0
                            highlighted: true
                            Material.accent: Theme.color.surface_1

                            action: Action {
                                text: modelData.name
                                onTriggered: {
                                    if (cat_section.is_mine)
                                        mine_section.remove(modelData.name);
                                    else
                                        mine_section.add(modelData.name);
                                }
                            }

                        }

                    }

                }

            }

        }

    }

    Settings {
        id: settings

        property alias cat_list: root.mine_cats

        category: QA.user_setting_category
    }

    ApiContainer {
        PlaylistCatalogueQuerier {
            id: qr_cat
        }

    }

}
