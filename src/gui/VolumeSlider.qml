import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    width: parent.width
    height: parent.height

    required property string labelText
    required property string tooltipText
    required property bool sliderEnabled

    property string sliderColour: virtualstudio.darkMode ? "#BABCBC" :  "#EAECEC"
    property string sliderPressedColour: virtualstudio.darkMode ? "#ACAFAF" : "#DEE0E0"
    property string sliderTrackColour: virtualstudio.darkMode ? "#5B5858" : "light gray"
    property string sliderActiveTrackColour: virtualstudio.darkMode ? "light gray" : "black"

    Item {
        anchors.fill: parent

        Text {
            id: label
            anchors.left: parent.left
            anchors.top: parent.top
            width: 40 * virtualstudio.uiScale
            horizontalAlignment: Text.AlignRight
            text: labelText
            font {family: "Poppins"; weight: Font.Medium; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
        }

        InfoTooltip {
            id: tooltip
            content: tooltipText
            size: 14
            anchors.left: label.right
            anchors.leftMargin: 2 * virtualstudio.uiScale
            anchors.verticalCenter: label.verticalCenter
        }

        Slider {
            id: slider
            value: labelText == "Monitor" ? virtualstudio.monitorVolume : (labelText == "Studio" ? virtualstudio.outputVolume : virtualstudio.inputVolume )
            onMoved: {
                if (labelText == "Monitor") {
                    virtualstudio.monitorVolume = value;
                } else if (labelText == "Studio") {
                    virtualstudio.outputVolume = value;
                } else {
                    virtualstudio.inputVolume = value;
                }
            }
            enabled: sliderEnabled
            from: 0.0
            to: 1.0
            stepSize: 0.01
            padding: 0
            opacity: sliderEnabled ? 1 : 0.3
            anchors.left: tooltip.right
            anchors.leftMargin: 8 * virtualstudio.uiScale
            anchors.right: parent.right
            anchors.verticalCenter: label.verticalCenter

            background: Rectangle {
                x: slider.leftPadding
                y: slider.topPadding + slider.availableHeight / 2 - height / 2
                implicitWidth: parent.width
                implicitHeight: 6
                width: slider.availableWidth
                height: implicitHeight
                radius: 4
                color: sliderTrackColour

                Rectangle {
                    width: slider.visualPosition * parent.width
                    height: parent.height
                    color: sliderActiveTrackColour
                    radius: 4
                }
            }

            handle: Rectangle {
                x: slider.leftPadding + slider.visualPosition * (slider.availableWidth - width)
                y: slider.topPadding + slider.availableHeight / 2 - height / 2
                implicitWidth: 16 * virtualstudio.uiScale
                implicitHeight: 16 * virtualstudio.uiScale
                radius: 8 * virtualstudio.uiScale
                color: slider.pressed ? sliderPressedColour : sliderColour
                border.color: buttonStroke
            }
        }
    }
}
