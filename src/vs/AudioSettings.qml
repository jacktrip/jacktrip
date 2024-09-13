import QtQuick
import QtQuick.Controls

Rectangle {
    width: parent.width
    height: parent.height
    color: backgroundColour

    property bool connected: false
    property bool showMeters: true
    property bool showTestAudio: true

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
    property string linkText: virtualstudio.darkMode ? "#8B8D8D" : "#272525"
    property string toolTipBackgroundColour: virtualstudio.darkMode ? "#323232" : "#F3F3F3"
    property string toolTipTextColour: textColour

    property string errorFlagColour: "#DB0A0A"
    property string disabledButtonTextColour: virtualstudio.darkMode ? "#827D7D" : "#BABCBC"

    function getCurrentInputDeviceIndex () {
        if (audio.inputDevice === "") {
            return audio.inputComboModel.findIndex(elem => elem.type === "element");
        }

        let idx = audio.inputComboModel.findIndex(elem => elem.type === "element" && elem.text === audio.inputDevice);
        if (idx < 0) {
            idx = audio.inputComboModel.findIndex(elem => elem.type === "element");
        }
        return idx;
    }

    function getCurrentOutputDeviceIndex() {
        if (audio.outputDevice === "") {
            return audio.outputComboModel.findIndex(elem => elem.type === "element");
        }

        let idx = audio.outputComboModel.findIndex(elem => elem.type === "element" && elem.text === audio.outputDevice);
        if (idx < 0) {
            idx = audio.outputComboModel.findIndex(elem => elem.type === "element");
        }
        return idx;
    }

    function getCurrentInputChannelsIndex() {
        let idx = audio.inputChannelsComboModel.findIndex(elem => elem.baseChannel === audio.baseInputChannel
            && elem.numChannels === audio.numInputChannels);
        if (idx < 0) {
            idx = 0;
        }
        return idx;
    }

    function getCurrentOutputChannelsIndex() {
        let idx = audio.outputChannelsComboModel.findIndex(elem => elem.baseChannel === audio.baseOutputChannel
            && elem.numChannels === audio.numOutputChannels);
        if (idx < 0) {
            idx = 0;
        }
        return idx;
    }

    function getCurrentMixModeIndex() {
        let idx = audio.inputMixModeComboModel.findIndex(elem => elem.value === audio.inputMixMode);
        if (idx < 0) {
            idx = 0;
        }
        return idx;
    }

    Loader {
        anchors.fill: parent
        sourceComponent: audio.audioBackend == "JACK" ? usingJACK : ((!audio.deviceModelsInitialized || audio.scanningDevices) ? scanningDevices : usingRtAudio);
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
                bottomPadding: 10 * virtualstudio.uiScale
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
                anchors.top: outputLabel.bottom
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
                model: audio.outputComboModel
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
                                audio.outputDevice = modelData.text
                                if (modelData.category.startsWith("Low-Latency")) {
                                    let inputComboIdx = inputCombo.model.findIndex(it => it.category.startsWith("Low-Latency") && it.text === modelData.text);
                                    if (inputComboIdx !== null && inputComboIdx !== undefined) {
                                        inputCombo.currentIndex = inputComboIdx;
                                        audio.inputDevice = modelData.text
                                    }
                                }
                                if (connected) {
                                    virtualstudio.triggerReconnect(false);
                                } else {
                                    audio.validateDevices()
                                    audio.restartAudio()
                                }
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
                model: showMeters ? audio.outputMeterLevels : [0, 0]
                clipped: audio.outputClipped
                visible: showMeters
                enabled: audio.audioReady && !Boolean(audio.devicesError)
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
                visible: showMeters
            }

            Text {
                id: outputChannelsLabel
                anchors.left: outputCombo.left
                anchors.right: outputCombo.horizontalCenter
                anchors.top: showMeters ? outputSlider.bottom : outputCombo.bottom
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
                model: audio.outputChannelsComboModel
                enabled: audio.outputChannelsComboModel.length > 1
                currentIndex: getCurrentOutputChannelsIndex()
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
                            audio.baseOutputChannel = modelData.baseChannel
                            audio.numOutputChannels = modelData.numChannels
                            if (connected) {
                                virtualstudio.triggerReconnect(false);
                            } else {
                                audio.validateDevices()
                                audio.restartAudio()
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
                    text: outputChannelsCombo.model[outputChannelsCombo.currentIndex].label || ""
                }
            }

            Button {
                id: testOutputAudioButton
                visible: showTestAudio
                background: Rectangle {
                    radius: 6 * virtualstudio.uiScale
                    color: testOutputAudioButton.down ? buttonPressedColour : (testOutputAudioButton.hovered ? buttonHoverColour : buttonColour)
                    border.width: 1
                    border.color: testOutputAudioButton.down || testOutputAudioButton.hovered ? buttonPressedStroke : (testOutputAudioButton.hovered ? buttonHoverStroke : buttonStroke)
                }
                onClicked: { audio.playOutputAudio() }
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
                anchors.top: showTestAudio ? testOutputAudioButton.bottom : outputChannelsCombo.bottom
                anchors.topMargin: 24 * virtualstudio.uiScale
                width: parent.width - x - (16 * virtualstudio.uiScale); height: 2 * virtualstudio.uiScale
                color: "#E0E0E0"
            }

            Text {
                id: inputLabel
                anchors.left: outputLabel.left
                anchors.top: divider1.bottom
                anchors.topMargin: 24 * virtualstudio.uiScale
                text: "Input Device"
                font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                bottomPadding: 10 * virtualstudio.uiScale
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
                anchors.left: inputLabel.left
                anchors.top: inputLabel.bottom
                width: 32 * virtualstudio.uiScale
                height: 32 * virtualstudio.uiScale
                icon.source: "mic.svg"
            }

            ComboBox {
                id: inputCombo
                model: audio.inputComboModel
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
                                audio.inputDevice = modelData.text
                                if (modelData.category.startsWith("Low-Latency")) {
                                    let outputComboIdx = outputCombo.model.findIndex(it => it.category.startsWith("Low-Latency") && it.text === modelData.text);
                                    if (outputComboIdx !== null && outputComboIdx !== undefined) {
                                        outputCombo.currentIndex = outputComboIdx;
                                        audio.outputDevice = modelData.text
                                    }
                                }
                                if (connected) {
                                    virtualstudio.triggerReconnect(false);
                                } else {
                                    audio.validateDevices()
                                    audio.restartAudio()
                                }
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
                model: showMeters ? audio.inputMeterLevels : [0, 0]
                clipped: audio.inputClipped
                visible: showMeters
                enabled: audio.audioReady && !Boolean(audio.devicesError)
            }

            VolumeSlider {
                id: inputSlider
                anchors.left: inputCombo.left
                anchors.right: parent.right
                anchors.rightMargin: rightMargin * virtualstudio.uiScale
                anchors.top: inputDeviceMeters.bottom
                anchors.topMargin: 16 * virtualstudio.uiScale
                height: 30 * virtualstudio.uiScale
                labelText: "Send"
                tooltipText: "How loudly other participants hear you"
                showLabel: false
                sliderEnabled: true
                visible: showMeters
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
                anchors.top: showMeters ? inputSlider.bottom : inputCombo.bottom
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
                model: audio.inputChannelsComboModel
                enabled: audio.inputChannelsComboModel.length > 1
                currentIndex: getCurrentInputChannelsIndex()
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
                            audio.baseInputChannel = modelData.baseChannel
                            audio.numInputChannels = modelData.numChannels
                            if (connected) {
                                virtualstudio.triggerReconnect(false);
                            } else {
                                audio.validateDevices()
                                audio.restartAudio()
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
                    text: inputChannelsCombo.model[inputChannelsCombo.currentIndex].label || ""
                }
            }

            Text {
                id: inputMixModeLabel
                anchors.left: inputCombo.horizontalCenter
                anchors.right: inputCombo.right
                anchors.rightMargin: 8 * virtualstudio.uiScale
                anchors.top: showMeters ? inputSlider.bottom : inputCombo.bottom
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
                model: audio.inputMixModeComboModel
                enabled: audio.inputMixModeComboModel.length > 1
                currentIndex: getCurrentMixModeIndex()
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
                            audio.inputMixMode = audio.inputMixModeComboModel[index].value
                            if (connected) {
                                virtualstudio.triggerReconnect(false);
                            } else {
                                audio.validateDevices()
                                audio.restartAudio()
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
                text: audio.inputChannelsComboModel.length > 1 ? "Choose up to 2 channels" : "Only 1 channel available"
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
                    if (audio.inputMixMode === 2) {
                        return "Treat the channels as Left and Right signals, coming through each speaker separately.";
                    } else if (audio.inputMixMode === 3) {
                        return "Combine the channels into one central channel coming through both speakers.";
                    } else if (audio.inputMixMode === 1) {
                        return "Send a single channel of audio";
                    } else {
                        return "";
                    }
                })()
                font { family: "Poppins"; pixelSize: fontExtraSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                color: textColour
            }

            Connections {
                target: audio
                // anything that sets currentIndex to the value of a function needs
                // to be manually updated whenever there is a change to any vars it uses
                function onInputDeviceChanged() {
                    inputCombo.currentIndex = getCurrentInputDeviceIndex();
                }
                function onOutputDeviceChanged() {
                    outputCombo.currentIndex = getCurrentOutputDeviceIndex();
                }
                function onNumInputChannelsChanged() {
                    inputChannelsCombo.currentIndex = getCurrentInputChannelsIndex();
                }
                function onBaseInputChannelChanged() {
                    inputChannelsCombo.currentIndex = getCurrentInputChannelsIndex();
                }
                function onNumOutputChannelsChanged() {
                    outputChannelsCombo.currentIndex = getCurrentOutputChannelsIndex();
                }
                function onBaseOutputChannelChanged() {
                    outputChannelsCombo.currentIndex = getCurrentOutputChannelsIndex();
                }
                function onInputMixModeChanged() {
                    inputMixModeCombo.currentIndex = getCurrentMixModeIndex();
                }
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
                bottomPadding: 10 * virtualstudio.uiScale
                color: textColour
            }

            AppIcon {
                id: jackHeadphonesIcon
                anchors.left: jackOutputLabel.left
                anchors.top: jackOutputLabel.bottom
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
                model: showMeters ? audio.outputMeterLevels : [0, 0]
                clipped: audio.outputClipped
                enabled: audio.audioReady && !Boolean(audio.devicesError)
            }

            Button {
                id: jackTestOutputAudioButton
                background: Rectangle {
                    radius: 6 * virtualstudio.uiScale
                    color: jackTestOutputAudioButton.down ? buttonPressedColour : (jackTestOutputAudioButton.hovered ? buttonHoverColour : buttonColour)
                    border.width: 1
                    border.color: jackTestOutputAudioButton.down ? buttonPressedStroke : (jackTestOutputAudioButton.hovered ? buttonHoverStroke : buttonStroke)
                }
                onClicked: { audio.playOutputAudio() }
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
                bottomPadding: 10 * virtualstudio.uiScale
                color: textColour
            }

            AppIcon {
                id: jackMicrophoneIcon
                anchors.left: jackInputLabel.left
                anchors.top: jackInputLabel.bottom
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
                model: showMeters ? audio.inputMeterLevels : [0, 0]
                clipped: audio.inputClipped
                enabled: audio.audioReady && !Boolean(audio.devicesError)
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

    Component {
        id: scanningDevices

        Item {
            anchors.top: parent.top
            anchors.topMargin: 24 * virtualstudio.uiScale
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.leftMargin: leftMargin * virtualstudio.uiScale
            anchors.right: parent.right

            Text {
                id: scanningDevicesLabel
                x: 0; y: 0
                width: parent.width - (16 * virtualstudio.uiScale)
                text: "Scanning audio devices..."
                font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
                wrapMode: Text.WordWrap
                color: textColour
            }
        }
    }
}
