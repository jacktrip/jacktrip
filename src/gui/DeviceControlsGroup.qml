import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.12

Item {
    required property bool showMinified

    id: deviceControlsGroup
    width: parent.width
    height: 66
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 20

    Rectangle {
        anchors.fill: parent
        color: backgroundColour
    }

    RowLayout {
        id: layout
        anchors.fill: parent
        spacing: 2

        Rectangle {
            color: backgroundColour
            Layout.fillHeight: true
            Layout.fillWidth: true

            DeviceControls {
                isInput: true
            }
        }

        Rectangle {
            color: backgroundColour
            Layout.fillHeight: true
            Layout.fillWidth: true

            DeviceControls {
                isInput: false
            }
        }

        Rectangle {
            color: backgroundColour
            Layout.fillHeight: true
            Layout.preferredWidth: 48
            Layout.minimumWidth: 48
            Layout.maximumWidth: 48

            ColumnLayout {
                anchors.fill: parent
                spacing: 2

                Button {
                    Layout.fillWidth: true
                    Layout.leftMargin: 4 * virtualstudio.uiScale
                    Layout.rightMargin: 4 * virtualstudio.uiScale
                    id: expandButton
                    height: 30
                    background: Rectangle {
                        radius: 4 * virtualstudio.uiScale
                        color: expandButton.down ? browserButtonPressedColour : (expandButton.hovered ? browserButtonHoverColour : browserButtonColour)
                    }
                    onClicked: console.log("yo")

                    Image {
                        id: expandIcon
                        width: 20 * virtualstudio.uiScale; height: 20 * virtualstudio.uiScale
                        anchors { verticalCenter: parent.verticalCenter; horizontalCenter: parent.horizontalCenter }
                        source: showMinified ? "expand_more.svg" : "expand_less.svg"
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
                    Layout.fillWidth: true
                    id: settingsButton
                    width: 32
                    height: 32
                    background: Rectangle {
                        radius: 8 * virtualstudio.uiScale
                        color: settingsButton.down ? browserButtonPressedColour : (settingsButton.hovered ? browserButtonHoverColour : browserButtonColour)
                    }
                    icon.name: "edit-cut"
                    icon.source: "cog.svg"
                    icon.color: textColour
                    onClicked: studioStatus === "Ready" ? virtualstudio.windowState = "change_devices" : virtualstudio.disconnect() ;

                    Text {
                        anchors.top: settingsButton.bottom
                        text: studioStatus === "Ready" ? "Settings" : "Back"
                        font { family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale}
                        anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
                        color: textColour
                    }
                }
            }
        }
    }
}