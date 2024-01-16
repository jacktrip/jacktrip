import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    required property bool showMinified

    property string disabledButtonText: "#D3D4D4"
    property string saveButtonText: "#DB0A0A"
    property int fullHeight: 88 * virtualstudio.uiScale

    id: deviceControlsGroup
    width: parent.width
    height: showMinified && showDeviceControls ? 0 : fullHeight
    color: backgroundColour

    property bool showDeviceControls: studioStatus === "Ready"
    property bool isUsingRtAudio: audio.audioBackend == "RtAudio"

    MouseArea {
        anchors.fill: parent
        propagateComposedEvents: false
    }

    RowLayout {
        id: layout
        anchors.fill: parent
        spacing: 2
        visible: !feedbackDetectedModal.visible

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
            visible: !showDeviceControls

            Button {
                id: backButton
                anchors.centerIn: parent
                width: 180 * virtualstudio.uiScale
                height: 36 * virtualstudio.uiScale
                background: Rectangle {
                    radius: 8 * virtualstudio.uiScale
                    color: backButton.down ? browserButtonPressedColour : (backButton.hovered ? browserButtonHoverColour : browserButtonColour)
                }
                onClicked: virtualstudio.disconnect()

                Text {
                    text: "Back to Studios"
                    font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale}
                    anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
                    color: textColour
                }
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
            visible: showDeviceControls

            DeviceControls {
                isInput: true
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
            visible: showDeviceControls

            DeviceControls {
                isInput: false
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.preferredWidth: 48 * virtualstudio.uiScale
            visible: showDeviceControls && isUsingRtAudio

            ColumnLayout {
                anchors.fill: parent
                spacing: 2

                Item {
                    Layout.preferredHeight: 24 * virtualstudio.uiScale
                    Layout.preferredWidth: 24 * virtualstudio.uiScale
                    Layout.topMargin: 2 * virtualstudio.uiScale
                    Layout.rightMargin: 2 * virtualstudio.uiScale
                    Layout.alignment: Qt.AlignRight | Qt.AlignTop

                    Button {
                        id: closeDeviceControlsButton
                        width: 24 * virtualstudio.uiScale
                        height: 24 * virtualstudio.uiScale
                        background: Rectangle {
                            color: backgroundColour
                        }
                        anchors.top: parent.top
                        anchors.right: parent.right
                        onClicked: {
                            virtualstudio.collapseDeviceControls = true;
                        }

                        AppIcon {
                            id: closeDeviceControlsIcon
                            anchors { verticalCenter: parent.verticalCenter; horizontalCenter: parent.horizontalCenter }
                            width: 24 * virtualstudio.uiScale
                            height: 24 * virtualstudio.uiScale
                            color: closeDeviceControlsButton.hovered ? textColour : browserButtonHoverColour
                            icon.source: "close.svg"
                            onClicked: {
                                virtualstudio.collapseDeviceControls = true;
                            }
                        }
                    }
                }

                Item {
                    Layout.preferredWidth: 40 * virtualstudio.uiScale
                    Layout.preferredHeight: 64 * virtualstudio.uiScale
                    Layout.bottomMargin: 5 * virtualstudio.uiScale
                    Layout.topMargin: 2 * virtualstudio.uiScale
                    Layout.rightMargin: 2 * virtualstudio.uiScale
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignTop

                    Button {
                        id: changeDevicesButton
                        width: 36 * virtualstudio.uiScale
                        height: 36 * virtualstudio.uiScale
                        anchors.top: parent.top
                        anchors.horizontalCenter: parent.horizontalCenter
                        background: Rectangle {
                            radius: 8 * virtualstudio.uiScale
                            color: changeDevicesButton.down ? browserButtonPressedColour : (changeDevicesButton.hovered ? browserButtonHoverColour : browserButtonColour)
                        }
                        onClicked: {
                            virtualstudio.windowState = "change_devices"
                            if (!audio.deviceModelsInitialized) {
                                audio.refreshDevices();
                            }
                        }

                        AppIcon {
                            id: changeDevicesIcon
                            anchors { verticalCenter: parent.verticalCenter; horizontalCenter: parent.horizontalCenter }
                            width: 20 * virtualstudio.uiScale
                            height: 20 * virtualstudio.uiScale
                            icon.source: "cog.svg"
                            onClicked: {
                                virtualstudio.windowState = "change_devices"
                                if (!audio.deviceModelsInitialized) {
                                    audio.refreshDevices();
                                }
                            }
                        }
                    }

                    Text {
                        anchors.top: changeDevicesButton.bottom
                        text: "Devices"
                        font { family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale}
                        anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
                        color: textColour
                    }
                }
            }
        }
    }

    Rectangle {
        id: backgroundBorder
        width: parent.width
        height: 1
        anchors.top: layout.top
        color: strokeColor
    }

    Popup {
        id: feedbackDetectedModal
        padding: 1
        width: parent.width
        height: parent.height
        anchors.centerIn: parent
        dim: false
        modal: false
        focus: true
        closePolicy: Popup.NoAutoClose

        background: Rectangle {
            anchors.fill: parent
            color: "transparent"
            border.width: 1
            border.color: buttonStroke
            clip: true
        }

        contentItem: Rectangle {
            width: parent.width
            height: 232 * virtualstudio.uiScale
            color: backgroundColour

            Item {
                id: feedbackDetectedContent
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.leftMargin: 16 * virtualstudio.uiScale
                anchors.right: parent.right

                AppIcon {
                    id: feedbackWarningIcon
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.topMargin: 10 * virtualstudio.uiScale
                    width: 32 * virtualstudio.uiScale
                    height: 32 * virtualstudio.uiScale
                    icon.source: "warning.svg"
                    color: "#F21B1B"
                    visible: !showMinified
                }

                AppIcon {
                    id: feedbackWarningIconMinified
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    height: 24 * virtualstudio.uiScale
                    width: 24 * virtualstudio.uiScale
                    icon.source: "warning.svg"
                    color: "#F21B1B"
                    visible: showMinified
                }

                Text {
                    id: feedbackDetectedHeader
                    anchors.top: parent.top
                    anchors.topMargin: 10 * virtualstudio.uiScale
                    anchors.left: feedbackWarningIcon.right
                    anchors.leftMargin: 16 * virtualstudio.uiScale
                    width: parent.width
                    text: "Audio feedback detected!"
                    font {family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale; bold: true }
                    color: textColour
                    elide: Text.ElideRight
                    wrapMode: Text.WordWrap
                    visible: !showMinified
                }

                Text {
                    id: feedbackDetectedText
                    anchors.top: feedbackDetectedHeader.bottom
                    anchors.topMargin: 4 * virtualstudio.uiScale
                    anchors.left: feedbackWarningIcon.right
                    anchors.leftMargin: 16 * virtualstudio.uiScale
                    width: parent.width
                    text: "JackTrip detected a feedback loop. Your monitor and input volume have automatically been disabled."
                    font {family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
                    color: textColour
                    elide: Text.ElideRight
                    wrapMode: Text.WordWrap
                    visible: !showMinified
                }

                Text {
                    id: feedbackDetectedTextMinified
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: feedbackWarningIcon.right
                    anchors.leftMargin: 16 * virtualstudio.uiScale
                    width: parent.width
                    text: "JackTrip detected a feedback loop. Your monitor and input volume have automatically been disabled."
                    font {family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
                    color: textColour
                    elide: Text.ElideRight
                    wrapMode: Text.WordWrap
                    visible: showMinified
                }

                Text {
                    id: feedbackDetectedText2
                    anchors.top: feedbackDetectedText.bottom
                    anchors.topMargin: 2 * virtualstudio.uiScale
                    anchors.left: feedbackWarningIcon.right
                    anchors.leftMargin: 16 * virtualstudio.uiScale
                    width: parent.width
                    text: "You can disable this behavior under <b>Settings</b> > <b>Advanced</b>"
                    textFormat: Text.RichText
                    font {family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
                    color: textColour
                    elide: Text.ElideRight
                    wrapMode: Text.WordWrap
                    visible: !showMinified
                }

                Button {
                    id: closeFeedbackDetectedModalButton
                    anchors.right: parent.right
                    anchors.rightMargin: rightMargin * virtualstudio.uiScale
                    anchors.verticalCenter: parent.verticalCenter
                    width: 128 * virtualstudio.uiScale;
                    height: 30 * virtualstudio.uiScale
                    onClicked: feedbackDetectedModal.close()

                    background: Rectangle {
                        radius: 6 * virtualstudio.uiScale
                        color: closeFeedbackDetectedModalButton.down ? browserButtonPressedColour : (closeFeedbackDetectedModalButton.hovered ? browserButtonHoverColour : browserButtonColour)
                        border.width: 1
                        border.color: closeFeedbackDetectedModalButton.down ? browserButtonPressedStroke : (closeFeedbackDetectedModalButton.hovered ? browserButtonHoverStroke : browserButtonStroke)
                    }

                    Text {
                        text: "Ok"
                        font.family: "Poppins"
                        font.pixelSize: showMinified ? fontTiny * virtualstudio.fontScale * virtualstudio.uiScale : fontSmall * virtualstudio.fontScale * virtualstudio.uiScale
                        font.weight: Font.Bold
                        color: !Boolean(audio.devicesError) && audio.backendAvailable ? saveButtonText : disabledButtonText
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    visible: !showMinified
                }

                Button {
                    id: closeFeedbackDetectedModalButtonMinified
                    anchors.right: parent.right
                    anchors.rightMargin: rightMargin * virtualstudio.uiScale
                    anchors.verticalCenter: parent.verticalCenter
                    width: 80 * virtualstudio.uiScale
                    height: 24 * virtualstudio.uiScale
                    onClicked: feedbackDetectedModal.close()

                    background: Rectangle {
                        radius: 6 * virtualstudio.uiScale
                        color: closeFeedbackDetectedModalButton.down ? browserButtonPressedColour : (closeFeedbackDetectedModalButton.hovered ? browserButtonHoverColour : browserButtonColour)
                        border.width: 1
                        border.color: closeFeedbackDetectedModalButton.down ? browserButtonPressedStroke : (closeFeedbackDetectedModalButton.hovered ? browserButtonHoverStroke : browserButtonStroke)
                    }

                    Text {
                        text: "Ok"
                        font.family: "Poppins"
                        font.pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale
                        font.weight: Font.Bold
                        color: !Boolean(audio.devicesError) && audio.backendAvailable ? saveButtonText : disabledButtonText
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    visible: showMinified
                }
            }
        }
    }

    Connections {
        target: virtualstudio

        function onFeedbackDetected() {
            if (virtualstudio.windowState === "connected") {
                feedbackDetectedModal.visible = true;
            }
        }
    }
}