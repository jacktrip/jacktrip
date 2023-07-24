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
            anchors.verticalCenter: parent.verticalCenter

            background: Rectangle {
                color: isInput ? (virtualstudio.inputMuted ? muteButtonMutedColor : buttonColour) : "transparent"
                width: 24 * virtualstudio.uiScale
                radius: 4 * virtualstudio.uiScale
            }

            onClicked: isInput ? virtualstudio.inputMuted = !virtualstudio.inputMuted : console.log()

            AppIcon {
                id: iconImage
                anchors.centerIn: parent
                width: 24 * virtualstudio.uiScale
                height: 24 * virtualstudio.uiScale
                icon.source: isInput ? (virtualstudio.inputMuted ? "micoff.svg" : "mic.svg") : "headphones.svg"
                color: isInput ? (virtualstudio.inputMuted ? "red" : ( virtualstudio.darkMode ? "#CCCCCC" : "#333333" )) : (virtualstudio.darkMode ? "#CCCCCC" : "#333333")
                onClicked: isInput ? virtualstudio.inputMuted = !virtualstudio.inputMuted : console.log()
            }

            ToolTip {
                visible: isInput && iconButton.hovered
                x: iconButton.x + iconButton.width
                y: iconButton.y + iconButton.height

                contentItem: Text {
                    text: virtualstudio.inputMuted ? qsTr("Click to unmute yourself") : qsTr("Click to mute yourself")
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
            spacing: 2

            VolumeSlider {
                Layout.fillWidth: true
                Layout.fillHeight: true
                labelText: "Send"
                tooltipText: "How loudly other participants hear you"
                sliderEnabled: !virtualstudio.inputMuted
            }

            Item {
                id: warning
                Layout.fillWidth: true
                Layout.fillHeight: true

                InfoTooltip {
                    id: warningIcon
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.leftMargin: 10 * virtualstudio.uiScale
                    size: 16 * virtualstudio.uiScale
                    content: qsTr(virtualstudio.devicesError || virtualstudio.devicesWarning)
                    iconSource: "warning.svg"
                    iconColor: "red"
                }

                Text {
                    id: warningMessageText
                    color: "red"
                    font {
                        family: "Poppins"
                        pixelSize: 10 * virtualstudio.uiScale
                    }

                    elide: Text.ElideRight
                    text: (virtualstudio.devicesError || virtualstudio.devicesWarning)

                    anchors.verticalCenter: warningIcon.verticalCenter
                    anchors.left: warningIcon.right
                    anchors.leftMargin: 12 * virtualstudio.uiScale
                    anchors.right: warning.right
                    anchors.rightMargin: 12 * virtualstudio.uiScale
                }

                visible: Boolean(virtualstudio.devicesError) || Boolean(virtualstudio.devicesWarning);
            }
        }
    }

    Component {
        id: outputControls

        ColumnLayout {
            anchors.fill: parent
            spacing: 2

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
        spacing: 2

        Item {
            Layout.preferredHeight: minifiedHeight
            Layout.fillWidth: true

            RowLayout {
                anchors.fill: parent
                spacing: 8

                Item {
                    Layout.fillHeight: true
                    Layout.preferredWidth: 100

                    Loader {
                        id: typeIconIndicator
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        sourceComponent: controlIndicator
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
                        size: 16
                        anchors.left: label.right
                        anchors.leftMargin: 2 * virtualstudio.uiScale
                        anchors.verticalCenter: label.verticalCenter
                    }
                }

                Item {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.preferredWidth: 200

                    Meter {
                        anchors.fill: parent
                        anchors.topMargin: 5 * virtualstudio.uiScale
                        anchors.rightMargin: 8 * virtualstudio.uiScale
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

                Item {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignVCenter
                    Layout.leftMargin: 8 * virtualstudio.uiScale
                    Layout.rightMargin: 8 * virtualstudio.uiScale

                    Loader {
                        anchors.fill: parent
                        anchors.verticalCenter: parent.verticalCenter
                        sourceComponent: isInput ? inputControls : outputControls
                    }
                }
            }
        }
    }
}
