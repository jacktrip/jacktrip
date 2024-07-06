import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects

Item {
    required property string content
    required property int size

    width: size * virtualstudio.uiScale
    height: size * virtualstudio.uiScale

    property string iconSource: "help.svg"
    property string iconColor: ""
    property string backgroundColour: virtualstudio.darkMode ? "#323232" : "#F3F3F3"
    property bool showToolTip: false

    Item {
        anchors.fill: parent

        AppIcon {
            id: tooltipIcon
            anchors.centerIn: parent
            width: parent.width
            height: parent.height
            icon.source: iconSource
            color: iconColor
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
                elide: Text.ElideRight
                wrapMode: Text.WordWrap
            }

            background: Rectangle {
                color: backgroundColour
                radius: 4
                layer.enabled: true
                layer.effect: Glow {
                    radius: 8
                    color: "#66000000"
                    transparentBorder: true
                }
            }
        }
    }
}
