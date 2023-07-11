import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    width: parent.width; height: parent.height
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
    property real imageLightnessValue: virtualstudio.darkMode ? 0.8 : 0.2
    property real muteButtonLightnessValue: virtualstudio.darkMode ? 1.0 : 0.0
    property real muteButtonMutedLightnessValue: 0.24
    property real muteButtonMutedSaturationValue: 0.73
    property string buttonStroke: virtualstudio.darkMode ? "#80827D7D" : "#34979797"
    property string sliderColour: virtualstudio.darkMode ? "#BABCBC" :  "#EAECEC"
    property string sliderPressedColour: virtualstudio.darkMode ? "#ACAFAF" : "#DEE0E0"
    property string sliderTrackColour: virtualstudio.darkMode ? "#5B5858" : "light gray"
    property string sliderActiveTrackColour: virtualstudio.darkMode ? "light gray" : "black"
    property string shadowColour: virtualstudio.darkMode ? "#40000000" : "#80A1A1A1"
    property string toolTipBackgroundColour: virtualstudio.darkMode ? "#323232" : "#F3F3F3"
    property string toolTipTextColour: textColour
    property string warningTextColour: "#DB0A0A"

    property string browserButtonColour: virtualstudio.darkMode ? "#494646" : "#EAECEC"
    property string browserButtonHoverColour: virtualstudio.darkMode ? "#5B5858" : "#D3D4D4"
    property string browserButtonPressedColour: virtualstudio.darkMode ? "#524F4F" : "#DEE0E0"
    property string browserButtonStroke: virtualstudio.darkMode ? "#80827D7D" : "#40979797"
    property string browserButtonHoverStroke: virtualstudio.darkMode ? "#7B7777" : "#BABCBC"
    property string browserButtonPressedStroke: virtualstudio.darkMode ? "#827D7D" : "#BABCBC"

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

            Text {
                id: outputLabel
                x: 0; y: 0
                text: "Output Device"
                font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                color: textColour
            }

            AppIcon {
                id: outputHelpIcon
                anchors.left: outputLabel.right
                anchors.bottom: outputLabel.top
                anchors.bottomMargin: -8 * virtualstudio.uiScale
                width: 16 * virtualstudio.uiScale
                height: 16 * virtualstudio.uiScale
                icon.source: "help.svg"

                property bool showToolTip: false

                MouseArea {
                    id: outputMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: outputHelpIcon.showToolTip = true
                    onExited: outputHelpIcon.showToolTip = false
                }

                ToolTip {
                    visible: outputHelpIcon.showToolTip
                    contentItem: Rectangle {
                        color: toolTipBackgroundColour
                        radius: 3
                        anchors.fill: parent
                        anchors.bottomMargin: bottomToolTipMargin * virtualstudio.uiScale
                        anchors.rightMargin: rightToolTipMargin * virtualstudio.uiScale
                        layer.enabled: true
                        border.width: 1
                        border.color: buttonStroke

                        Text {
                            anchors.centerIn: parent
                            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale}
                            text: qsTr("How you'll hear the studio audio")
                            color: toolTipTextColour
                        }
                    }
                    background: Rectangle {
                        color: "transparent"
                    }
                }
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
                model: virtualstudio.outputComboModel
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
                                virtualstudio.outputDevice = modelData.text
                                if (modelData.category === "Low-Latency (ASIO)") {
                                    let inputComboIdx = inputCombo.model.findIndex(it => it.category === "Low-Latency (ASIO)" && it.text === modelData.text);
                                    if (inputComboIdx !== null && inputComboIdx !== undefined) {
                                        inputCombo.currentIndex = inputComboIdx;
                                        virtualstudio.inputDevice = modelData.text
                                    }
                                }
                                virtualstudio.validateDevicesState()
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
                enabled: virtualstudio.connectionState == "Connected"
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

            Text {
                id: inputLabel
                anchors.left: outputLabel.left
                anchors.top: outputChannelsCombo.bottom
                anchors.topMargin: 32 * virtualstudio.uiScale
                text: "Input Device"
                font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                color: textColour
            }

            AppIcon {
                id: inputHelpIcon
                anchors.left: inputLabel.right
                anchors.bottom: inputLabel.top
                anchors.bottomMargin: -8 * virtualstudio.uiScale
                width: 16 * virtualstudio.uiScale
                height: 16 * virtualstudio.uiScale
                icon.source: "help.svg"

                property bool showToolTip: false

                MouseArea {
                    id: inputMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: inputHelpIcon.showToolTip = true
                    onExited: inputHelpIcon.showToolTip = false
                }

                ToolTip {
                    visible: inputHelpIcon.showToolTip
                    contentItem: Rectangle {
                        color: toolTipBackgroundColour
                        radius: 3
                        anchors.fill: parent
                        anchors.bottomMargin: bottomToolTipMargin * virtualstudio.uiScale
                        anchors.rightMargin: rightToolTipMargin * virtualstudio.uiScale
                        layer.enabled: true
                        border.width: 1
                        border.color: buttonStroke

                        Text {
                            anchors.centerIn: parent
                            font { family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale}
                            text: qsTr("Audio sent to the studio (microphone, instrument, mixer, etc.)")
                            color: toolTipTextColour
                        }
                    }
                    background: Rectangle {
                        color: "transparent"
                    }
                }
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
                model: virtualstudio.inputComboModel
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
                                virtualstudio.inputDevice = modelData.text
                                if (modelData.category === "Low-Latency (ASIO)") {
                                    let outputComboIdx = outputCombo.model.findIndex(it => it.category === "Low-Latency (ASIO)" && it.text === modelData.text);
                                    if (outputComboIdx !== null && outputComboIdx !== undefined) {
                                        outputCombo.currentIndex = outputComboIdx;
                                        virtualstudio.outputDevice = modelData.text
                                    }
                                }
                                virtualstudio.validateDevicesState()
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
                enabled: virtualstudio.connectionState == "Connected"
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
                enabled: virtualstudio.connectionState == "Connected"
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
                font { family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
                color: textColour
            }
        }
    }

    Button {
        id: backButton
        background: Rectangle {
            radius: 6 * virtualstudio.uiScale
            color: backButton.down ? browserButtonPressedColour : (backButton.hovered ? browserButtonHoverColour : browserButtonColour)
        }
        onClicked: virtualstudio.windowState = "connected";
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

    Button {
        id: leaveButton
        background: Rectangle {
            radius: 6 * virtualstudio.uiScale
            color: leaveButton.down ? browserButtonPressedColour : (leaveButton.hovered ? browserButtonHoverColour : browserButtonColour)
        }
        onClicked: virtualstudio.disconnect()
        anchors.left: backButton.right
        anchors.leftMargin: 16 * virtualstudio.uiScale;
        anchors.verticalCenter: backButton.verticalCenter
        width: 150 * virtualstudio.uiScale; height: 36 * virtualstudio.uiScale

        Text {
            text: "Leave"
            font { family: "Poppins"; weight: Font.Bold; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale}
            anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
            color: saveButtonText
        }
    }
}
