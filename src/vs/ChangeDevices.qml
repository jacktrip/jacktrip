import QtQuick
import QtQuick.Controls

Rectangle {
    width: parent.width; height: parent.height
    color: backgroundColour
    clip: true

    property int fontBig: 28
    property int fontMedium: 12
    property int fontSmall: 10
    property int fontTiny: 8

    property int rightMargin: 16
    property int bottomToolTipMargin: 8
    property int rightToolTipMargin: 4

    property string saveButtonText: "#000000"
    property string textColour: virtualstudio.darkMode ? "#FAFBFB" : "#0F0D0D"
    property string meterColor: virtualstudio.darkMode ? "gray" : "#E0E0E0"
    property real muteButtonLightnessValue: virtualstudio.darkMode ? 1.0 : 0.0
    property real muteButtonMutedLightnessValue: 0.24
    property real muteButtonMutedSaturationValue: 0.73
    property string buttonStroke: virtualstudio.darkMode ? "#80827D7D" : "#34979797"
    property string shadowColour: virtualstudio.darkMode ? "#40000000" : "#80A1A1A1"
    property string toolTipBackgroundColour: virtualstudio.darkMode ? "#323232" : "#F3F3F3"
    property string toolTipTextColour: textColour
    property string sliderColour: virtualstudio.darkMode ? "#BABCBC" :  "#EAECEC"
    property string sliderPressedColour: virtualstudio.darkMode ? "#ACAFAF" : "#DEE0E0"
    property string sliderTrackColour: virtualstudio.darkMode ? "#5B5858" : "light gray"
    property string sliderActiveTrackColour: virtualstudio.darkMode ? "light gray" : "black"
    property string checkboxStroke: "#0062cc"
    property string checkboxPressedStroke: "#007AFF"

    property string browserButtonColour: virtualstudio.darkMode ? "#494646" : "#EAECEC"
    property string browserButtonHoverColour: virtualstudio.darkMode ? "#5B5858" : "#D3D4D4"
    property string browserButtonPressedColour: virtualstudio.darkMode ? "#524F4F" : "#DEE0E0"
    property string browserButtonStroke: virtualstudio.darkMode ? "#80827D7D" : "#40979797"
    property string browserButtonHoverStroke: virtualstudio.darkMode ? "#7B7777" : "#BABCBC"
    property string browserButtonPressedStroke: virtualstudio.darkMode ? "#827D7D" : "#BABCBC"

    property string linkText: virtualstudio.darkMode ? "#8B8D8D" : "#272525"

    function getQueueBufferString () {
        let queueBuffer = virtualstudio.queueBuffer;
        if (useStudioQueueBuffer.checkState == Qt.Checked) {
            queueBuffer = virtualstudio.currentStudio.queueBuffer;
        }
        if (queueBuffer == 0) {
            return "auto";
        }
        return queueBuffer + " ms";
    }

    MouseArea {
        anchors.fill: parent
        propagateComposedEvents: false
    }

    Rectangle {
        id: audioSettingsView
        width: parent.width;
        color: backgroundColour
        radius: 6 * virtualstudio.uiScale

        DeviceRefreshButton {
            id: refreshButton
            anchors.top: parent.top;
            anchors.topMargin: 16 * virtualstudio.uiScale;
            anchors.right: parent.right;
            anchors.rightMargin: 16 * virtualstudio.uiScale;
            enabled: !audio.scanningDevices
            onDeviceRefresh: function () {
                virtualstudio.triggerReconnect(true);
            }
        }

        Text {
            text: "Restarting Audio"
            anchors.verticalCenter: refreshButton.verticalCenter
            anchors.right: refreshButton.left;
            anchors.rightMargin: 16 * virtualstudio.uiScale;
            font { family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            visible: audio.scanningDevices
        }

        AudioSettings {
            id: audioSettings
            showMeters: false
            showTestAudio: false
            connected: true
            height: 300 * virtualstudio.uiScale
            anchors.top: refreshButton.bottom;
            anchors.topMargin: 16 * virtualstudio.uiScale;
        }

        Rectangle {
            id: latencyDivider
            anchors.top: audioSettings.bottom
            anchors.topMargin: 54 * virtualstudio.uiScale
            x: 24 * virtualstudio.uiScale
            width: parent.width - x - (24 * virtualstudio.uiScale);
            height: 2 * virtualstudio.uiScale
            color: "#E0E0E0"
        }

        Text {
            id: queueBufferLabel
            anchors.top: latencyDivider.bottom
            anchors.topMargin: 16 * virtualstudio.uiScale
            x: 24 * virtualstudio.uiScale
            text: "Audio Quality"
            font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
        }

        InfoTooltip {
            id: queueBufferTooltip
            content: "JackTrip analyzes your Internet connection to find the best balance between audio latency and quality. Add additional latency to further improve quality."
            size: 16 * virtualstudio.uiScale
            anchors.left: queueBufferLabel.right
            anchors.bottom: queueBufferLabel.top
            anchors.bottomMargin: -8 * virtualstudio.uiScale
        }

        AppIcon {
            id: balanceIcon
            anchors.left: queueBufferLabel.left
            anchors.top: queueBufferLabel.bottom
            width: 32 * virtualstudio.uiScale
            height: 32 * virtualstudio.uiScale
            icon.source: "balance.svg"
        }

        CheckBox {
            id: useStudioQueueBuffer
            checked: virtualstudio.useStudioQueueBuffer
            text: qsTr("Use Studio settings (recommended)")
            anchors.top: latencyDivider.bottom
            anchors.topMargin: 16 * virtualstudio.uiScale
            x: 168 * virtualstudio.uiScale;
            onClicked: { virtualstudio.useStudioQueueBuffer = useStudioQueueBuffer.checkState == Qt.Checked; }
            indicator: Rectangle {
                implicitWidth: 16 * virtualstudio.uiScale
                implicitHeight: 16 * virtualstudio.uiScale
                x: useStudioQueueBuffer.leftPadding
                y: parent.height / 2 - height / 2
                radius: 3 * virtualstudio.uiScale
                border.color: useStudioQueueBuffer.down || useStudioQueueBuffer.hovered ? checkboxPressedStroke : checkboxStroke

                Rectangle {
                    width: 10 * virtualstudio.uiScale
                    height: 10 * virtualstudio.uiScale
                    x: 3 * virtualstudio.uiScale
                    y: 3 * virtualstudio.uiScale
                    radius: 2 * virtualstudio.uiScale
                    color: useStudioQueueBuffer.down || useStudioQueueBuffer.hovered ? checkboxPressedStroke : checkboxStroke
                    visible: useStudioQueueBuffer.checked
                }
            }
            contentItem: Text {
                text: useStudioQueueBuffer.text
                font.family: "Poppins"
                font.pixelSize: 10 * virtualstudio.fontScale * virtualstudio.uiScale
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                leftPadding: useStudioQueueBuffer.indicator.width + useStudioQueueBuffer.spacing
                color: textColour
            }
        }

        Text {
            id: queueBufferText
            anchors.top: latencyDivider.bottom
            anchors.topMargin: 16 * virtualstudio.uiScale
            anchors.right: parent.right
            anchors.rightMargin: 24 * virtualstudio.uiScale
            text: "Audio Quality: " + getQueueBufferString()
            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
        }

        Text {
            id: currentLatency
            anchors.top: queueBufferText.bottom
            anchors.topMargin: 6 * virtualstudio.uiScale
            anchors.right: parent.right
            anchors.rightMargin: 24 * virtualstudio.uiScale
            text: "Ingress Jitter Latency: " + Math.round(virtualstudio.networkStats.clientBufferLatency) + " ms"
            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
        }

        Button {
            id: queueBufferAutoButton
            width: 60 * virtualstudio.uiScale
            height: 30 * virtualstudio.uiScale
            anchors.top: useStudioQueueBuffer.bottom
            anchors.topMargin: 16 * virtualstudio.uiScale
            anchors.left: useStudioQueueBuffer.left
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: virtualstudio.queueBuffer == 0 ? (virtualstudio.darkMode ? "#FFFFFF" : "#000000") : browserButtonColour
            }
            onClicked: {
                virtualstudio.queueBuffer = virtualstudio.queueBuffer == 0 ? 5 : 0;
            }
            Text {
                text: "Auto"
                font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale}
                anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
                color: virtualstudio.queueBuffer == 0 ? (virtualstudio.darkMode ? "#000000" : "#FFFFFF") : textColour
            }

            visible: useStudioQueueBuffer.checkState != Qt.Checked
        }

        Slider {
            id: queueBufferSlider
            value: virtualstudio.queueBuffer
            onMoved: {
                virtualstudio.queueBuffer = value;
            }
            from: 1
            to: 250
            stepSize: 1
            padding: 0
            visible: useStudioQueueBuffer.checkState != Qt.Checked
            enabled: virtualstudio.queueBuffer != 0

            anchors.top: useStudioQueueBuffer.bottom
            anchors.topMargin: 16 * virtualstudio.uiScale
            anchors.left: queueBufferAutoButton.right
            anchors.leftMargin: 8 * virtualstudio.uiScale
            width: parent.width - x - (16 * virtualstudio.uiScale);

            background: Rectangle {
                x: queueBufferSlider.leftPadding
                y: queueBufferSlider.topPadding + queueBufferSlider.availableHeight / 2 - height / 2
                implicitWidth: parent.width
                implicitHeight: 6
                width: queueBufferSlider.availableWidth
                height: implicitHeight
                radius: 4
                color: sliderTrackColour

                Rectangle {
                    width: queueBufferSlider.visualPosition * parent.width
                    height: parent.height
                    color: sliderActiveTrackColour
                    radius: 4
                }
            }

            handle: Rectangle {
                x: queueBufferSlider.leftPadding + queueBufferSlider.visualPosition * (queueBufferSlider.availableWidth - width)
                y: queueBufferSlider.topPadding + queueBufferSlider.availableHeight / 2 - height / 2
                implicitWidth: 26 * virtualstudio.uiScale
                implicitHeight: 26 * virtualstudio.uiScale
                radius: 13 * virtualstudio.uiScale
                color: queueBufferSlider.pressed ? sliderPressedColour : sliderColour
                border.color: buttonStroke
                visible: virtualstudio.queueBuffer != 0
            }
        }

        Text {
            id: lowerLatencyText
            anchors.top: queueBufferSlider.bottom
            anchors.topMargin: 8 * virtualstudio.uiScale
            anchors.left: queueBufferSlider.left
            anchors.leftMargin: 8 * virtualstudio.uiScale
            text: "Lower Latency"
            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            visible: useStudioQueueBuffer.checkState != Qt.Checked
        }

        Text {
            id: higherQualityText
            anchors.top: queueBufferSlider.bottom
            anchors.topMargin: 8 * virtualstudio.uiScale
            anchors.right: queueBufferSlider.right
            anchors.rightMargin: 8 * virtualstudio.uiScale
            text: "Higher Quality"
            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            visible: useStudioQueueBuffer.checkState != Qt.Checked
        }
    }

    Button {
        id: backButton
        background: Rectangle {
            radius: 6 * virtualstudio.uiScale
            color: backButton.down ? browserButtonPressedColour : (backButton.hovered ? browserButtonHoverColour : browserButtonColour)
        }
        onClicked: {
            virtualstudio.saveSettings();
            virtualstudio.windowState = "connected";
        }
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 16 * virtualstudio.uiScale;
        anchors.left: parent.left
        anchors.leftMargin: 16 * virtualstudio.uiScale;
        width: 150 * virtualstudio.uiScale; height: 36 * virtualstudio.uiScale

        Text {
            text: "Back"
            font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale}
            anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
            color: textColour
        }
    }

    DeviceWarning {
        id: deviceWarning
        anchors.left: backButton.right
        anchors.leftMargin: 24 * virtualstudio.uiScale
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 16 * virtualstudio.uiScale;
        visible: Boolean(audio.devicesError) || Boolean(audio.devicesWarning)
    }
}
