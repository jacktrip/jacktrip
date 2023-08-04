import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    required property bool showMinified

    property int minifiedHeight: 36
    property int fullHeight: 80

    id: deviceControlsGroup
    width: parent.width
    height: showMinified ? minifiedHeight : fullHeight
    color: backgroundColour

    property bool showDeviceControls: studioStatus === "Ready"

    MouseArea {
        anchors.fill: parent
        propagateComposedEvents: false
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

                Item {
                    Layout.preferredHeight: 48
                    Layout.preferredWidth: 40
                    Layout.alignment: Qt.AlignHCenter
                    visible: !showMinified

                    Button {
                        id: changeDevicesButton
                        width: 36
                        height: 36
                        anchors.top: parent.top
                        anchors.horizontalCenter: parent.horizontalCenter
                        background: Rectangle {
                            radius: 8 * virtualstudio.uiScale
                            color: changeDevicesButton.down ? browserButtonPressedColour : (changeDevicesButton.hovered ? browserButtonHoverColour : browserButtonColour)
                        }
                        onClicked: virtualstudio.windowState = "change_devices"

                        AppIcon {
                            id: changeDevicesIcon
                            anchors { verticalCenter: parent.verticalCenter; horizontalCenter: parent.horizontalCenter }
                            width: 20 * virtualstudio.uiScale
                            height: 20 * virtualstudio.uiScale
                            icon.source: "cog.svg"
                            onClicked: virtualstudio.windowState = "change_devices"
                        }
                    }

                    Text {
                        anchors.top: changeDevicesButton.bottom
                        text: "Devices"
                        font { family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale}
                        anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
                        color: textColour
                    }
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