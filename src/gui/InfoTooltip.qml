import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.12

Item {
    width: parent.width
    height: parent.height

    required property string content
    required property int size

    property string toolTipBackgroundColour: virtualstudio.darkMode ? "#323232" : "#F3F3F3"
    property real imageLightnessValue: virtualstudio.darkMode ? 0.8 : 0.2
    property int rightToolTipMargin: 4

    Item {
        Image {
            id: helpIcon
            source: "help.svg"
            sourceSize: Qt.size(size * virtualstudio.uiScale, size * virtualstudio.uiScale)
            fillMode: Image.PreserveAspectFit
            smooth: true

            property bool showToolTip: false

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
                onEntered: helpIcon.showToolTip = true
                onExited: helpIcon.showToolTip = false
            }

            ToolTip {
                visible: helpIcon.showToolTip
                contentItem: Rectangle {
                    color: toolTipBackgroundColour
                    radius: 3
                    anchors.top: parent.bottom
                    anchors.topMargin: 20 * virtualstudio.uiScale
                    anchors.rightMargin: 4 * virtualstudio.uiScale
                    layer.enabled: true
                    border.width: 1
                    border.color: buttonStroke

                    Text {
                        anchors.centerIn: parent
                        font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale}
                        text: content
                        color: textColour
                    }
                }
                background: Rectangle {
                    color: "transparent"
                }
            }
        }

        Colorize {
            anchors.fill: helpIcon
            source: helpIcon
            hue: 0
            saturation: 0
            lightness: imageLightnessValue
        }
    }
}
