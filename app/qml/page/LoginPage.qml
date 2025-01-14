import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qcm.App as QA
import Qcm.Material as MD

MD.Page {
    id: root
    ColumnLayout {
        anchors.fill: parent

        readonly property bool loginCodeOk: qr_login.data.code === 200 || qr_qrlogin.data.code === 803

        onLoginCodeOkChanged: {
            QA.Global.querier_user.query();
        }

        QA.QrcodeLoginQuerier {
            id: qr_qrlogin
            key: qr_unikey.data.key
        }
        QA.QrcodeUnikeyQuerier {
            id: qr_unikey

            readonly property int loginCode: qr_qrlogin.data.code

            onLoginCodeChanged: {
                if (loginCode === 800)
                    query();
            }
        }
        QA.LoginQuerier {
            id: qr_login
            function login() {
                username = tf_username.text;
                password = QA.App.md5(tf_password.text);
                query();
            }

            autoReload: false
        }

        ColumnLayout {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: false
            Layout.preferredWidth: 300

            MD.Text {
                Layout.bottomMargin: 20
                Layout.fillWidth: true
                typescale: MD.Token.typescale.title_large
                font.capitalization: Font.Capitalize
                text: qsTr('login')
            }
            MD.TabBar {
                id: bar
                Layout.fillWidth: true
                font.capitalization: Font.Capitalize

                onCurrentIndexChanged: {
                    view.currentIndex = currentIndex;
                }

                MD.TabButton {
                    text: qsTr("email")
                }
                MD.TabButton {
                    text: qsTr("qr")
                }
            }
            SwipeView {
                id: view
                Layout.fillWidth: true
                clip: true
                implicitHeight: Math.max(mail_pane.implicitHeight, qr_pane.implicitHeight)

                onCurrentIndexChanged: {
                    bar.currentIndex = currentIndex;
                }

                MD.Pane {
                    id: mail_pane
                    ColumnLayout {
                        anchors.fill: parent

                        MD.TextField {
                            id: tf_username
                            Layout.fillWidth: true
                            placeholderText: 'email'
                        }
                        MD.TextField {
                            id: tf_password
                            Layout.fillWidth: true
                            echoMode: TextInput.Password
                            placeholderText: 'password'
                        }
                        MD.Text {
                            MD.MatProp.textColor: MD.Token.color.error
                            text: {
                                switch (qr_login.data.code) {
                                case 501:
                                    return qsTr('email not exists');
                                case 502:
                                    return qsTr('wrong password');
                                default:
                                    return '';
                                }
                            }
                        }
                        MD.Button {
                            Layout.fillWidth: true
                            enabled: qr_login.status !== QA.ApiQuerierBase.Querying
                            font.capitalization: Font.Capitalize
                            highlighted: true
                            text: qsTr('login in')

                            Component.onCompleted: {
                                tf_username.accepted.connect(clicked);
                                tf_password.accepted.connect(clicked);
                            }
                            onClicked: {
                                qr_login.login();
                            }
                        }
                    }
                }
                MD.Pane {
                    id: qr_pane
                    clip: true

                    StackView {
                        id: qr_stack

                        property QtObject cur: switch (qr_qrlogin.data.code) {
                        case 803:
                        case 802:
                            return comp_qr_wait_comfirm;
                        default:
                            return comp_qr_wait_scan;
                        }

                        anchors.fill: parent
                        implicitHeight: 224

                        onCurChanged: {
                            replace(currentItem, cur);
                        }
                    }
                    Component {
                        id: comp_qr_wait_scan
                        ColumnLayout {
                            MD.Pane {
                                Layout.alignment: Qt.AlignCenter
                                MD.MatProp.backgroundColor: 'white'
                                MD.MatProp.elevation: MD.Token.elevation.level3
                                padding: 12
                                radius: 12
                                MD.Image {
                                    id: qr_image
                                    anchors.centerIn: parent
                                    cache: false
                                    source: `image://qr/${qr_unikey.data.qrurl}`
                                    sourceSize.height: 200
                                    sourceSize.width: 200
                                }
                            }
                        }
                    }
                    Component {
                        id: comp_qr_wait_comfirm
                        ColumnLayout {
                            MD.Image {
                                Layout.alignment: Qt.AlignHCenter

                                source: `image://ncm/${qr_qrlogin.data.avatarUrl}`
                                sourceSize.height: 96
                                sourceSize.width: 96
                                radius: height / 2
                            }
                            MD.Text {
                                Layout.alignment: Qt.AlignHCenter
                                text: qr_qrlogin.data.nickname
                            }
                            MD.Text {
                                Layout.alignment: Qt.AlignHCenter
                                text: qr_qrlogin.data.message
                            }
                        }
                    }
                    Timer {
                        interval: 2000
                        repeat: true
                        running: qr_pane.SwipeView.isCurrentItem
                        triggeredOnStart: true

                        onTriggered: {
                            qr_qrlogin.query();
                        }
                    }
                }
            }
        }
    }

    MD.IconButton {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.leftMargin: 16
        anchors.bottomMargin: 16

        action: Action {
            readonly property bool is_dark_theme: QA.Global.color_scheme == MD.MdColorMgr.Dark
            icon.name: is_dark_theme ? MD.Token.icon.dark_mode : MD.Token.icon.light_mode

            onTriggered: {
                QA.Global.color_scheme = is_dark_theme ? MD.MdColorMgr.Light : MD.MdColorMgr.Dark;
            }
        }
    }
}
