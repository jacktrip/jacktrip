import QtQuick
import QtQuick.Controls

Rectangle {
    width: parent.width
    height: parent.height
    color: backgroundColour

    property int fontBig: 20
    property int fontMedium: 13
    property int fontSmall: 11
    property int fontExtraSmall: 8

    property int leftMargin: 48
    property int rightMargin: 16
    property int bottomToolTipMargin: 8
    property int rightToolTipMargin: 4
    property int buttonWidth: 103
    property int buttonHeight: 25

    property string backgroundColour: virtualstudio.darkMode ? "#272525" : "#FAFBFB"
    property string textColour: virtualstudio.darkMode ? "#FAFBFB" : "#0F0D0D"
    property string buttonColour: virtualstudio.darkMode ? "#494646" : "#EAECEC"
    property string buttonHoverColour: virtualstudio.darkMode ? "#5B5858" : "#D3D4D4"
    property string buttonPressedColour: virtualstudio.darkMode ? "#524F4F" : "#DEE0E0"
    property string buttonStroke: virtualstudio.darkMode ? "#80827D7D" : "#40979797"
    property string buttonHoverStroke: virtualstudio.darkMode ? "#7B7777" : "#BABCBC"
    property string buttonPressedStroke: virtualstudio.darkMode ? "#827D7D" : "#BABCBC"
    property string warningTextColour: "#DB0A0A"
    property string linkText: virtualstudio.darkMode ? "#8B8D8D" : "#272525"
    property string toolTipBackgroundColour: virtualstudio.darkMode ? "#323232" : "#F3F3F3"
    property string toolTipTextColour: textColour

    property string errorFlagColour: "#DB0A0A"
    property string disabledButtonTextColour: virtualstudio.darkMode ? "#827D7D" : "#BABCBC"

    property bool isUsingJack: virtualstudio.audioBackend == "JACK"
    property bool isUsingRtAudio: virtualstudio.audioBackend == "RtAudio"
    property bool hasNoBackend: !isUsingJack && !isUsingRtAudio && !virtualstudio.backendAvailable;

    function getCurrentInputDeviceIndex () {
        if (virtualstudio.inputDevice === "") {
            return virtualstudio.inputComboModel.findIndex(elem => elem.type === "element");
        }

        let idx = virtualstudio.inputComboModel.findIndex(elem => elem.type === "element" && elem.text === virtualstudio.inputDevice);
        if (idx < 0) {
            idx = virtualstudio.inputComboModel.findIndex(elem => elem.type === "element");
        }
        return idx;
    }

    function getCurrentOutputDeviceIndex() {
        if (virtualstudio.outputDevice === "") {
            return virtualstudio.outputComboModel.findIndex(elem => elem.type === "element");
        }

        let idx = virtualstudio.outputComboModel.findIndex(elem => elem.type === "element" && elem.text === virtualstudio.outputDevice);
        if (idx < 0) {
            idx = virtualstudio.outputComboModel.findIndex(elem => elem.type === "element");
        }
        return idx;
    }

    Loader {
        anchors.fill: parent
        sourceComponent: Boolean(audioInterface) ? (isUsingRtAudio ? usingRtAudio : (isUsingJack ? usingJACK : noBackend)) : noBackend
    }

    Component {
        id: usingRtAudio

        Item {
            anchors.top: parent.top
            anchors.topMargin: 24 * virtualstudio.uiScale
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.leftMargin: 24 * virtualstudio.uiScale
            anchors.right: parent.right

            Rectangle {
                id: leftSpacer
                x: 0; y: 0
                width: 144 * virtualstudio.uiScale
                height: 0
                color: "transparent"
            }

            Text {
                id: outputLabel
                x: 0; y: 0
                text: "Output Device"
                font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                color: textColour
            }

            InfoTooltip {
                id: outputHelpIcon
                anchors.left: outputLabel.right
                anchors.bottom: outputLabel.top
                anchors.bottomMargin: -8 * virtualstudio.uiScale
                size: 16 * virtualstudio.uiScale
                content: qsTr("How you'll hear the studio audio")
            }

            AppIcon {
                id: headphonesIcon
                anchors.left: outputLabel.left
                anchors.verticalCenter: outputDeviceMeters.verticalCenter
                width: 28 * virtualstudio.uiScale
                height: 28 * virtualstudio.uiScale
                icon.source: "headphones.svg"
            }

            ComboBox {
                id: outputCombo
                anchors.left: leftSpacer.right
                anchors.verticalCenter: outputLabel.verticalCenter
                anchors.rightMargin: rightMargin * virtualstudio.uiScale
                width: parent.width - leftSpacer.width - rightMargin * virtualstudio.uiScale
                model: virtualstudio.outputComboModel
                currentIndex: getCurrentOutputDeviceIndex()
                delegate: ItemDelegate {
                    required property var modelData
                    required property int index

                    leftPadding: 0

                    width: parent.width
                    contentItem: Text {
                        leftPadding: modelData.type === "element" && outputCombo.model.filter(it => it.type === "header").length > 0 ? 24 : 12
                        text: modelData.text
                        font.bold: modelData.type === "header"
                    }
                    highlighted: outputCombo.highlightedIndex === index
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (modelData.type == "element") {
                                outputCombo.currentIndex = index
                                outputCombo.popup.close()
                                virtualstudio.outputDevice = modelData.text
                                if (modelData.category === "Low-Latency (ASIO)") {
                                    let inputComboIdx = inputCombo.model.findIndex(it => it.category === "Low-Latency (ASIO)" && it.text === modelData.text);
                                    if (inputComboIdx !== null && inputComboIdx !== undefined) {
                                        inputCombo.currentIndex = inputComboIdx;
                                        virtualstudio.inputDevice = modelData.text
                                    }
                                }
                                virtualstudio.restartAudio()
                            }
                        }
                    }
                }
                contentItem: Text {
                    leftPadding: 12
                    font: outputCombo.font
                    horizontalAlignment: Text.AlignHLeft
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                    text: outputCombo.model[outputCombo.currentIndex]!=undefined && outputCombo.model[outputCombo.currentIndex].text ? outputCombo.model[outputCombo.currentIndex].text : ""
                }
            }

            Meter {
                id: outputDeviceMeters
                anchors.left: outputCombo.left
                anchors.right: outputCombo.right
                anchors.top: outputCombo.bottom
                anchors.topMargin: 16 * virtualstudio.uiScale
                height: 24 * virtualstudio.uiScale
                model: virtualstudio.outputMeterLevels
                clipped: virtualstudio.outputClipped
                enabled: virtualstudio.audioReady && !Boolean(virtualstudio.devicesError)
            }

            VolumeSlider {
                id: outputSlider
                anchors.left: outputCombo.left
                anchors.right: parent.right
                anchors.rightMargin: rightMargin * virtualstudio.uiScale
                anchors.top: outputDeviceMeters.bottom
                anchors.topMargin: 16 * virtualstudio.uiScale
                height: 30 * virtualstudio.uiScale
                labelText: "Studio"
                tooltipText: "How loudly you hear other participants"
                showLabel: false
                sliderEnabled: true
            }

            Text {
                id: outputChannelsLabel
                anchors.left: outputCombo.left
                anchors.right: outputCombo.horizontalCenter
                anchors.top: outputSlider.bottom
                anchors.topMargin: 12 * virtualstudio.uiScale
                textFormat: Text.RichText
                text: "Output Channel(s)"
                font { family: "Poppins"; pixelSize: fontExtraSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                color: textColour
            }

            ComboBox {
                id: outputChannelsCombo
                anchors.left: outputCombo.left
                anchors.right: outputCombo.horizontalCenter
                anchors.rightMargin: 8 * virtualstudio.uiScale
                anchors.top: outputChannelsLabel.bottom
                anchors.topMargin: 4 * virtualstudio.uiScale
                model: virtualstudio.outputChannelsComboModel
                currentIndex: (() => {
                    let idx = virtualstudio.outputChannelsComboModel.findIndex(elem => elem.baseChannel === virtualstudio.baseOutputChannel
                        && elem.numChannels === virtualstudio.numOutputChannels);
                    if (idx < 0) {
                        idx = 0;
                    }
                    return idx;
                })()
                delegate: ItemDelegate {
                    required property var modelData
                    required property int index
                    width: parent.width
                    contentItem: Text {
                        text: modelData.label
                    }
                    highlighted: outputChannelsCombo.highlightedIndex === index
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            outputChannelsCombo.currentIndex = index
                            outputChannelsCombo.popup.close()
                            virtualstudio.baseOutputChannel = modelData.baseChannel
                            virtualstudio.numOutputChannels = modelData.numChannels
                            virtualstudio.validateDevicesState()
                        }
                    }
                }
                contentItem: Text {
                    leftPadding: 12
                    font: inputCombo.font
                    horizontalAlignment: Text.AlignHLeft
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                    text: outputChannelsCombo.model[outputChannelsCombo.currentIndex].label || ""
                }
            }

            Button {
                id: testOutputAudioButton
                background: Rectangle {
                    radius: 6 * virtualstudio.uiScale
                    color: testOutputAudioButton.down ? buttonPressedColour : (testOutputAudioButton.hovered ? buttonHoverColour : buttonColour)
                    border.width: 1
                    border.color: testOutputAudioButton.down || testOutputAudioButton.hovered ? buttonPressedStroke : (testOutputAudioButton.hovered ? buttonHoverStroke : buttonStroke)
                }
                onClicked: { virtualstudio.playOutputAudio() }
                anchors.right: parent.right
                anchors.rightMargin: rightMargin * virtualstudio.uiScale
                anchors.verticalCenter: outputChannelsCombo.verticalCenter
                width: 144 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
                Text {
                    text: "Play Test Tone"
                    font { family: "Poppins"; pixelSize: fontExtraSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                    anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
                    color: textColour
                }
            }

            Rectangle {
                id: divider1
                anchors.top: testOutputAudioButton.bottom
                anchors.topMargin: 24 * virtualstudio.uiScale
                width: parent.width - x - (16 * virtualstudio.uiScale); height: 2 * virtualstudio.uiScale
                color: "#E0E0E0"
            }

            Text {
                id: inputLabel
                anchors.left: outputLabel.left
                anchors.top: divider1.bottom
                anchors.topMargin: 32 * virtualstudio.uiScale
                text: "Input Device"
                font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                color: textColour
            }

            InfoTooltip {
                id: inputHelpIcon
                anchors.left: inputLabel.right
                anchors.bottom: inputLabel.top
                anchors.bottomMargin: -8 * virtualstudio.uiScale
                size: 16 * virtualstudio.uiScale
                content: qsTr("Audio sent to the studio (microphone, instrument, mixer, etc.)")
            }

            AppIcon {
                id: microphoneIcon
                anchors.left: outputLabel.left
                anchors.verticalCenter: inputDeviceMeters.verticalCenter
                width: 32 * virtualstudio.uiScale
                height: 32 * virtualstudio.uiScale
                icon.source: "mic.svg"
            }

            ComboBox {
                id: inputCombo
                model: virtualstudio.inputComboModel
                currentIndex: getCurrentInputDeviceIndex()
                anchors.left: outputCombo.left
                anchors.right: outputCombo.right
                anchors.verticalCenter: inputLabel.verticalCenter
                delegate: ItemDelegate {
                    required property var modelData
                    required property int index

                    leftPadding: 0

                    width: parent.width
                    contentItem: Text {
                        leftPadding: modelData.type === "element" && inputCombo.model.filter(it => it.type === "header").length > 0 ? 24 : 12
                        text: modelData.text
                        font.bold: modelData.type === "header"
                    }
                    highlighted: inputCombo.highlightedIndex === index
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (modelData.type == "element") {
                                inputCombo.currentIndex = index
                                inputCombo.popup.close()
                                virtualstudio.inputDevice = modelData.text
                                if (modelData.category === "Low-Latency (ASIO)") {
                                    let outputComboIdx = outputCombo.model.findIndex(it => it.category === "Low-Latency (ASIO)" && it.text === modelData.text);
                                    if (outputComboIdx !== null && outputComboIdx !== undefined) {
                                        outputCombo.currentIndex = outputComboIdx;
                                        virtualstudio.outputDevice = modelData.text
                                    }
                                }
                                virtualstudio.restartAudio()
                            }
                        }
                    }
                }
                contentItem: Text {
                    leftPadding: 12
                    font: inputCombo.font
                    horizontalAlignment: Text.AlignHLeft
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                    text: inputCombo.model[inputCombo.currentIndex] != undefined && inputCombo.model[inputCombo.currentIndex].text ? inputCombo.model[inputCombo.currentIndex].text : ""
                }
            }

            Meter {
                id: inputDeviceMeters
                anchors.left: inputCombo.left
                anchors.right: inputCombo.right
                anchors.top: inputCombo.bottom
                anchors.topMargin: 16 * virtualstudio.uiScale
                height: 24 * virtualstudio.uiScale
                model: virtualstudio.inputMeterLevels
                clipped: virtualstudio.inputClipped
                enabled: virtualstudio.audioReady && !Boolean(virtualstudio.devicesError)
            }

            VolumeSlider {
                id: inputSlider
                anchors.left: inputDeviceMeters.left
                anchors.right: parent.right
                anchors.rightMargin: rightMargin * virtualstudio.uiScale
                anchors.top: inputDeviceMeters.bottom
                anchors.topMargin: 16 * virtualstudio.uiScale
                height: 30 * virtualstudio.uiScale
                labelText: "Send"
                tooltipText: "How loudly other participants hear you"
                showLabel: false
                sliderEnabled: true
            }

            Button {
                id: hiddenInputButton
                anchors.right: parent.right
                anchors.rightMargin: rightMargin * virtualstudio.uiScale
                anchors.verticalCenter: inputSlider.verticalCenter
                width: 144 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
                visible: false
            }

            Text {
                id: inputChannelsLabel
                anchors.left: inputCombo.left
                anchors.right: inputCombo.horizontalCenter
                anchors.top: inputSlider.bottom
                anchors.topMargin: 12 * virtualstudio.uiScale
                textFormat: Text.RichText
                text: "Input Channel(s)"
                font { family: "Poppins"; pixelSize: fontExtraSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                color: textColour
            }

            ComboBox {
                id: inputChannelsCombo
                anchors.left: inputCombo.left
                anchors.right: inputCombo.horizontalCenter
                anchors.rightMargin: 8 * virtualstudio.uiScale
                anchors.top: inputChannelsLabel.bottom
                anchors.topMargin: 4 * virtualstudio.uiScale
                model: virtualstudio.inputChannelsComboModel
                currentIndex: (() => {
                    let idx = virtualstudio.inputChannelsComboModel.findIndex(elem => elem.baseChannel === virtualstudio.baseInputChannel
                        && elem.numChannels === virtualstudio.numInputChannels);
                    if (idx < 0) {
                        idx = 0;
                    }
                    return idx;
                })()
                delegate: ItemDelegate {
                    required property var modelData
                    required property int index
                    width: parent.width
                    contentItem: Text {
                        text: modelData.label
                    }
                    highlighted: inputChannelsCombo.highlightedIndex === index
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            inputChannelsCombo.currentIndex = index
                            inputChannelsCombo.popup.close()
                            virtualstudio.baseInputChannel = modelData.baseChannel
                            virtualstudio.numInputChannels = modelData.numChannels
                            virtualstudio.validateDevicesState()
                        }
                    }
                }
                contentItem: Text {
                    leftPadding: 12
                    font: inputCombo.font
                    horizontalAlignment: Text.AlignHLeft
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                    text: inputChannelsCombo.model[inputChannelsCombo.currentIndex].label || ""
                }
            }

            Text {
                id: inputMixModeLabel
                anchors.left: inputCombo.horizontalCenter
                anchors.right: inputCombo.right
                anchors.rightMargin: 8 * virtualstudio.uiScale
                anchors.top: inputSlider.bottom
                anchors.topMargin: 12 * virtualstudio.uiScale
                textFormat: Text.RichText
                text: "Mono / Stereo"
                font { family: "Poppins"; pixelSize: fontExtraSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                color: textColour
            }

            ComboBox {
                id: inputMixModeCombo
                anchors.left: inputCombo.horizontalCenter
                anchors.right: inputCombo.right
                anchors.rightMargin: 8 * virtualstudio.uiScale
                anchors.top: inputMixModeLabel.bottom
                anchors.topMargin: 4 * virtualstudio.uiScale
                model: virtualstudio.inputMixModeComboModel
                currentIndex: (() => {
                    let idx = virtualstudio.inputMixModeComboModel.findIndex(elem => elem.value === virtualstudio.inputMixMode);
                    if (idx < 0) {
                        idx = 0;
                    }
                    return idx;
                })()
                delegate: ItemDelegate {
                    required property var modelData
                    required property int index
                    width: parent.width
                    contentItem: Text {
                        text: modelData.label
                    }
                    highlighted: inputMixModeCombo.highlightedIndex === index
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            inputMixModeCombo.currentIndex = index
                            inputMixModeCombo.popup.close()
                            virtualstudio.inputMixMode = virtualstudio.inputMixModeComboModel[index].value
                            virtualstudio.validateDevicesState()
                        }
                    }
                }
                contentItem: Text {
                    leftPadding: 12
                    font: inputCombo.font
                    horizontalAlignment: Text.AlignHLeft
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                    text: inputMixModeCombo.model[inputMixModeCombo.currentIndex].label || ""
                }
            }

            Text {
                id: inputChannelHelpMessage
                anchors.left: inputChannelsCombo.left
                anchors.leftMargin: 2 * virtualstudio.uiScale
                anchors.right: inputChannelsCombo.right
                anchors.top: inputChannelsCombo.bottom
                anchors.topMargin: 8 * virtualstudio.uiScale
                textFormat: Text.RichText
                wrapMode: Text.WordWrap
                text: "Choose up to 2 channels"
                font { family: "Poppins"; pixelSize: fontExtraSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                color: textColour
            }

            Text {
                id: inputMixModeHelpMessage
                anchors.left: inputMixModeCombo.left
                anchors.leftMargin: 2 * virtualstudio.uiScale
                anchors.right: inputMixModeCombo.right
                anchors.top: inputMixModeCombo.bottom
                anchors.topMargin: 8 * virtualstudio.uiScale
                textFormat: Text.RichText
                wrapMode: Text.WordWrap
                text: (() => {
                    if (virtualstudio.inputMixMode === 2) {
                        return "Treat the channels as Left and Right signals, coming through each speaker separately.";
                    } else if (virtualstudio.inputMixMode === 3) {
                        return "Combine the channels into one central channel coming through both speakers.";
                    } else if (virtualstudio.inputMixMode === 1) {
                        return "Send a single channel of audio";
                    } else {
                        return "";
                    }
                })()
                font { family: "Poppins"; pixelSize: fontExtraSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                color: textColour
            }
        }
    }

    Component {
        id: usingJACK

        Item {
            anchors.top: parent.top
            anchors.topMargin: 24 * virtualstudio.uiScale
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.leftMargin: leftMargin * virtualstudio.uiScale
            anchors.right: parent.right

            Text {
                id: jackLabel
                x: 0; y: 0
                width: parent.width - rightMargin * virtualstudio.uiScale
                text: "Using JACK for audio input and output. Use QjackCtl to adjust your sample rate, buffer, and device settings."
                font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                wrapMode: Text.WordWrap
                color: textColour
            }

            Text {
                id: jackOutputLabel
                anchors.left: jackLabel.left
                anchors.top: jackLabel.bottom
                anchors.topMargin: 48 * virtualstudio.uiScale
                width: 144 * virtualstudio.uiScale
                text: "Output Volume"
                font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                wrapMode: Text.WordWrap
                color: textColour
            }

            AppIcon {
                id: jackHeadphonesIcon
                anchors.left: jackOutputLabel.left
                anchors.verticalCenter: jackOutputVolumeSlider.verticalCenter
                width: 28 * virtualstudio.uiScale
                height: 28 * virtualstudio.uiScale
                icon.source: "headphones.svg"
            }

            Meter {
                id: jackOutputMeters
                anchors.left: jackOutputLabel.right
                anchors.right: parent.right
                anchors.rightMargin: rightMargin * virtualstudio.uiScale
                anchors.verticalCenter: jackOutputLabel.verticalCenter
                height: 24 * virtualstudio.uiScale
                model: virtualstudio.outputMeterLevels
                clipped: virtualstudio.outputClipped
                enabled: virtualstudio.audioReady && !Boolean(virtualstudio.devicesError)
            }

            Button {
                id: jackTestOutputAudioButton
                background: Rectangle {
                    radius: 6 * virtualstudio.uiScale
                    color: jackTestOutputAudioButton.down ? buttonPressedColour : (jackTestOutputAudioButton.hovered ? buttonHoverColour : buttonColour)
                    border.width: 1
                    border.color: jackTestOutputAudioButton.down ? buttonPressedStroke : (jackTestOutputAudioButton.hovered ? buttonHoverStroke : buttonStroke)
                }
                onClicked: { virtualstudio.playOutputAudio() }
                anchors.right: parent.right
                anchors.rightMargin: rightMargin * virtualstudio.uiScale
                anchors.verticalCenter: jackOutputVolumeSlider.verticalCenter
                width: 144 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
                Text {
                    text: "Play Test Tone"
                    font { family: "Poppins"; pixelSize: fontExtraSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                    anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
                    color: textColour
                }
            }

            VolumeSlider {
                id: jackOutputVolumeSlider
                anchors.left: jackOutputMeters.left
                anchors.right: jackTestOutputAudioButton.left
                anchors.rightMargin: rightMargin * virtualstudio.uiScale
                anchors.top: jackOutputMeters.bottom
                anchors.topMargin: 16 * virtualstudio.uiScale
                height: 30 * virtualstudio.uiScale
                labelText: "Studio"
                tooltipText: "How loudly you hear other participants"
                showLabel: false
                sliderEnabled: true
            }

            Text {
                id: jackInputLabel
                anchors.left: jackLabel.left
                anchors.top: jackOutputVolumeSlider.bottom
                anchors.topMargin: 48 * virtualstudio.uiScale
                width: 144 * virtualstudio.uiScale
                text: "Input Volume"
                font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                wrapMode: Text.WordWrap
                color: textColour
            }

            AppIcon {
                id: jackMicrophoneIcon
                anchors.left: jackInputLabel.left
                anchors.verticalCenter: jackInputVolumeSlider.verticalCenter
                width: 32 * virtualstudio.uiScale
                height: 32 * virtualstudio.uiScale
                icon.source: "mic.svg"
            }

            Meter {
                id: jackInputMeters
                anchors.left: jackInputLabel.right
                anchors.right: parent.right
                anchors.rightMargin: rightMargin * virtualstudio.uiScale
                anchors.verticalCenter: jackInputLabel.verticalCenter
                height: 24 * virtualstudio.uiScale
                model: virtualstudio.inputMeterLevels
                clipped: virtualstudio.inputClipped
                enabled: virtualstudio.audioReady && !Boolean(virtualstudio.devicesError)
            }

            VolumeSlider {
                id: jackInputVolumeSlider
                anchors.left: jackInputMeters.left
                anchors.right: parent.right
                anchors.rightMargin: rightMargin * virtualstudio.uiScale
                anchors.top: jackInputMeters.bottom
                anchors.topMargin: 16 * virtualstudio.uiScale
                height: 30 * virtualstudio.uiScale
                labelText: "Send"
                tooltipText: "How loudly other participants hear you"
                showLabel: false
                sliderEnabled: true
            }

            Button {
                id: jackHiddenInputButton
                anchors.right: parent.right
                anchors.rightMargin: rightMargin * virtualstudio.uiScale
                anchors.verticalCenter: jackInputVolumeSlider.verticalCenter
                width: 144 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
                visible: false
            }
        }
    }

    Component {
        id: noBackend

        Item {
            anchors.top: parent.top
            anchors.topMargin: 24 * virtualstudio.uiScale
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.leftMargin: leftMargin * virtualstudio.uiScale
            anchors.right: parent.right

            Text {
                id: noBackendLabel
                x: 0; y: 0
                width: parent.width - (16 * virtualstudio.uiScale)
                text: "JackTrip has been compiled without an audio backend. Please rebuild with the rtaudio flag or without the nojack flag."
                font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
                wrapMode: Text.WordWrap
                color: textColour
            }
        }
    }
}
