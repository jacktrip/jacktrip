import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.12

Item {
    required property string content
    required property int size
    property string iconSource: "help.svg"

    width: size * virtualstudio.uiScale
    height: size * virtualstudio.uiScale

    property string toolTipBackgroundColour: virtualstudio.darkMode ? "#323232" : "#F3F3F3"
    property bool showToolTip: false

    Item {
        anchors.fill: parent

        Image {
            id: tooltipIcon
            source: iconSource
            sourceSize: Qt.size(parent.width, parent.height)
            fillMode: Image.PreserveAspectFit
            smooth: true
        }

        Colorize {
            anchors.fill: tooltipIcon
            source: tooltipIcon
            hue: 0
            saturation: 0
            lightness: 0.6
        }

        MouseArea {
            id: mouseArea
            anchors.fill: tooltipIcon
            hoverEnabled: true
            onEntered: showToolTip = true
            onExited: showToolTip = false
        }

        ToolTip {
            visible: showToolTip
            x: tooltipIcon.x + tooltipIcon.width
            y: tooltipIcon.y + tooltipIcon.height

            contentItem: Text {
                text: content
                font { family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
                color: textColour
            }

            background: Rectangle {
                color: backgroundColour
                radius: 4
                layer.enabled: true
                layer.effect: Glow {
                    samples: 17
                    color: "#55000000"
                    transparentBorder: true
                }
            }
        }
    }
}
