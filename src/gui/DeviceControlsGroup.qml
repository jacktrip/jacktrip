import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.12

Item {
    required property bool showMinified

    property int minifiedHeight: 36
    property int fullHeight: 80

    id: deviceControlsGroup
    width: parent.width
    height: showMinified ? minifiedHeight : fullHeight

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

            DeviceControls {
                isInput: true
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true

            DeviceControls {
                isInput: false
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.preferredWidth: 48

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

                    Image {
                        id: expandIcon
                        width: 20 * virtualstudio.uiScale; height: 20 * virtualstudio.uiScale
                        anchors { verticalCenter: parent.verticalCenter; horizontalCenter: parent.horizontalCenter }
                        source: showMinified ? "expand_less.svg" : "expand_more.svg"
                        sourceSize: Qt.size(expandIcon.width, expandIcon.height)
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                    }

                    Colorize {
                        anchors.fill: expandIcon
                        source: expandIcon
                        hue: 0
                        saturation: 0
                        lightness: imageLightnessValue
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
                    onClicked: studioStatus === "Ready" ? virtualstudio.windowState = "change_devices" : virtualstudio.disconnect() ;
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