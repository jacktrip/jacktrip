import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.12

Item {
    width: parent.width
    height: parent.height
    clip: true

    required property bool isInput

    Component {
        id: indicatorIcon

        Button {
            id: iconButton
            width: 24 * virtualstudio.uiScale
            height: 24 * virtualstudio.uiScale

            anchors.left: parent.left
            anchors.leftMargin: 8 * virtualstudio.uiScale
            anchors.verticalCenter: parent.verticalCenter

            background: Rectangle {
                color: isInput ? (virtualstudio.inputMuted ? muteButtonMutedColor : buttonColour) : "transparent"
                width: 24 * virtualstudio.uiScale
                radius: 4 * virtualstudio.uiScale
            }

            onClicked: isInput ? virtualstudio.inputMuted = !virtualstudio.inputMuted : console.log()

            Image {
                id: iconImage
                width: 18 * virtualstudio.uiScale; height: 18 * virtualstudio.uiScale
                anchors.centerIn: parent

                source: isInput ? (virtualstudio.inputMuted ? "micoff.svg" : "mic.svg") : "headphones.svg"
                sourceSize: Qt.size(iconImage.width, iconImage.height)
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            Colorize {
                anchors.fill: iconImage
                source: iconImage
                hue: 0
                saturation: isInput ? (virtualstudio.inputMuted ? muteButtonMutedSaturationValue : 0) : 0
                lightness: isInput ? (virtualstudio.inputMuted ? (iconButton.hovered ? muteButtonMutedLightnessValue + .1 : muteButtonMutedLightnessValue) : (iconButton.hovered ? muteButtonLightnessValue - .1 : muteButtonLightnessValue)) : imageLightnessValue
            }

            ToolTip {
                parent: iconButton
                visible: isInput && iconButton.hovered

                contentItem: Rectangle {
                    color: toolTipBackgroundColour
                    radius: 3
                    anchors.top: parent.bottom
                    anchors.topMargin: 20 * virtualstudio.uiScale
                    anchors.rightMargin: rightToolTipMargin * virtualstudio.uiScale
                    layer.enabled: true
                    border.width: 1
                    border.color: buttonStroke

                    Text {
                        anchors.centerIn: parent
                        font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                        text: virtualstudio.inputMuted ?  qsTr("Click to unmute yourself") : qsTr("Click to mute yourself")
                        color: toolTipTextColour
                    }
                }
                background: Rectangle {
                    color: "transparent"
                }
            }
        }
    }

    Component {
        id: inputControls

        Slider {
            visible: isInput
            id: inputSlider
            from: 0.0
            value: virtualstudio ? virtualstudio.inputVolume : 0.5
            onMoved: { virtualstudio.inputVolume = value }
            to: 1.0
            enabled: !virtualstudio.inputMuted
            padding: 0

            anchors.left: parent.left
            anchors.leftMargin: 16 * virtualstudio.uiScale
            anchors.right: parent.right
            anchors.rightMargin: 16 * virtualstudio.uiScale
            anchors.verticalCenter: parent.verticalCenter

            opacity: virtualstudio.inputMuted ? 0.3 : 1

            background: Rectangle {
                x: inputSlider.leftPadding
                y: inputSlider.topPadding + inputSlider.availableHeight / 2 - height / 2
                implicitWidth: parent.width
                implicitHeight: 6
                width: inputSlider.availableWidth
                height: implicitHeight
                radius: 4
                color: sliderTrackColour

                Rectangle {
                    width: inputSlider.visualPosition * parent.width
                    height: parent.height
                    color: sliderActiveTrackColour
                    radius: 4
                }
            }

            handle: Rectangle {
                x: inputSlider.leftPadding + inputSlider.visualPosition * (inputSlider.availableWidth - width)
                y: inputSlider.topPadding + inputSlider.availableHeight / 2 - height / 2
                implicitWidth: 16 * virtualstudio.uiScale
                implicitHeight: 16 * virtualstudio.uiScale
                radius: 8 * virtualstudio.uiScale
                color: inputSlider.pressed ? sliderPressedColour : sliderColour
                border.color: buttonStroke
            }
        }
    }

    Component {
        id: outputControls

        Item {
            anchors.fill: parent
            anchors.verticalCenter: parent.verticalCenter

            Slider {
                id: outputSlider
                from: 0.0
                value: virtualstudio ? virtualstudio.outputVolume : 0.5
                onMoved: { virtualstudio.outputVolume = value }
                to: 1.0
                padding: 0

                anchors.left: parent.left
                anchors.leftMargin: 16 * virtualstudio.uiScale
                anchors.right: parent.right
                anchors.rightMargin: 16 * virtualstudio.uiScale

                background: Rectangle {
                    x: outputSlider.leftPadding
                    y: outputSlider.topPadding + outputSlider.availableHeight / 2 - height / 2
                    implicitWidth: parent.width
                    implicitHeight: 6
                    width: outputSlider.availableWidth
                    height: implicitHeight
                    radius: 4
                    color: sliderTrackColour

                    Rectangle {
                        width: outputSlider.visualPosition * parent.width
                        height: parent.height
                        color: sliderActiveTrackColour
                        radius: 4
                    }
                }

                handle: Rectangle {
                    x: outputSlider.leftPadding + outputSlider.visualPosition * (outputSlider.availableWidth - width)
                    y: outputSlider.topPadding + outputSlider.availableHeight / 2 - height / 2
                    implicitWidth: 16 * virtualstudio.uiScale
                    implicitHeight: 16 * virtualstudio.uiScale
                    radius: 8 * virtualstudio.uiScale
                    color: outputSlider.pressed ? sliderPressedColour : sliderColour
                    border.color: buttonStroke
                }
            }

            Slider {
                id: monitorSlider
                from: 0.0
                value: virtualstudio ? virtualstudio.monitorVolume : 0.5
                onMoved: { virtualstudio.monitorVolume = value }
                to: 1.0
                padding: 0

                anchors.top: outputSlider.bottom
                anchors.topMargin: 4 * virtualstudio.uiScale
                anchors.left: parent.left
                anchors.leftMargin: 16 * virtualstudio.uiScale
                anchors.right: parent.right
                anchors.rightMargin: 16 * virtualstudio.uiScale

                background: Rectangle {
                    x: monitorSlider.leftPadding
                    y: monitorSlider.topPadding + monitorSlider.availableHeight / 2 - height / 2
                    implicitWidth: parent.width
                    implicitHeight: 6
                    width: monitorSlider.availableWidth
                    height: implicitHeight
                    radius: 4
                    color: sliderTrackColour

                    Rectangle {
                        width: monitorSlider.visualPosition * parent.width
                        height: parent.height
                        color: sliderActiveTrackColour
                        radius: 4
                    }
                }

                handle: Rectangle {
                    x: monitorSlider.leftPadding + monitorSlider.visualPosition * (monitorSlider.availableWidth - width)
                    y: monitorSlider.topPadding + monitorSlider.availableHeight / 2 - height / 2
                    implicitWidth: 16 * virtualstudio.uiScale
                    implicitHeight: 16 * virtualstudio.uiScale
                    radius: 8 * virtualstudio.uiScale
                    color: monitorSlider.pressed ? sliderPressedColour : sliderColour
                    border.color: buttonStroke
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 2

        Item {
            Layout.preferredHeight: 24
            Layout.minimumHeight: 24
            Layout.maximumHeight: 24
            Layout.fillWidth: true

            RowLayout {
                anchors.fill: parent
                spacing: 8

                Item {
                    Layout.fillHeight: true
                    Layout.preferredWidth: 104

                    Loader {
                        id: typeIconIndicator
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        sourceComponent: indicatorIcon
                    }

                    Text {
                        id: label
                        anchors.left: parent.left
                        anchors.leftMargin: 36 * virtualstudio.uiScale
                        anchors.verticalCenter: parent.verticalCenter

                        text: isInput ? "Input" : "Output"
                        font { family: "Poppins"; weight: Font.Bold; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                        color: textColour
                    }

                    InfoTooltip {
                        content: isInput ? qsTr("Audio sent to the studio (microphone, instrument, mixer, etc.)") : qsTr("How you'll hear the studio audio")
                        size: 12
                        width: 12
                        height: 12
                        anchors.left: label.right
                        anchors.leftMargin: 4 * virtualstudio.uiScale
                        anchors.verticalCenter: label.verticalCenter
                    }
                }

                Item {
                    id: meters
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.preferredWidth: 200
                    Layout.topMargin: 2
                    Layout.rightMargin: 8

                    Meter {
                        anchors.fill: parent
                        height: parent.height
                        model: isInput ? virtualstudio.inputMeterLevels : virtualstudio.outputMeterLevels
                        clipped: isInput ? virtualstudio.inputClipped : virtualstudio.outputClipped
                        enabled: true
                    }
                }
            }
        }

        Item {
            Layout.preferredHeight: 42
            Layout.minimumHeight: 42
            Layout.maximumHeight: 42
            Layout.fillWidth: true

            RowLayout {
                anchors.fill: parent
                spacing: 2

                Rectangle {
                    color: "transparent"
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    Loader {
                        anchors.fill: parent
                        sourceComponent: isInput ? inputControls : outputControls
                    }
                }
            }
        }
    }
}
