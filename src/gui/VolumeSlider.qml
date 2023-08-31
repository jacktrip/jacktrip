import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    width: parent.width
    height: parent.height

    required property string labelText
    required property string tooltipText
    required property bool sliderEnabled
    property bool showLabel: true
    property int fontSize: 8

    property string iconColor: virtualstudio.darkMode ? "#ACAFAF" : "gray"
    property string sliderPressedColour: virtualstudio.darkMode ? "#BABCBC" :  "#EAECEC"

    Item {
        anchors.fill: parent
        anchors.verticalCenter: parent.verticalCenter

        Text {
            id: label
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            width: 40 * virtualstudio.uiScale
            horizontalAlignment: Text.AlignRight
            text: labelText
            font {family: "Poppins"; weight: Font.Medium; pixelSize: fontSize * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            visible: showLabel
        }

        InfoTooltip {
            id: tooltip
            content: tooltipText
            size: 16
            anchors.left: label.right
            anchors.leftMargin: 2 * virtualstudio.uiScale
            anchors.verticalCenter: label.verticalCenter
            visible: showLabel
        }

        AppIcon {
            id: quieterIcon
            anchors.left: showLabel ? tooltip.right : parent.left
            anchors.leftMargin: showLabel ? 8 * virtualstudio.uiScale : 0
            anchors.verticalCenter: label.verticalCenter
            width: 16
            height: 16
            icon.source: "quiet.svg"
            color: iconColor
        }

        AppIcon {
            id: louderIcon
            anchors.right: parent.right
            anchors.verticalCenter: label.verticalCenter
            width: 18
            height: 18
            icon.source: "loud.svg"
            color: iconColor
        }

        Slider {
            id: slider
            value: labelText == "Monitor" ? audio.monitorVolume : (labelText == "Studio" ? audio.outputVolume : audio.inputVolume )
            onMoved: {
                if (labelText == "Monitor") {
                    audio.monitorVolume = value;
                } else if (labelText == "Studio") {
                    audio.outputVolume = value;
                } else {
                    audio.inputVolume = value;
                }
            }
            enabled: sliderEnabled
            from: 0.0
            to: 1.0
            stepSize: 0.01
            padding: 0
            anchors.left: quieterIcon.right
            anchors.leftMargin: 4 * virtualstudio.uiScale
            anchors.right: louderIcon.left
            anchors.rightMargin: 4 * virtualstudio.uiScale
            anchors.verticalCenter: label.verticalCenter

            background: Rectangle {
                x: slider.leftPadding
                y: slider.topPadding + slider.availableHeight / 2 - height / 2
                implicitWidth: parent.width
                implicitHeight: 8 * virtualstudio.uiScale
                width: slider.availableWidth
                height: implicitHeight
                radius: 4
                color: "gray"

                Rectangle {
                    width: slider.visualPosition * parent.width
                    height: parent.height
                    radius: 4
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0.0; color: sliderEnabled ? "#67C6F3" : "light gray" }
                        GradientStop { position: 1.0; color: sliderEnabled ? "#00897b" : "light gray" }
                    }
                }
            }

            handle: Rectangle {
                x: slider.leftPadding + slider.visualPosition * (slider.availableWidth - width)
                y: slider.topPadding + slider.availableHeight / 2 - height / 2
                implicitWidth: 18 * virtualstudio.uiScale
                implicitHeight: 18 * virtualstudio.uiScale
                radius: implicitWidth / 2
                color: slider.pressed ? sliderPressedColour : "white"
                border.width: 3
                border.color: sliderEnabled ? "#00897b" : "light gray"
            }
        }
    }
}
