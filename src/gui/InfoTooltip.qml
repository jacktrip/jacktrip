import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15

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

        AppIcon {
            id: tooltipIcon
            anchors.centerIn: parent
            width: parent.width
            height: parent.height
            icon.source: iconSource
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
                /*
                layer.effect: Glow {
                    samples: 17
                    color: "#55000000"
                    transparentBorder: true
                }
                */
            }
        }
    }
}
