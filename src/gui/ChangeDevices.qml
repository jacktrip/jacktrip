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

    property string saveButtonText: "#DB0A0A"
    property string textColour: virtualstudio.darkMode ? "#FAFBFB" : "#0F0D0D"
    property string meterColor: virtualstudio.darkMode ? "gray" : "#E0E0E0"
    property real muteButtonLightnessValue: virtualstudio.darkMode ? 1.0 : 0.0
    property real muteButtonMutedLightnessValue: 0.24
    property real muteButtonMutedSaturationValue: 0.73
    property string buttonStroke: virtualstudio.darkMode ? "#80827D7D" : "#34979797"
    property string shadowColour: virtualstudio.darkMode ? "#40000000" : "#80A1A1A1"
    property string toolTipBackgroundColour: virtualstudio.darkMode ? "#323232" : "#F3F3F3"
    property string toolTipTextColour: textColour

    property string browserButtonColour: virtualstudio.darkMode ? "#494646" : "#EAECEC"
    property string browserButtonHoverColour: virtualstudio.darkMode ? "#5B5858" : "#D3D4D4"
    property string browserButtonPressedColour: virtualstudio.darkMode ? "#524F4F" : "#DEE0E0"
    property string browserButtonStroke: virtualstudio.darkMode ? "#80827D7D" : "#40979797"
    property string browserButtonHoverStroke: virtualstudio.darkMode ? "#7B7777" : "#BABCBC"
    property string browserButtonPressedStroke: virtualstudio.darkMode ? "#827D7D" : "#BABCBC"

    property string linkText: virtualstudio.darkMode ? "#8B8D8D" : "#272525"

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

    Rectangle {
        width: parent.width; height: 360
        anchors.verticalCenter: parent.verticalCenter
        color: backgroundColour
        radius: 6 * virtualstudio.uiScale

        Item {
            id: usingRtAudio
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

            DeviceRefreshButton {
                id: refreshButton
                y: 0;
                x: parent.width - (144 + rightMargin) * virtualstudio.uiScale;
                enabled: !audio.scanningDevices
                onDeviceRefresh: function () {
                    virtualstudio.triggerReconnect(true);
                }
            }

            Text {
                text: "Scanning Devices"
                y: 0;
                anchors.right: refreshButton.left;
                anchors.rightMargin: 16 * virtualstudio.uiScale;
                font { family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
                color: textColour
                visible: audio.scanningDevices
            }

            Text {
                id: outputLabel
                x: 0;
                anchors.top: refreshButton.bottom
                anchors.topMargin: 24 * virtualstudio.uiScale
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
                anchors.top: outputLabel.bottom
                anchors.topMargin: bottomToolTipMargin * virtualstudio.uiScale
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
                enabled: virtualstudio.connectionState == "Connected"
                model: audio.outputComboModel
                currentIndex: getCurrentOutputDeviceIndex()
                delegate: ItemDelegate {
                    required property var modelData
                    required property int index

                    leftPadding: 0

                    width: parent.width
                    contentItem: Text {
                        leftPadding: modelData.type === "element" && outputCombo.model.filter(it => it.type === "header").length > 0 ? 24 : 12
                        text: modelData.text || ""
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
                                virtualstudio.triggerReconnect(false);
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
                    text: outputCombo.model[outputCombo.currentIndex] && outputCombo.model[outputCombo.currentIndex].text ? outputCombo.model[outputCombo.currentIndex].text : ""
                }
            }

            Text {
                id: outputChannelsLabel
                anchors.left: outputCombo.left
                anchors.right: outputCombo.horizontalCenter
                anchors.top: outputCombo.bottom
                anchors.topMargin: 12 * virtualstudio.uiScale
                textFormat: Text.RichText
                text: "Output Channel(s)"
                font { family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
                color: textColour
            }

            ComboBox {
                id: outputChannelsCombo
                anchors.left: outputCombo.left
                anchors.right: outputCombo.horizontalCenter
                anchors.rightMargin: 8 * virtualstudio.uiScale
                anchors.top: outputChannelsLabel.bottom
                anchors.topMargin: 4 * virtualstudio.uiScale
                enabled: audio.outputChannelsComboModel.length > 1 && virtualstudio.connectionState == "Connected"
                model: audio.outputChannelsComboModel
                currentIndex: (() => {
                    let idx = audio.outputChannelsComboModel.findIndex(elem => elem.baseChannel === audio.baseOutputChannel
                        && elem.numChannels === audio.numOutputChannels);
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
                            audio.baseOutputChannel = modelData.baseChannel
                            audio.numOutputChannels = modelData.numChannels
                            virtualstudio.triggerReconnect(false);
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

            Text {
                id: inputLabel
                anchors.left: outputLabel.left
                anchors.top: outputChannelsCombo.bottom
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
                anchors.left: inputLabel.left
                anchors.top: inputLabel.bottom
                anchors.topMargin: bottomToolTipMargin * virtualstudio.uiScale
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
                enabled: virtualstudio.connectionState == "Connected"
                delegate: ItemDelegate {
                    required property var modelData
                    required property int index

                    leftPadding: 0

                    width: parent.width
                    contentItem: Text {
                        leftPadding: modelData.type === "element" && inputCombo.model.filter(it => it.type === "header").length > 0 ? 24 : 12
                        text: modelData.text || ""
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
                                virtualstudio.triggerReconnect(false);
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
                    text: inputCombo.model[inputCombo.currentIndex] && inputCombo.model[inputCombo.currentIndex].text ? inputCombo.model[inputCombo.currentIndex].text : ""
                }
            }

            Text {
                id: inputChannelsLabel
                anchors.left: inputCombo.left
                anchors.right: inputCombo.horizontalCenter
                anchors.top: inputCombo.bottom
                anchors.topMargin: 12 * virtualstudio.uiScale
                textFormat: Text.RichText
                text: "Input Channel(s)"
                font { family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
                color: textColour
            }

            ComboBox {
                id: inputChannelsCombo
                anchors.left: inputCombo.left
                anchors.right: inputCombo.horizontalCenter
                anchors.rightMargin: 8 * virtualstudio.uiScale
                anchors.top: inputChannelsLabel.bottom
                anchors.topMargin: 4 * virtualstudio.uiScale
                enabled: audio.inputChannelsComboModel.length > 1 && virtualstudio.connectionState == "Connected"
                model: audio.inputChannelsComboModel
                currentIndex: (() => {
                    let idx = audio.inputChannelsComboModel.findIndex(elem => elem.baseChannel === audio.baseInputChannel
                        && elem.numChannels === audio.numInputChannels);
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
                            audio.baseInputChannel = modelData.baseChannel
                            audio.numInputChannels = modelData.numChannels
                            virtualstudio.triggerReconnect(false);
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
                anchors.top: inputCombo.bottom
                anchors.topMargin: 12 * virtualstudio.uiScale
                textFormat: Text.RichText
                text: "Mono / Stereo"
                font { family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
                color: textColour
            }

            ComboBox {
                id: inputMixModeCombo
                anchors.left: inputCombo.horizontalCenter
                anchors.right: inputCombo.right
                anchors.rightMargin: 8 * virtualstudio.uiScale
                anchors.top: inputMixModeLabel.bottom
                anchors.topMargin: 4 * virtualstudio.uiScale
                enabled: audio.inputMixModeComboModel.length > 1 && virtualstudio.connectionState == "Connected"
                model: audio.inputMixModeComboModel
                currentIndex: (() => {
                    let idx = audio.inputMixModeComboModel.findIndex(elem => elem.value === audio.inputMixMode);
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
                            audio.inputMixMode = audio.inputMixModeComboModel[index].value
                            virtualstudio.triggerReconnect(false);
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
                font { family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
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
                font { family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
                color: textColour
            }

            DeviceWarning {
                id: deviceWarning
                anchors.left: inputCombo.left
                anchors.top: inputMixModeHelpMessage.bottom
                anchors.topMargin: 48 * virtualstudio.uiScale
                visible: Boolean(audio.devicesError) || Boolean(audio.devicesWarning)
            }
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
}
