import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    required property bool showMinified

    property int minifiedHeight: 36
    property int fullHeight: 80

    id: deviceControlsGroup
    width: parent.width
    height: showMinified ? minifiedHeight : fullHeight
    property bool showDeviceControls: studioStatus === "Ready"

    Rectangle {
        anchors.fill: parent
        color: backgroundColour
    }

    RowLayout {
        id: layout
        anchors.fill: parent
        spacing: 2

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
            visible: !showDeviceControls

            Button {
                id: backButton
                anchors.centerIn: parent
                width: 180 * virtualstudio.uiScale
                height: 40 * virtualstudio.uiScale
                background: Rectangle {
                    radius: 8 * virtualstudio.uiScale
                    color: backButton.down ? browserButtonPressedColour : (backButton.hovered ? browserButtonHoverColour : browserButtonColour)
                }
                onClicked: virtualstudio.disconnect()

                Text {
                    text: "Back to Studios"
                    font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale}
                    anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
                    color: textColour
                }
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
            visible: showDeviceControls

            DeviceControls {
                isInput: true
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
            visible: showDeviceControls

            DeviceControls {
                isInput: false
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.preferredWidth: 48
            visible: showDeviceControls

            ColumnLayout {
                anchors.fill: parent
                spacing: 2

                Button {
                    Layout.preferredHeight: 20
                    Layout.preferredWidth: 40
                    Layout.alignment: Qt.AlignHCenter

                    id: expandButton
                    background: Rectangle {
                        radius: 4 * virtualstudio.uiScale
                        color: expandButton.down ? browserButtonPressedColour : (expandButton.hovered ? browserButtonHoverColour : browserButtonColour)
                    }
                    onClicked: showMinified = !showMinified

                    AppIcon {
                        id: expandIcon
                        anchors { verticalCenter: parent.verticalCenter; horizontalCenter: parent.horizontalCenter }
                        width: 20 * virtualstudio.uiScale
                        height: 20 * virtualstudio.uiScale
                        icon.source: showMinified ? "expand_less.svg" : "expand_more.svg"
                        onClicked: showMinified = !showMinified
                    }
                }

                Button {
                    Layout.preferredHeight: 32
                    Layout.preferredWidth: 32
                    Layout.alignment: Qt.AlignHCenter
                    id: settingsButton
                    visible: !showMinified
                    background: Rectangle {
                        radius: 8 * virtualstudio.uiScale
                        color: settingsButton.down ? browserButtonPressedColour : (settingsButton.hovered ? browserButtonHoverColour : browserButtonColour)
                    }
                    icon.name: "edit-cut"
                    icon.source: "cog.svg"
                    icon.color: textColour
                    onClicked: virtualstudio.windowState = "change_devices"
                }
            }
        }
    }

    Rectangle {
        id: backgroundBorder
        width: parent.width
        height: 1
        anchors.top: layout.top
        color: strokeColor
    }
}