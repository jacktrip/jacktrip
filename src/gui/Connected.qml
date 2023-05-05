import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.12

Item {
    width: parent.width; height: parent.height
    clip: true

    property bool connecting: false

    property int leftHeaderMargin: 16
    property int fontBig: 28
    property int fontMedium: 12
    property int fontSmall: 10
    property int fontTiny: 8

    property int bodyMargin: 60
    property int rightMargin: 16
    property int bottomToolTipMargin: 8
    property int rightToolTipMargin: 4

    property string studioStatus: (virtualstudio.currentStudio >= 0 ? serverModel[virtualstudio.currentStudio].status : "")
    property bool showReadyScreen: studioStatus === "Ready"
    property bool showStartingScreen: studioStatus === "Starting"
    property bool showStoppingScreen: (virtualstudio.currentStudio >= 0 ? serverModel[virtualstudio.currentStudio].isAdmin && !serverModel[virtualstudio.currentStudio].enabled && serverModel[virtualstudio.currentStudio].cloudId !== "" : false)
    property bool showWaitingScreen: !showStoppingScreen && !showStartingScreen && !showReadyScreen

    property string buttonColour: virtualstudio.darkMode ? "#494646" : "#EAECEC"

    property string browserButtonColour: virtualstudio.darkMode ? "#494646" : "#EAECEC"
    property string browserButtonHoverColour: virtualstudio.darkMode ? "#5B5858" : "#D3D4D4"
    property string browserButtonPressedColour: virtualstudio.darkMode ? "#524F4F" : "#DEE0E0"
    property string browserButtonStroke: virtualstudio.darkMode ? "#80827D7D" : "#40979797"
    property string browserButtonHoverStroke: virtualstudio.darkMode ? "#7B7777" : "#BABCBC"
    property string browserButtonPressedStroke: virtualstudio.darkMode ? "#827D7D" : "#BABCBC"
    property string saveButtonBackgroundColour: "#F2F3F3"
    property string saveButtonPressedColour: "#E7E8E8"
    property string saveButtonStroke: "#EAEBEB"
    property string saveButtonPressedStroke: "#B0B5B5"
    property string saveButtonText: "#DB0A0A"

    property string muteButtonMutedColor: "#FCB6B6"
    property string textColour: virtualstudio.darkMode ? "#FAFBFB" : "#0F0D0D"
    property string meterColor: virtualstudio.darkMode ? "gray" : "#E0E0E0"
    property real imageLightnessValue: virtualstudio.darkMode ? 1.0 : 0.0
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
    property string linkText: virtualstudio.darkMode ? "#8B8D8D" : "#272525"

    property string meterGreen: "#61C554"
    property string meterYellow: "#F5BF4F"
    property string meterRed: "#F21B1B"

    property bool isUsingRtAudio: virtualstudio.audioBackend == "RtAudio"

    function getCurrentInputDeviceIndex () {
        if (virtualstudio.inputDevice === "") {
            return inputComboModel.findIndex(elem => elem.type === "element");
        }

        let idx = inputComboModel.findIndex(elem => elem.type === "element" && elem.text === virtualstudio.inputDevice);
        if (idx < 0) {
            idx = inputComboModel.findIndex(elem => elem.type === "element");
        }

        return idx;
    }

    function getCurrentOutputDeviceIndex() {
        if (virtualstudio.outputDevice === "") {
            return outputComboModel.findIndex(elem => elem.type === "element");
        }

        let idx = outputComboModel.findIndex(elem => elem.type === "element" && elem.text === virtualstudio.outputDevice);
        if (idx < 0) {
            idx = outputComboModel.findIndex(elem => elem.type === "element");
        }

        return idx;
    }

    function getNetworkStatsText (networkStats) {
        let minRtt = networkStats.minRtt;
        let maxRtt = networkStats.maxRtt;
        let avgRtt = networkStats.avgRtt;

        let texts = ["Measuring stats ...", ""];

        if (!minRtt || !maxRtt) {
            return texts;
        }

        texts[0] = "<b>" + minRtt + " ms - " + maxRtt + " ms</b>, avg " + avgRtt + " ms round-trip time";

        let quality = "poor";
        if (avgRtt <= 25) {

            if (maxRtt <= 30) {
                quality = "excellent";
            } else {
                quality = "good";
            }

        } else if (avgRtt <= 30) {
            quality = "good";
        } else if (avgRtt <= 35) {
            quality = "fair";
        }

        texts[1] = "Your connection quality is <b>" + quality + "</b>."
        return texts;
    }

    Connections {
        target: virtualstudio
        function onInputDeviceChanged() {
            inputCombo.currentIndex = getCurrentInputDeviceIndex();
        }
        function onOutputDeviceChanged() {
            outputCombo.currentIndex = getCurrentOutputDeviceIndex();
        }
    }

    Image {
        id: jtlogo
        x: parent.width - (49 * virtualstudio.uiScale); y: 16 * virtualstudio.uiScale
        width: 32 * virtualstudio.uiScale; height: 59 * virtualstudio.uiScale
        source: "logo.svg"
        sourceSize: Qt.size(jtlogo.width,jtlogo.height)
        fillMode: Image.PreserveAspectFit
        smooth: true
    }

    Text {
        id: heading
        text: studioStatus === "Starting" ? "Starting..." : virtualstudio.connectionState
        x: leftHeaderMargin * virtualstudio.uiScale; y: 34 * virtualstudio.uiScale
        font { family: "Poppins"; weight: Font.Bold; pixelSize: fontBig * virtualstudio.fontScale * virtualstudio.uiScale }
        color: textColour
    }

    Studio {
        x: leftHeaderMargin * virtualstudio.uiScale; y: 96 * virtualstudio.uiScale
        width: parent.width - (2 * x)
        connected: true
        serverLocation: virtualstudio.currentStudio >= 0 && virtualstudio.regions[serverModel[virtualstudio.currentStudio].location] ? "in " + virtualstudio.regions[serverModel[virtualstudio.currentStudio].location].label : ""
        flagImage: virtualstudio.currentStudio >= 0 ? ( serverModel[virtualstudio.currentStudio].bannerURL ? serverModel[virtualstudio.currentStudio].bannerURL : serverModel[virtualstudio.currentStudio].flag ) : "flags/DE.svg"
        studioName: virtualstudio.currentStudio >= 0 ? serverModel[virtualstudio.currentStudio].name : "Test Studio"
        publicStudio: virtualstudio.currentStudio >= 0 ? serverModel[virtualstudio.currentStudio].isPublic : false
        admin: virtualstudio.currentStudio >= 0 ? serverModel[virtualstudio.currentStudio].isAdmin : false
        available: virtualstudio.currentStudio >= 0 ? serverModel[virtualstudio.currentStudio].canConnect : false
        studioId: virtualstudio.currentStudio >= 0 ? serverModel[virtualstudio.currentStudio].id : ""
        inviteKeyString: virtualstudio.currentStudio >= 0 ? serverModel[virtualstudio.currentStudio].inviteKey : ""
    }

    Item {
        id: deviceSettings
        visible: showReadyScreen && isUsingRtAudio
        x: bodyMargin * virtualstudio.uiScale; y: 192 * virtualstudio.uiScale
        width: parent.width - (2 * x)
        height: 384 * virtualstudio.uiScale
        clip: true

        Button {
            id: deviceSettingsButton
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: deviceSettingsButton.down ? browserButtonPressedColour : (deviceSettingsButton.hovered ? browserButtonHoverColour : browserButtonColour)
            }
            onClicked: popup.open()
            anchors.right: parent.right
            width: 144 * virtualstudio.uiScale; height: 24 * virtualstudio.uiScale

            Text {
                text: "Change Device Settings"
                font { family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale}
                anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
                color: textColour
            }
        }

        Popup {
            id: popup
            padding: 1
            width: parent.width
            height: parent.height
            anchors.centerIn: parent
            modal: true
            focus: true
            closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
            onClosed: {
                virtualstudio.applySettings()
            }

            background: Rectangle {
                anchors.fill: parent
                color: "transparent"
                radius: 6 * virtualstudio.uiScale
                border.width: 1
                border.color: buttonStroke
                clip: true
            }

            contentItem: Rectangle {
                width: parent.width
                height: parent.height
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

                    visible: isUsingRtAudio

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

                    Image {
                        id: outputHelpIcon
                        anchors.left: outputLabel.right
                        anchors.bottom: outputLabel.top
                        anchors.bottomMargin: -8 * virtualstudio.uiScale
                        source: "help.svg"
                        sourceSize: Qt.size(12 * virtualstudio.uiScale, 12 * virtualstudio.uiScale)
                        fillMode: Image.PreserveAspectFit
                        smooth: true

                        property bool showToolTip: false

                        Colorize {
                            anchors.fill: parent
                            source: parent
                            hue: 0
                            saturation: 0
                            lightness: virtualstudio.darkMode ? 0.8 : 0.2
                        }

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

                    Image {
                        id: headphonesIcon
                        anchors.left: outputLabel.left
                        anchors.top: outputLabel.bottom
                        anchors.topMargin: bottomToolTipMargin * virtualstudio.uiScale
                        source: "headphones.svg"
                        sourceSize: Qt.size(28 * virtualstudio.uiScale, 28 * virtualstudio.uiScale)
                        fillMode: Image.PreserveAspectFit
                        smooth: true

                        Colorize {
                            anchors.fill: parent
                            source: parent
                            hue: 0
                            saturation: 0
                            lightness: virtualstudio.darkMode ? 1 : 0
                        }
                    }

                    ComboBox {
                        id: outputCombo
                        anchors.left: leftSpacer.right
                        anchors.verticalCenter: outputLabel.verticalCenter
                        anchors.rightMargin: rightMargin * virtualstudio.uiScale
                        width: parent.width - leftSpacer.width - rightMargin * virtualstudio.uiScale
                        enabled: virtualstudio.connectionState == "Connected"
                        model: outputComboModel
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
                            text: outputCombo.model[outputCombo.currentIndex].text ? outputCombo.model[outputCombo.currentIndex].text : ""
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
                        model: outputChannelsComboModel
                        currentIndex: (() => {
                            let idx = outputChannelsComboModel.findIndex(elem => elem.baseChannel === virtualstudio.baseOutputChannel
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

                    Image {
                        id: inputHelpIcon
                        anchors.left: inputLabel.right
                        anchors.bottom: inputLabel.top
                        anchors.bottomMargin: -8 * virtualstudio.uiScale
                        source: "help.svg"
                        sourceSize: Qt.size(12 * virtualstudio.uiScale, 12 * virtualstudio.uiScale)
                        fillMode: Image.PreserveAspectFit
                        smooth: true

                        property bool showToolTip: false

                        Colorize {
                            anchors.fill: parent
                            source: parent
                            hue: 0
                            saturation: 0
                            lightness: virtualstudio.darkMode ? 0.8 : 0.2
                        }

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

                    Image {
                        id: microphoneIcon
                        anchors.left: inputLabel.left
                        anchors.top: inputLabel.bottom
                        anchors.topMargin: bottomToolTipMargin * virtualstudio.uiScale
                        source: "mic.svg"
                        sourceSize: Qt.size(32 * virtualstudio.uiScale, 32 * virtualstudio.uiScale)
                        fillMode: Image.PreserveAspectFit
                        smooth: true

                        Colorize {
                            anchors.fill: parent
                            source: parent
                            hue: 0
                            saturation: 0
                            lightness: virtualstudio.darkMode ? 1 : 0
                        }
                    }

                    ComboBox {
                        id: inputCombo
                        model: inputComboModel
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
                            text: inputCombo.model[inputCombo.currentIndex].text ? inputCombo.model[inputCombo.currentIndex].text : ""
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
                        model: inputChannelsComboModel
                        currentIndex: (() => {
                            let idx = inputChannelsComboModel.findIndex(elem => elem.baseChannel === virtualstudio.baseInputChannel
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
                        model: inputMixModeComboModel
                        currentIndex: (() => {
                            let idx = inputMixModeComboModel.findIndex(elem => elem.value === virtualstudio.inputMixMode);
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
                                    virtualstudio.inputMixMode = inputMixModeComboModel[index].value
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

                    Button {
                        id: closePopupButton
                        anchors.right: parent.right
                        anchors.rightMargin: rightMargin * virtualstudio.uiScale
                        anchors.bottomMargin: rightMargin * virtualstudio.uiScale
                        anchors.bottom: parent.bottom
                        width: 150 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
                        onClicked: popup.close()

                        background: Rectangle {
                            radius: 6 * virtualstudio.uiScale
                            color: closePopupButton.down ? browserButtonPressedColour : (closePopupButton.hovered ? browserButtonHoverColour : browserButtonColour)
                            border.width: 1
                            border.color: closePopupButton.down ? browserButtonPressedStroke : (closePopupButton.hovered ? browserButtonHoverStroke : browserButtonStroke)
                        }

                        Text {
                            text: "Close"
                            font.family: "Poppins"
                            font.pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale
                            font.weight: Font.Bold
                            color: !Boolean(virtualstudio.devicesError) && virtualstudio.backendAvailable ? saveButtonText : disabledButtonText
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    Button {
                        id: refreshButton
                        text: "Refresh Devices"
                        anchors.right: closePopupButton.left
                        anchors.rightMargin: 8 * virtualstudio.uiScale
                        anchors.bottomMargin: rightMargin * virtualstudio.uiScale
                        anchors.bottom: parent.bottom
                        width: 150 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
                        enabled: virtualstudio.connectionState == "Connected"

                        palette.buttonText: textColour
                        background: Rectangle {
                            radius: 6 * virtualstudio.uiScale
                            color: refreshButton.down ? browserButtonPressedColour : (refreshButton.hovered ? browserButtonHoverColour : browserButtonColour)
                            border.width: 1
                            border.color: refreshButton.down ? browserButtonPressedStroke : (refreshButton.hovered ? browserButtonHoverStroke : browserButtonStroke)
                        }

                        icon {
                            source: "refresh.svg";
                            color: textColour;
                        }
                        display: AbstractButton.TextBesideIcon
                        onClicked: {
                            virtualstudio.validateDevicesState();
                        }

                        font {
                            family: "Poppins"
                            pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale
                        }
                    }
                }
            }
        }
    }

    Item {
        id: inputDevice
        visible: showReadyScreen
        x: bodyMargin * virtualstudio.uiScale; y: 240 * virtualstudio.uiScale
        width: Math.min(parent.width / 2, 320 * virtualstudio.uiScale) - x
        height: 100 * virtualstudio.uiScale
        clip: true

        Image {
            id: mic
            source: "mic.svg"
            x: 0; y: 0
            width: 28 * virtualstudio.uiScale; height: 28 * virtualstudio.uiScale
            sourceSize: Qt.size(mic.width,mic.height)
            fillMode: Image.PreserveAspectFit
            smooth: true
        }

        Colorize {
            anchors.fill: mic
            source: mic
            hue: 0
            saturation: 0
            lightness: imageLightnessValue
        }

        Text {
            id: inputDeviceHeader
            x: 64 * virtualstudio.uiScale
            width: parent.width - 64 * virtualstudio.uiScale
            text: "<b>Input Device</b>"
            font {family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            anchors.verticalCenter: mic.verticalCenter
            color: textColour
            elide: Text.ElideRight
        }

        Text {
            id: inputDeviceName
            width: parent.width - 100 * virtualstudio.uiScale
            anchors.top: inputDeviceHeader.bottom
            anchors.left: inputDeviceHeader.left
            text: virtualstudio.audioBackend == "JACK" ?
                virtualstudio.audioBackend : virtualstudio.inputDevice
            font {family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            elide: Text.ElideRight
        }
    }

    Item {
        id: outputDevice
        visible: showReadyScreen
        x: bodyMargin * virtualstudio.uiScale; y: 320 * virtualstudio.uiScale
        width: Math.min(parent.width / 2, 320 * virtualstudio.uiScale) - x
        height: 124 * virtualstudio.uiScale
        clip: true
        anchors.top: inputDevice.bottom

        Image {
            id: headphones
            source: "headphones.svg"
            x: 0; y: 0
            width: 28 * virtualstudio.uiScale; height: 28 * virtualstudio.uiScale
            sourceSize: Qt.size(headphones.width,headphones.height)
            fillMode: Image.PreserveAspectFit
            smooth: true
        }

        Colorize {
            anchors.fill: headphones
            source: headphones
            hue: 0
            saturation: 0
            lightness: imageLightnessValue
        }

        Text {
            id: outputDeviceHeader
            x: 64 * virtualstudio.uiScale
            width: parent.width - 64 * virtualstudio.uiScale
            text: "<b>Output Device</b>"
            font {family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            anchors.verticalCenter: headphones.verticalCenter
            color: textColour
            elide: Text.ElideRight
        }

        Text {
            id: outputDeviceName
            width: parent.width - 100 * virtualstudio.uiScale
            anchors.top: outputDeviceHeader.bottom
            anchors.left: outputDeviceHeader.left
            text: virtualstudio.audioBackend == "JACK" ?
                virtualstudio.audioBackend : virtualstudio.outputDevice
            font {family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            elide: Text.ElideRight
        }
    }

    Item {
        id: inputControls
        visible: showReadyScreen
        x: inputDevice.x + inputDevice.width; y: 240 * virtualstudio.uiScale
        width: parent.width - inputDevice.width - 2 * bodyMargin * virtualstudio.uiScale

        Meter {
            id: inputDeviceMeters
            x: 0; y: 0
            width: parent.width
            height: 100 * virtualstudio.uiScale
            model: inputMeterModel
            clipped: inputClipped
        }

        Slider {
            id: inputSlider
            from: 0.0
            value: virtualstudio ? virtualstudio.inputVolume : 0.5
            onMoved: { virtualstudio.inputVolume = value }
            to: 1.0
            enabled: !virtualstudio.inputMuted
            padding: 0
            y: inputDeviceMeters.y + 36 * virtualstudio.uiScale
            anchors.left: inputMute.right
            anchors.right: inputStudioText.left
            anchors.leftMargin: 8 * virtualstudio.uiScale
            anchors.rightMargin: 16 * virtualstudio.uiScale
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
                implicitWidth: 26 * virtualstudio.uiScale
                implicitHeight: 26 * virtualstudio.uiScale
                radius: 13 * virtualstudio.uiScale
                color: inputSlider.pressed ? sliderPressedColour : sliderColour
                border.color: buttonStroke
                opacity: virtualstudio.inputMuted ? 0.3 : 1
            }
        }

        Button {
            id: inputMute
            width: 24 * virtualstudio.uiScale
            height: 24
            anchors.left: inputDeviceMeters.left
            anchors.verticalCenter: inputDeviceMeters.verticalCenter
            background: Rectangle {
                color: virtualstudio.inputMuted ? muteButtonMutedColor : buttonColour
                width: 24 * virtualstudio.uiScale
                radius: 4 * virtualstudio.uiScale
            }
            onClicked: { virtualstudio.inputMuted = !virtualstudio.inputMuted }
            Image {
                id: micMute
                width: 18 * virtualstudio.uiScale; height: 18 * virtualstudio.uiScale
                anchors { verticalCenter: parent.verticalCenter; horizontalCenter: parent.horizontalCenter }
                source: virtualstudio.inputMuted ? "micoff.svg" : "mic.svg"
                sourceSize: Qt.size(micMute.width,micMute.height)
                fillMode: Image.PreserveAspectFit
                smooth: true
            }
            Colorize {
                anchors.fill: micMute
                source: micMute
                hue: 0
                saturation: virtualstudio.inputMuted ? muteButtonMutedSaturationValue : 0
                lightness: virtualstudio.inputMuted ? (inputMute.hovered ? muteButtonMutedLightnessValue + .1 : muteButtonMutedLightnessValue) : (inputMute.hovered ? muteButtonLightnessValue - .1 : muteButtonLightnessValue)
            }
            ToolTip {
                parent: inputMute
                visible: inputMute.hovered
                bottomPadding: bottomToolTipMargin * virtualstudio.uiScale
                rightPadding: rightToolTipMargin * virtualstudio.uiScale
                delay: 100
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
                        text: virtualstudio.inputMuted ?  qsTr("Click to unmute yourself") : qsTr("Click to mute yourself")
                        color: toolTipTextColour
                    }
                }
                background: Rectangle {
                    color: "transparent"
                }
            }
        }

        Text {
            id: inputStudioText
            width: 40 * virtualstudio.uiScale
            height: 24
            horizontalAlignment: Text.AlignRight
            anchors.right: inputDeviceMeters.right
            anchors.verticalCenter: inputSlider.verticalCenter
            topPadding: 4 * virtualstudio.uiScale
            rightPadding: 4 * virtualstudio.uiScale
            text: "Send"
            font {family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale; bold: true }
            color: textColour
        }

        Image {
            id: inputStudioHelpIcon
            anchors.left: inputStudioText.right
            anchors.verticalCenter: inputStudioText.verticalCenter
            anchors.bottomMargin: -8 * virtualstudio.uiScale
            source: "help.svg"
            sourceSize: Qt.size(12 * virtualstudio.uiScale, 12 * virtualstudio.uiScale)
            fillMode: Image.PreserveAspectFit
            smooth: true

            property bool showToolTip: false

            Colorize {
                anchors.fill: parent
                source: parent
                hue: 0
                saturation: 0
                lightness: virtualstudio.darkMode ? 0.8 : 0.2
            }

            MouseArea {
                id: inputStudioMouseArea
                anchors.fill: parent
                hoverEnabled: true
                onEntered: inputStudioHelpIcon.showToolTip = true
                onExited: inputStudioHelpIcon.showToolTip = false
            }

            ToolTip {
                visible: inputStudioHelpIcon.showToolTip
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
                        text: qsTr("How loudly other participants hear you")
                        color: toolTipTextColour
                    }
                }
                background: Rectangle {
                    color: "transparent"
                }
            }
        }
    }

    Item {
        id: outputControls
        visible: showReadyScreen
        x: outputDevice.x + outputDevice.width; y: 320 * virtualstudio.uiScale
        width: parent.width - inputDevice.width - 2 * bodyMargin * virtualstudio.uiScale
        anchors.top: inputDevice.bottom

        Meter {
            id: outputDeviceMeters
            x: 0; y: 0
            width: parent.width
            height: 100 * virtualstudio.uiScale
            model: outputMeterModel
            clipped: outputClipped
        }

        Slider {
            id: outputSlider
            from: 0.0
            value: virtualstudio ? virtualstudio.outputVolume : 0.5
            onMoved: { virtualstudio.outputVolume = value }
            to: 1.0
            padding: 0
            y: outputDeviceMeters.y + 36 * virtualstudio.uiScale
            anchors.left: outputDeviceMeters.left
            anchors.right: outputStudioText.left
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
                implicitWidth: 26 * virtualstudio.uiScale
                implicitHeight: 26 * virtualstudio.uiScale
                radius: 13 * virtualstudio.uiScale
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
            y: outputSlider.y + 36 * virtualstudio.uiScale
            anchors.left: outputDeviceMeters.left
            anchors.right: outputMonText.left
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
                implicitWidth: 26 * virtualstudio.uiScale
                implicitHeight: 26 * virtualstudio.uiScale
                radius: 13 * virtualstudio.uiScale
                color: monitorSlider.pressed ? sliderPressedColour : sliderColour
                border.color: buttonStroke
            }
        }

        Text {
            id: outputStudioText
            width: 40 * virtualstudio.uiScale
            height: 24
            horizontalAlignment: Text.AlignRight
            anchors.right: outputDeviceMeters.right
            anchors.verticalCenter: outputSlider.verticalCenter
            topPadding: 4 * virtualstudio.uiScale
            rightPadding: 4 * virtualstudio.uiScale
            text: "Studio"
            font {family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale; bold: true }
            color: textColour
        }

        Image {
            id: outputStudioHelpIcon
            anchors.left: outputStudioText.right
            anchors.verticalCenter: outputStudioText.verticalCenter
            anchors.bottomMargin: -8 * virtualstudio.uiScale
            source: "help.svg"
            sourceSize: Qt.size(12 * virtualstudio.uiScale, 12 * virtualstudio.uiScale)
            fillMode: Image.PreserveAspectFit
            smooth: true

            property bool showToolTip: false

            Colorize {
                anchors.fill: parent
                source: parent
                hue: 0
                saturation: 0
                lightness: virtualstudio.darkMode ? 0.8 : 0.2
            }

            MouseArea {
                id: outputStudioMouseArea
                anchors.fill: parent
                hoverEnabled: true
                onEntered: outputStudioHelpIcon.showToolTip = true
                onExited: outputStudioHelpIcon.showToolTip = false
            }

            ToolTip {
                visible: outputStudioHelpIcon.showToolTip
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
                        text: qsTr("How loudly you hear other participants")
                        color: toolTipTextColour
                    }
                }
                background: Rectangle {
                    color: "transparent"
                }
            }
        }

        Text {
            id: outputMonText
            width: 40 * virtualstudio.uiScale
            height: 24
            horizontalAlignment: Text.AlignRight
            anchors.right: outputDeviceMeters.right
            anchors.verticalCenter: monitorSlider.verticalCenter
            topPadding: 4 * virtualstudio.uiScale
            rightPadding: 4 * virtualstudio.uiScale
            text: "Monitor"
            font {family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale; bold: true }
            color: textColour
        }

        Image {
            id: outputMonHelpIcon
            anchors.left: outputMonText.right
            anchors.verticalCenter: outputMonText.verticalCenter
            anchors.bottomMargin: -8 * virtualstudio.uiScale
            source: "help.svg"
            sourceSize: Qt.size(12 * virtualstudio.uiScale, 12 * virtualstudio.uiScale)
            fillMode: Image.PreserveAspectFit
            smooth: true

            property bool showToolTip: false

            Colorize {
                anchors.fill: parent
                source: parent
                hue: 0
                saturation: 0
                lightness: virtualstudio.darkMode ? 0.8 : 0.2
            }

            MouseArea {
                id: outputMonMouseArea
                anchors.fill: parent
                hoverEnabled: true
                onEntered: outputMonHelpIcon.showToolTip = true
                onExited: outputMonHelpIcon.showToolTip = false
            }

            ToolTip {
                visible: outputMonHelpIcon.showToolTip
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
                        text: qsTr("How loudly you hear yourself")
                        color: toolTipTextColour
                    }
                }
                background: Rectangle {
                    color: "transparent"
                }
            }
        }
    }

    Item {
        id: networkStatsHeader
        visible: showReadyScreen
        x: bodyMargin * virtualstudio.uiScale; y: 450 * virtualstudio.uiScale
        width: Math.min(parent.width / 2, 320 * virtualstudio.uiScale) - x
        height: 128 * virtualstudio.uiScale
        anchors.top: outputDevice.bottom

        Image {
            id: network
            source: "network.svg"
            x: 0; y: 0
            width: 28 * virtualstudio.uiScale; height: 28 * virtualstudio.uiScale
            sourceSize: Qt.size(network.width,network.height)
            fillMode: Image.PreserveAspectFit
            smooth: true
        }

        Colorize {
            anchors.fill: network
            source: network
            hue: 0
            saturation: 0
            lightness: imageLightnessValue
        }

        Text {
            id: networkStatsHeaderText
            text: "<b>Network</b>"
            font {family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            x: 64 * virtualstudio.uiScale
            anchors.verticalCenter: network.verticalCenter
            color: textColour
        }
    }

    Item {
        id: networkStatsText
        visible: showReadyScreen
        x: networkStatsHeader.x + networkStatsHeader.width; y: 450 * virtualstudio.uiScale
        width: parent.width - networkStatsHeader.width - 2 * bodyMargin * virtualstudio.uiScale
        height: 72 * virtualstudio.uiScale
        anchors.top: outputDevice.bottom

        Text {
            id: netstat0
            x: 0; y: 0
            text: getNetworkStatsText(virtualstudio.networkStats)[0]
            font {family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
        }

        Text {
            id: netstat1
            x: 0
            text: getNetworkStatsText(virtualstudio.networkStats)[1]
            font {family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
            topPadding: 8 * virtualstudio.uiScale
            anchors.top: netstat0.bottom
            color: textColour
        }
    }

    Item {
        id: devicesWarning
        visible: showReadyScreen && Boolean(virtualstudio.devicesWarning)
        x: bodyMargin * virtualstudio.uiScale
        width: parent.width - (2 * x)
        anchors.top: networkStatsText.bottom
        anchors.topMargin: 12 * virtualstudio.uiScale

        Text {
            x: 0; y: 0
            width: devicesWarning.width
            textFormat: Text.RichText
            text: (virtualstudio.devicesWarning)
                + ((virtualstudio.devicesWarningHelpUrl)
                    ? `&nbsp;<a style="color: ${linkText};" href=${virtualstudio.devicesWarningHelpUrl}>Learn More.</a>`
                    : ""
                )
            onLinkActivated: link => {
                virtualstudio.openLink(link)
            }
            horizontalAlignment: Text.AlignHLeft
            wrapMode: Text.WordWrap
            font {family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
            color: warningTextColour
        }
    }

    Item {
        id: waitingScreen
        visible: showWaitingScreen
        x: bodyMargin * virtualstudio.uiScale; y: 230 * virtualstudio.uiScale
        width: parent.width - (2 * x)

        property bool isAdmin: (virtualstudio.currentStudio >= 0 ? serverModel[virtualstudio.currentStudio].isAdmin : false)

        Text {
            id: waitingText0
            x: 0
            width: parent.width
            color: textColour
            font {family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            text: parent.isAdmin
                    ? "Waiting for this studio to start. Please start the studio using one of the options below."
                    : "This studio is currently inactive. Please contact an owner or admin for this studio to start it."
            wrapMode: Text.WordWrap
        }

        Item {
            id: startButtonsBox
            anchors.top: waitingText0.bottom
            anchors.topMargin: 16 * virtualstudio.uiScale
            anchors.bottomMargin: 16 * virtualstudio.uiScale
            visible: parent.isAdmin

            height: 64 * virtualstudio.uiScale

            Button {
                id: startStudioNowButton
                anchors.verticalCenter: startButtonsBox.verticalCenter
                x: 0
                onClicked: {
                    virtualstudio.manageStudio(-1, true)
                }

                width: 210 * virtualstudio.uiScale; height: 45 * virtualstudio.uiScale
                background: Rectangle {
                    radius: 6 * virtualstudio.uiScale
                    color: startStudioNowButton.down ? browserButtonPressedColour : (startStudioNowButton.hovered ? browserButtonHoverColour : browserButtonColour)
                    border.width: 1
                    border.color: startStudioNowButton.down ? browserButtonPressedStroke : (startStudioNowButton.hovered ? browserButtonHoverStroke : browserButtonStroke)
                }

                Text {
                    text: "Start Studio"
                    font.family: "Poppins"
                    font.pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    color: textColour
                }
            }

            Text {
                id: startStudioInBrowserText
                anchors.verticalCenter: startStudioNowButton.verticalCenter
                anchors.left: startStudioNowButton.right
                anchors.leftMargin: 24 * virtualstudio.uiScale
                width: 240 * virtualstudio.uiScale
                textFormat: Text.RichText
                text:`<a style="color: ${textColour};" href="https://${virtualstudio.apiHost}/studios/${virtualstudio.currentStudio >= 0 ? serverModel[virtualstudio.currentStudio].id : ""}/live?start=true">Change Settings and Start</a>`

                onLinkActivated: link => {
                    virtualstudio.openLink(link)
                }
                horizontalAlignment: Text.AlignHLeft
                wrapMode: Text.WordWrap
                font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            }
        }

        Text {
            id: waitingText1
            x: 0
            width: parent.width
            color: textColour
            anchors.top: parent.isAdmin ? startButtonsBox.bottom : waitingText0.bottom
            anchors.topMargin: 16 * virtualstudio.uiScale
            anchors.bottomMargin: 16 * virtualstudio.uiScale
            visible: parent.isAdmin
            font {family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            text: "You will be automatically connected to the studio when it is ready."
            wrapMode: Text.WordWrap
        }

        Text {
            id: connectedErrorMessage1
            x: 0
            width: parent.width
            color: warningTextColour
            anchors.top: waitingText1.bottom
            anchors.topMargin: 16 * virtualstudio.uiScale
            anchors.bottomMargin: 16 * virtualstudio.uiScale
            visible: parent.isAdmin && Boolean(virtualstudio.connectedErrorMsg)
            textFormat: Text.RichText
            text: virtualstudio.connectedErrorMsg == "one-studio-limit-reached"
                ? `Your current plan allows you to use 1 studio at a time. <a style="color: ${linkText}; cursor: pointer" href="https://help.jacktrip.org/hc/en-us/requests/new">Contact us</a> to use multiple studios at a time.`
                : ""
            onLinkActivated: link => {
                virtualstudio.openLink(link)
            }
            font {family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            wrapMode: Text.WordWrap
        }
    }

    Item {
        id: studioStartingScreen
        visible: showStartingScreen
        x: bodyMargin * virtualstudio.uiScale; y: 230 * virtualstudio.uiScale
        width: parent.width - (2 * x)

        Text {
            id: studioStartingText0
            x: 0
            width: parent.width
            color: textColour
            font {family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            text: "This studio is currently starting up. You will be connected automatically when it is ready."
            wrapMode: Text.WordWrap
        }
    }

    Item {
        id: studioStoppingScreen
        visible: showStoppingScreen
        x: bodyMargin * virtualstudio.uiScale; y: 230 * virtualstudio.uiScale
        width: parent.width - (2 * x)

        Text {
            id: studioStoppingText0
            x: 0
            width: parent.width
            color: textColour
            font {family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            text: "This studio is shutting down, please wait to start it again."
            wrapMode: Text.WordWrap
        }
    }
}
