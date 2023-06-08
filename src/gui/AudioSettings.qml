import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.12

Rectangle {
    width: parent.width
    height: parent.height
    color: Styles.background.defaultColor

    property int leftMargin: 48
    property int rightMargin: 16
    property int bottomToolTipMargin: 8
    property int rightToolTipMargin: 4
    property int buttonWidth: 103
    property int buttonHeight: 25

    property string buttonStroke: virtualstudio.darkMode ? "#80827D7D" : "#40979797"
    property string linkText: virtualstudio.darkMode ? "#8B8D8D" : "#272525"

    property string errorFlagColour: "#DB0A0A"
    property string disabledButtonTextColour: virtualstudio.darkMode ? "#827D7D" : "#BABCBC"

    property bool isUsingJack: virtualstudio.audioBackend == "JACK"
    property bool isUsingRtAudio: virtualstudio.audioBackend == "RtAudio"
    property bool hasNoBackend: !isUsingJack && !isUsingRtAudio && !virtualstudio.backendAvailable;

    property int inputCurrIndex: getCurrentInputDeviceIndex()
    property int outputCurrIndex: getCurrentOutputDeviceIndex()

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

    Loader {
        anchors.fill: parent
        sourceComponent: isUsingRtAudio ? usingRtAudio : (isUsingJack ? usingJACK : noBackend)
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
                font { family: "Poppins"; pixelSize: Styles.font.sm }
                color: Styles.text.color.standard
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
                        color: Styles.button.bgColor
                        radius: 3
                        anchors.fill: parent
                        anchors.bottomMargin: bottomToolTipMargin * virtualstudio.uiScale
                        anchors.rightMargin: rightToolTipMargin * virtualstudio.uiScale
                        layer.enabled: true
                        border.width: 1
                        border.color: Styles.button.strokeColor

                        Text {
                            anchors.centerIn: parent
                            font { family: "Poppins"; pixelSize: Styles.font.xs }
                            text: qsTr("How you'll hear the studio audio")
                            color: Styles.text.color.standard
                        }
                    }
                    background: Rectangle {
                        color: "transparent"
                    }
                }
            }

            Colorize {
                anchors.fill: outputHelpIcon
                source: outputHelpIcon
                hue: 0
                saturation: 0
                lightness: Styles.image.lightness
            }

            Image {
                id: headphonesIcon
                anchors.left: outputLabel.left
                anchors.verticalCenter: outputDeviceMeters.verticalCenter
                source: "headphones.svg"
                sourceSize: Qt.size(28 * virtualstudio.uiScale, 28 * virtualstudio.uiScale)
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            Colorize {
                anchors.fill: headphonesIcon
                source: headphonesIcon
                hue: 0
                saturation: 0
                lightness: Styles.image.lightness
            }

            ComboBox {
                id: outputCombo
                anchors.left: leftSpacer.right
                anchors.verticalCenter: outputLabel.verticalCenter
                anchors.rightMargin: rightMargin * virtualstudio.uiScale
                width: parent.width - leftSpacer.width - rightMargin * virtualstudio.uiScale
                model: outputComboModel
                currentIndex: outputCurrIndex
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

            Slider {
                id: outputSlider
                from: 0.0
                value: audioInterface ? audioInterface.outputVolume : 0.5
                onMoved: { audioInterface.outputVolume = value }
                to: 1.0
                padding: 0
                anchors.left: outputQuieterIcon.right
                anchors.leftMargin: 8 * virtualstudio.uiScale
                anchors.right: outputLouderIcon.left
                anchors.rightMargin: 8 * virtualstudio.uiScale
                anchors.top: outputDeviceMeters.bottom
                anchors.topMargin: 16 * virtualstudio.uiScale

                background: Rectangle {
                    x: outputSlider.leftPadding
                    y: outputSlider.topPadding + outputSlider.availableHeight / 2 - height / 2
                    implicitWidth: parent.width
                    implicitHeight: 6
                    width: outputSlider.availableWidth
                    height: implicitHeight
                    radius: 4
                    color: Styles.slider.trackInactiveColor

                    Rectangle {
                        width: outputSlider.visualPosition * parent.width
                        height: parent.height
                        color: Styles.slider.trackActiveColor
                        radius: 4
                    }
                }

                handle: Rectangle {
                    x: outputSlider.leftPadding + outputSlider.visualPosition * (outputSlider.availableWidth - width)
                    y: outputSlider.topPadding + outputSlider.availableHeight / 2 - height / 2
                    implicitWidth: 26 * virtualstudio.uiScale
                    implicitHeight: 26 * virtualstudio.uiScale
                    radius: 13 * virtualstudio.uiScale
                    color: outputSlider.pressed ? Styles.slider.bgPressedColor : Styles.slider.bgColor
                    border.color: buttonStroke
                }
            }

            Image {
                id: outputQuieterIcon
                anchors.left: outputCombo.left
                anchors.verticalCenter: outputSlider.verticalCenter
                source: "quiet.svg"
                sourceSize: Qt.size(16 * virtualstudio.uiScale, 16 * virtualstudio.uiScale)
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            Colorize {
                anchors.fill: outputQuieterIcon
                source: outputQuieterIcon
                hue: 0
                saturation: 0
                lightness: Styles.image.lightness
            }

            Image {
                id: outputLouderIcon
                anchors.right: parent.right
                anchors.rightMargin: rightMargin * virtualstudio.uiScale
                anchors.verticalCenter: outputSlider.verticalCenter
                source: "loud.svg"
                sourceSize: Qt.size(16 * virtualstudio.uiScale, 16 * virtualstudio.uiScale)
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            Colorize {
                anchors.fill: outputLouderIcon
                source: outputLouderIcon
                hue: 0
                saturation: 0
                lightness: Styles.image.lightness
            }

            Text {
                id: outputChannelsLabel
                anchors.left: outputCombo.left
                anchors.right: outputCombo.horizontalCenter
                anchors.top: outputSlider.bottom
                anchors.topMargin: 12 * virtualstudio.uiScale
                textFormat: Text.RichText
                text: "Output Channel(s)"
                font { family: "Poppins"; pixelSize: Styles.font.xs }
                color: Styles.text.color.standard
            }

            ComboBox {
                id: outputChannelsCombo
                anchors.left: outputCombo.left
                anchors.right: outputCombo.horizontalCenter
                anchors.rightMargin: 8 * virtualstudio.uiScale
                anchors.top: outputChannelsLabel.bottom
                anchors.topMargin: 4 * virtualstudio.uiScale
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

            Button {
                id: testOutputAudioButton
                background: Rectangle {
                    radius: 6 * virtualstudio.uiScale
                    color: testOutputAudioButton.down ? Styles.button.bgPressedColor : (testOutputAudioButton.hovered ? Styles.button.bgHoverColor : Styles.button.bgColor )
                    border.width: 1
                    border.color: testOutputAudioButton.down || testOutputAudioButton.hovered ? Styles.button.strokePressedColor : (testOutputAudioButton.hovered ? Styles.button.strokeHoverColor : Styles.button.strokeColor )
                }
                onClicked: { virtualstudio.playOutputAudio() }
                anchors.right: parent.right
                anchors.rightMargin: rightMargin * virtualstudio.uiScale
                anchors.verticalCenter: outputChannelsCombo.verticalCenter
                width: 144 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
                Text {
                    text: "Play Test Tone"
                    font { family: "Poppins"; pixelSize: Styles.font.xs }
                    anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
                    color: Styles.text.color.standard
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
                font { family: "Poppins"; pixelSize: Styles.font.sm }
                color: Styles.text.color.standard
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
                        color: Styles.button.bgColor
                        radius: 3
                        anchors.fill: parent
                        anchors.bottomMargin: bottomToolTipMargin * virtualstudio.uiScale
                        anchors.rightMargin: rightToolTipMargin * virtualstudio.uiScale
                        layer.enabled: true
                        border.width: 1
                        border.color: Styles.button.strokeColor

                        Text {
                            anchors.centerIn: parent
                            font { family: "Poppins"; pixelSize: Styles.font.xs }
                            text: qsTr("Audio sent to the studio (microphone, instrument, mixer, etc.)")
                            color: Styles.text.color.standard
                        }
                    }
                    background: Rectangle {
                        color: "transparent"
                    }
                }
            }

            Colorize {
                anchors.fill: inputHelpIcon
                source: inputHelpIcon
                hue: 0
                saturation: 0
                lightness: Styles.image.lightness
            }

            Image {
                id: microphoneIcon
                anchors.left: outputLabel.left
                anchors.verticalCenter: inputDeviceMeters.verticalCenter
                source: "mic.svg"
                sourceSize: Qt.size(32 * virtualstudio.uiScale, 32 * virtualstudio.uiScale)
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            Colorize {
                anchors.fill: microphoneIcon
                source: microphoneIcon
                hue: 0
                saturation: 0
                lightness: Styles.image.lightness
            }

            ComboBox {
                id: inputCombo
                model: inputComboModel
                currentIndex: inputCurrIndex
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

            Slider {
                id: inputSlider
                from: 0.0
                value: audioInterface ? audioInterface.inputVolume : 0.5
                onMoved: { audioInterface.inputVolume = value }
                to: 1.0
                padding: 0
                anchors.left: inputQuieterIcon.right
                anchors.leftMargin: 8 * virtualstudio.uiScale
                anchors.right: inputLouderIcon.left
                anchors.rightMargin: 8 * virtualstudio.uiScale
                anchors.top: inputDeviceMeters.bottom
                anchors.topMargin: 16 * virtualstudio.uiScale

                background: Rectangle {
                    x: inputSlider.leftPadding
                    y: inputSlider.topPadding + inputSlider.availableHeight / 2 - height / 2
                    implicitWidth: parent.width
                    implicitHeight: 6
                    width: inputSlider.availableWidth
                    height: implicitHeight
                    radius: 4
                    color: Styles.slider.trackInactiveColor

                    Rectangle {
                        width: inputSlider.visualPosition * parent.width
                        height: parent.height
                        color: Styles.slider.trackActiveColor
                        radius: 4
                    }
                }

                handle: Rectangle {
                    x: inputSlider.leftPadding + inputSlider.visualPosition * (inputSlider.availableWidth - width)
                    y: inputSlider.topPadding + inputSlider.availableHeight / 2 - height / 2
                    implicitWidth: 26 * virtualstudio.uiScale
                    implicitHeight: 26 * virtualstudio.uiScale
                    radius: 13 * virtualstudio.uiScale
                    color: inputSlider.pressed ? Styles.slider.bgPressedColor : Styles.slider.bgColor
                    border.color: buttonStroke
                }
            }

            Image {
                id: inputQuieterIcon
                anchors.left: inputDeviceMeters.left
                anchors.verticalCenter: inputSlider.verticalCenter
                source: "quiet.svg"
                sourceSize: Qt.size(16 * virtualstudio.uiScale, 16 * virtualstudio.uiScale)
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            Colorize {
                anchors.fill: inputQuieterIcon
                source: inputQuieterIcon
                hue: 0
                saturation: 0
                lightness: Styles.image.lightness
            }

            Image {
                id: inputLouderIcon
                anchors.right: parent.right
                anchors.rightMargin: rightMargin * virtualstudio.uiScale
                anchors.verticalCenter: inputSlider.verticalCenter
                source: "loud.svg"
                sourceSize: Qt.size(16 * virtualstudio.uiScale, 16 * virtualstudio.uiScale)
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            Colorize {
                anchors.fill: inputLouderIcon
                source: inputLouderIcon
                hue: 0
                saturation: 0
                lightness: Styles.image.lightness
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
                font { family: "Poppins"; pixelSize: Styles.font.xs }
                color: Styles.text.color.standard
            }

            ComboBox {
                id: inputChannelsCombo
                anchors.left: inputCombo.left
                anchors.right: inputCombo.horizontalCenter
                anchors.rightMargin: 8 * virtualstudio.uiScale
                anchors.top: inputChannelsLabel.bottom
                anchors.topMargin: 4 * virtualstudio.uiScale
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
                anchors.top: inputSlider.bottom
                anchors.topMargin: 12 * virtualstudio.uiScale
                textFormat: Text.RichText
                text: "Mono / Stereo"
                font { family: "Poppins"; pixelSize: Styles.font.xs }
                color: Styles.text.color.standard
            }

            ComboBox {
                id: inputMixModeCombo
                anchors.left: inputCombo.horizontalCenter
                anchors.right: inputCombo.right
                anchors.rightMargin: 8 * virtualstudio.uiScale
                anchors.top: inputMixModeLabel.bottom
                anchors.topMargin: 4 * virtualstudio.uiScale
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
                font { family: "Poppins"; pixelSize: Styles.font.xs }
                color: Styles.text.color.standard
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
                font { family: "Poppins"; pixelSize: Styles.font.xs }
                color: Styles.text.color.standard
            }

            Text {
                id: warningOrErrorMessage
                anchors.left: inputLabel.left
                anchors.right: parent.right
                anchors.rightMargin: 16 * virtualstudio.uiScale
                anchors.top: inputMixModeHelpMessage.bottom
                anchors.topMargin: 8 * virtualstudio.uiScale
                anchors.bottomMargin: 8 * virtualstudio.uiScale
                textFormat: Text.RichText
                text: (virtualstudio.devicesError || virtualstudio.devicesWarning)
                    + ((virtualstudio.devicesErrorHelpUrl || virtualstudio.devicesWarningHelpUrl)
                        ? `&nbsp;<a style="color: ${linkText};" href=${virtualstudio.devicesErrorHelpUrl || virtualstudio.devicesWarningHelpUrl}>Learn More.</a>`
                        : ""
                    )
                onLinkActivated: link => {
                    virtualstudio.openLink(link)
                }
                horizontalAlignment: Text.AlignHLeft
                wrapMode: Text.WordWrap
                color: Styles.text.color.warning
                font { family: "Poppins"; pixelSize: Styles.font.xs }
                visible: Boolean(virtualstudio.devicesError) || Boolean(virtualstudio.devicesWarning);
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
                font { family: "Poppins"; pixelSize: Styles.font.sm }
                wrapMode: Text.WordWrap
                color: Styles.text.color.standard
            }

            Text {
                id: jackOutputLabel
                anchors.left: jackLabel.left
                anchors.top: jackLabel.bottom
                anchors.topMargin: 48 * virtualstudio.uiScale
                width: 144 * virtualstudio.uiScale
                text: "Output Volume"
                font { family: "Poppins"; pixelSize: Styles.font.sm }
                wrapMode: Text.WordWrap
                color: Styles.text.color.standard
            }

            Image {
                id: jackHeadphonesIcon
                anchors.left: jackOutputLabel.left
                anchors.verticalCenter: jackOutputVolumeSlider.verticalCenter
                source: "headphones.svg"
                sourceSize: Qt.size(28 * virtualstudio.uiScale, 28 * virtualstudio.uiScale)
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            Colorize {
                anchors.fill: jackHeadphonesIcon
                source: jackHeadphonesIcon
                hue: 0
                saturation: 0
                lightness: Styles.image.lightness
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
                    color: jackTestOutputAudioButton.down ? Styles.button.bgPressedColor : (jackTestOutputAudioButton.hovered ? Styles.button.bgHoverColor : Styles.button.bgColor )
                    border.width: 1
                    border.color: jackTestOutputAudioButton.down || jackTestOutputAudioButton.hovered ? Styles.button.strokePressedColor : (jackTestOutputAudioButton.hovered ? Styles.button.strokeHoverColor : Styles.button.strokeColor )
                }
                onClicked: { virtualstudio.playOutputAudio() }
                anchors.right: parent.right
                anchors.rightMargin: rightMargin * virtualstudio.uiScale
                anchors.verticalCenter: jackOutputVolumeSlider.verticalCenter
                width: 144 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
                Text {
                    text: "Play Test Tone"
                    font { family: "Poppins"; pixelSize: Styles.font.xs }
                    anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
                    color: Styles.text.color.standard
                }
            }

            Slider {
                id: jackOutputVolumeSlider
                from: 0.0
                value: audioInterface ? audioInterface.outputVolume : 0.5
                onMoved: { audioInterface.outputVolume = value }
                to: 1.0
                padding: 0
                anchors.left: jackOutputQuieterButton.right
                anchors.leftMargin: 8 * virtualstudio.uiScale
                anchors.right: jackOutputLouderIcon.left
                anchors.rightMargin: 8 * virtualstudio.uiScale
                anchors.top: jackOutputMeters.bottom
                anchors.topMargin: 16 * virtualstudio.uiScale

                background: Rectangle {
                    x: jackOutputVolumeSlider.leftPadding
                    y: jackOutputVolumeSlider.topPadding + jackOutputVolumeSlider.availableHeight / 2 - height / 2
                    implicitWidth: parent.width
                    implicitHeight: 6
                    width: jackOutputVolumeSlider.availableWidth
                    height: implicitHeight
                    radius: 4
                    color: Styles.slider.trackInactiveColor

                    Rectangle {
                        width: jackOutputVolumeSlider.visualPosition * parent.width
                        height: parent.height
                        color: Styles.slider.trackActiveColor
                        radius: 4
                    }
                }

                handle: Rectangle {
                    x: jackOutputVolumeSlider.leftPadding + jackOutputVolumeSlider.visualPosition * (jackOutputVolumeSlider.availableWidth - width)
                    y: jackOutputVolumeSlider.topPadding + jackOutputVolumeSlider.availableHeight / 2 - height / 2
                    implicitWidth: 26 * virtualstudio.uiScale
                    implicitHeight: 26 * virtualstudio.uiScale
                    radius: 13 * virtualstudio.uiScale
                    color: jackOutputVolumeSlider.pressed ? Styles.slider.bgPressedColor : Styles.slider.bgColor
                    border.color: buttonStroke
                }
            }

            Image {
                id: jackOutputQuieterButton
                anchors.left: jackOutputMeters.left
                anchors.verticalCenter: jackOutputVolumeSlider.verticalCenter
                source: "quiet.svg"
                sourceSize: Qt.size(16 * virtualstudio.uiScale, 16 * virtualstudio.uiScale)
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            Colorize {
                anchors.fill: jackOutputQuieterButton
                source: jackOutputQuieterButton
                hue: 0
                saturation: 0
                lightness: Styles.image.lightness
            }

            Image {
                id: jackOutputLouderIcon
                anchors.right: jackTestOutputAudioButton.left
                anchors.rightMargin: rightMargin * virtualstudio.uiScale
                anchors.verticalCenter: jackOutputVolumeSlider.verticalCenter
                source: "loud.svg"
                sourceSize: Qt.size(16 * virtualstudio.uiScale, 16 * virtualstudio.uiScale)
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            Colorize {
                anchors.fill: jackOutputLouderIcon
                source: jackOutputLouderIcon
                hue: 0
                saturation: 0
                lightness: Styles.image.lightness
            }

            Text {
                id: jackInputLabel
                anchors.left: jackLabel.left
                anchors.top: jackOutputVolumeSlider.bottom
                anchors.topMargin: 48 * virtualstudio.uiScale
                width: 144 * virtualstudio.uiScale
                text: "Input Volume"
                font { family: "Poppins"; pixelSize: Styles.font.sm }
                wrapMode: Text.WordWrap
                color: Styles.text.color.standard
            }

            Image {
                id: jackMicrophoneIcon
                anchors.left: jackInputLabel.left
                anchors.verticalCenter: jackInputVolumeSlider.verticalCenter
                source: "mic.svg"
                sourceSize: Qt.size(32 * virtualstudio.uiScale, 32 * virtualstudio.uiScale)
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            Colorize {
                anchors.fill: jackMicrophoneIcon
                source: jackMicrophoneIcon
                hue: 0
                saturation: 0
                lightness: Styles.image.lightness
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

            Slider {
                id: jackInputVolumeSlider
                from: 0.0
                value: audioInterface ? audioInterface.inputVolume : 0.5
                onMoved: { audioInterface.inputVolume = value }
                to: 1.0
                padding: 0
                anchors.left: jackInputQuieterButton.right
                anchors.leftMargin: 8 * virtualstudio.uiScale
                anchors.right: jackInputLouderIcon.left
                anchors.rightMargin: 8 * virtualstudio.uiScale
                anchors.top: jackInputMeters.bottom
                anchors.topMargin: 16 * virtualstudio.uiScale

                background: Rectangle {
                    x: jackInputVolumeSlider.leftPadding
                    y: jackInputVolumeSlider.topPadding + jackInputVolumeSlider.availableHeight / 2 - height / 2
                    implicitWidth: parent.width
                    implicitHeight: 6
                    width: jackInputVolumeSlider.availableWidth
                    height: implicitHeight
                    radius: 4
                    color: Styles.slider.trackInactiveColor

                    Rectangle {
                        width: jackInputVolumeSlider.visualPosition * parent.width
                        height: parent.height
                        color: Styles.slider.trackActiveColor
                        radius: 4
                    }
                }

                handle: Rectangle {
                    x: jackInputVolumeSlider.leftPadding + jackInputVolumeSlider.visualPosition * (jackInputVolumeSlider.availableWidth - width)
                    y: jackInputVolumeSlider.topPadding + jackInputVolumeSlider.availableHeight / 2 - height / 2
                    implicitWidth: 26 * virtualstudio.uiScale
                    implicitHeight: 26 * virtualstudio.uiScale
                    radius: 13 * virtualstudio.uiScale
                    color: jackInputVolumeSlider.pressed ? Styles.slider.bgPressedColor : Styles.slider.bgColor
                    border.color: buttonStroke
                }
            }

            Image {
                id: jackInputQuieterButton
                anchors.left: jackInputMeters.left
                anchors.verticalCenter: jackInputVolumeSlider.verticalCenter
                source: "quiet.svg"
                sourceSize: Qt.size(16 * virtualstudio.uiScale, 16 * virtualstudio.uiScale)
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            Colorize {
                anchors.fill: jackInputQuieterButton
                source: jackInputQuieterButton
                hue: 0
                saturation: 0
                lightness: Styles.image.lightness
            }

            Image {
                id: jackInputLouderIcon
                anchors.right: parent.right
                anchors.rightMargin: rightMargin * virtualstudio.uiScale
                anchors.verticalCenter: jackInputVolumeSlider.verticalCenter
                source: "loud.svg"
                sourceSize: Qt.size(16 * virtualstudio.uiScale, 16 * virtualstudio.uiScale)
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            Colorize {
                anchors.fill: jackInputLouderIcon
                source: jackInputLouderIcon
                hue: 0
                saturation: 0
                lightness: Styles.image.lightness
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
                font { family: "Poppins"; pixelSize: Styles.font.md }
                wrapMode: Text.WordWrap
                color: Styles.text.color.standard
            }
        }
    }
}
