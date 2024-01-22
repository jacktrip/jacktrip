import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects

Item {
    width: parent.width
    height: parent.height
    clip: true

    required property bool isInput
    property string muteButtonMutedColor: "#FCB6B6"
    property string buttonColour: virtualstudio.darkMode ? "#494646" : "#D5D9D9"

    Component {
        id: controlIndicator

        Button {
            id: iconButton
            width: 24 * virtualstudio.uiScale
            height: 24 * virtualstudio.uiScale

            anchors.left: parent.left
            anchors.leftMargin: 8 * virtualstudio.uiScale

            background: Rectangle {
                color: isInput ? (audio.inputMuted ? muteButtonMutedColor : buttonColour) : "transparent"
                width: 24 * virtualstudio.uiScale
                radius: 4 * virtualstudio.uiScale
            }

            onClicked: isInput ? audio.inputMuted = !audio.inputMuted : console.log()

            AppIcon {
                id: iconImage
                anchors.centerIn: parent
                width: 24 * virtualstudio.uiScale
                height: 24 * virtualstudio.uiScale
                icon.source: isInput ? (audio.inputMuted ? "micoff.svg" : "mic.svg") : "headphones.svg"
                color: isInput ? (audio.inputMuted ? "red" : ( virtualstudio.darkMode ? "#CCCCCC" : "#333333" )) : (virtualstudio.darkMode ? "#CCCCCC" : "#333333")
                onClicked: isInput ? audio.inputMuted = !audio.inputMuted : console.log()
            }

            ToolTip {
                visible: isInput && iconButton.hovered
                x: iconButton.x + iconButton.width
                y: iconButton.y + iconButton.height

                contentItem: Text {
                    text: audio.inputMuted ? qsTr("Click to unmute yourself") : qsTr("Click to mute yourself")
                    font { family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
                    color: textColour
                }

                background: Rectangle {
                    color: toolTipBackgroundColour
                    radius: 4
                    layer.enabled: true
                    layer.effect: Glow {
                        color: "#66000000"
                        transparentBorder: true
                    }
                }
            }
        }
    }

    Component {
        id: inputControls

        ColumnLayout {
            anchors.fill: parent
            spacing: 2 * virtualstudio.uiScale

            VolumeSlider {
                Layout.fillWidth: true
                Layout.fillHeight: true
                labelText: "Send"
                tooltipText: "How loudly other participants hear you"
                sliderEnabled: !audio.inputMuted
            }

            DeviceWarning {
                id: deviceWarning
                visible: Boolean(audio.devicesError) || Boolean(audio.devicesWarning)
            }
        }
    }

    Component {
        id: outputControls

        ColumnLayout {
            anchors.fill: parent
            spacing: 4 * virtualstudio.uiScale

            VolumeSlider {
                Layout.fillWidth: true
                Layout.fillHeight: true
                labelText: "Studio"
                tooltipText: "How loudly you hear other participants"
                sliderEnabled: true
            }

            VolumeSlider {
                Layout.fillWidth: true
                Layout.fillHeight: true
                labelText: "Monitor"
                tooltipText: "How loudly you hear yourself"
                sliderEnabled: true
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 5 * virtualstudio.uiScale

        Item {
            Layout.topMargin: 5 * virtualstudio.uiScale
            Layout.preferredHeight: 30 * virtualstudio.uiScale
            Layout.fillWidth: true

            RowLayout {
                anchors.fill: parent
                spacing: 8 * virtualstudio.uiScale

                Item {
                    Layout.fillHeight: true
                    Layout.preferredWidth: 100 * virtualstudio.uiScale

                    Loader {
                        id: typeIconIndicator
                        anchors.left: parent.left
                        sourceComponent: controlIndicator
                    }

                    Text {
                        id: label
                        anchors.left: parent.left
                        anchors.leftMargin: 36 * virtualstudio.uiScale

                        text: isInput ? "Input" : "Output"
                        font { family: "Poppins"; weight: Font.Bold; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                        color: textColour
                    }

                    InfoTooltip {
                        content: isInput ? qsTr("Audio sent to the studio (microphone, instrument, mixer, etc.)") : qsTr("How you'll hear the studio audio")
                        size: 16
                        anchors.left: label.right
                        anchors.leftMargin: 2 * virtualstudio.uiScale
                        anchors.verticalCenter: label.verticalCenter
                    }
                }

                Item {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.preferredWidth: 200 * virtualstudio.uiScale

                    Meter {
                        anchors.fill: parent
                        anchors.rightMargin: 8 * virtualstudio.uiScale
                        model: isInput ? audio.inputMeterLevels : audio.outputMeterLevels
                        clipped: isInput ? audio.inputClipped : audio.outputClipped
                        enabled: true
                    }
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.bottomMargin: 5 * virtualstudio.uiScale

            RowLayout {
                anchors.fill: parent
                spacing: 8 * virtualstudio.uiScale

                Item {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.leftMargin: 8 * virtualstudio.uiScale
                    Layout.rightMargin: 8 * virtualstudio.uiScale

                    Loader {
                        anchors.fill: parent
                        anchors.top: parent.top
                        sourceComponent: isInput ? inputControls : outputControls
                    }
                }
            }
        }
    }
}
