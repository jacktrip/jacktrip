import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.12

Item {
    width: parent.width; height: parent.height
    clip: true

    Rectangle {
        width: parent.width; height: parent.height
        color: backgroundColour
    }

    property int fontBig: 20
    property int fontMedium: 13
    property int fontSmall: 11
    property int fontExtraSmall: 8

    property int leftMargin: 48
    property int rightMargin: 16
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
    property string sliderColour: virtualstudio.darkMode ? "#BABCBC" :  "#EAECEC"
    property string sliderPressedColour: virtualstudio.darkMode ? "#ACAFAF" : "#DEE0E0"
    property string sliderTrackColour: virtualstudio.darkMode ? "#5B5858" : "light gray"
    property string sliderActiveTrackColour: virtualstudio.darkMode ? "light gray" : "black"
    property string warningTextColour: "#DB0A0A"
    property string linkText: virtualstudio.darkMode ? "#8B8D8D" : "#272525"

    property string errorFlagColour: "#DB0A0A"
    property string disabledButtonTextColour: virtualstudio.darkMode ? "#827D7D" : "#BABCBC"

    property string settingsGroupView: "Audio"

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

    ToolBar {
        id: header
        width: parent.width
        height: 64 * virtualstudio.uiScale

        background: Rectangle {
            border.color: "#33979797"
            color: backgroundColour
            width: parent.width
        }

        contentItem: Item {
            id: headerContent
            width: header.width
            height: header.height
            x: 16 * virtualstudio.uiScale; y: 32 * virtualstudio.uiScale

            property bool isUsingRtAudio: virtualstudio.audioBackend == "RtAudio"

            Label {
                id: pageTitle
                text: "Settings"
                height: headerContent.height;
                anchors.left: headerContent.left;
                anchors.leftMargin: 32 * virtualstudio.uiScale
                elide: Label.ElideRight
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font { family: "Poppins"; weight: Font.Bold; pixelSize: fontBig * virtualstudio.fontScale * virtualstudio.uiScale }
                color: textColour
            }

            Button {
                id: refreshButton
                text: "Refresh Devices"
                // anchors.right: header.right
                // anchors.rightMargin: rightMargin * virtualstudio.uiScale

                anchors.verticalCenter: pageTitle.verticalCenter;
                anchors.right: headerContent.right;
                anchors.rightMargin: 16 * virtualstudio.uiScale;

                palette.buttonText: textColour
                background: Rectangle {
                    radius: 6 * virtualstudio.uiScale
                    color: refreshButton.down ? buttonPressedColour : (refreshButton.hovered ? buttonHoverColour : buttonColour)
                    border.width: 1
                    border.color: refreshButton.down ? buttonPressedStroke : (refreshButton.hovered ? buttonHoverStroke : buttonStroke)
                }
                icon {
                    source: "refresh.svg";
                    color: textColour;
                }
                display: AbstractButton.TextBesideIcon
                onClicked: {
                    virtualstudio.refreshDevices();
                    inputCombo.currentIndex = getCurrentInputDeviceIndex();
                    outputCombo.currentIndex = getCurrentOutputDeviceIndex();
                }
                width: 144 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
                font {
                    family: "Poppins"
                    pixelSize: fontExtraSmall * virtualstudio.fontScale * virtualstudio.uiScale
                }
                visible: parent.isUsingRtAudio
            }

        }


    }

    Drawer {
        id: drawer
        width: 0.2 * parent.width
        height: parent.height - header.height
        y: header.height-1
        modal: false
        interactive: false
        visible: virtualstudio.windowState == "settings"

        background: Rectangle {
            border.color: "#33979797"
            color: backgroundColour
        }

        ButtonGroup {
            buttons: viewControls.children
            onClicked: { settingsGroupView = button.text }
        }

        Column {
            id: viewControls
            width: parent.width
            spacing: 24 * virtualstudio.uiScale
            anchors.centerIn: parent
            Button {
                id: audioBtn
                text: "Audio"
                width: parent.width
                contentItem: Item {
                    implicitWidth: audioButtonText.implicitWidth
                    implicitHeight: audioButtonText.implicitHeight

                    Label {
                        id: audioButtonText
                        text: audioBtn.text
                        width: Boolean(virtualstudio.devicesError) ? parent.width - 16 * virtualstudio.uiScale : parent.width
                        font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        color: textColour
                    }

                    Rectangle {
                        id: audioDevicesErrorFlag
                        anchors.left: audioButtonText.right
                        anchors.verticalCenter: audioButtonText.verticalCenter
                        anchors.rightMargin: 16 * virtualstudio.uiScale
                        width: 8 * virtualstudio.uiScale 
                        height: 8 * virtualstudio.uiScale 
                        color: errorFlagColour
                        radius: 4 * virtualstudio.uiScale
                        visible: Boolean(virtualstudio.devicesError)
                    }
                }
                background: Rectangle {
                    width: parent.width
                    color: audioBtn.down ? buttonPressedColour : (audioBtn.hovered || settingsGroupView == "Audio" ? buttonHoverColour : backgroundColour)
                }
            }
            Button {
                id: appearanceBtn
                text: "Appearance"
                width: parent.width
                contentItem: Item {
                    implicitWidth: appearanceButtonText.implicitWidth
                    implicitHeight: appearanceButtonText.implicitHeight

                    Label {
                        id: appearanceButtonText
                        text: appearanceBtn.text
                        width: parent.width
                        font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        color: textColour
                    }
                }
                

                background: Rectangle {
                    width: parent.width
                    color: appearanceBtn.down ? buttonPressedColour : (appearanceBtn.hovered || settingsGroupView == "Appearance" ? buttonHoverColour : backgroundColour)
                }
            }
            Button {
                id: advancedBtn
                text: "Advanced"
                width: parent.width
                contentItem: Item {
                    implicitWidth: advancedButtonText.implicitWidth
                    implicitHeight: advancedButtonText.implicitHeight

                    Label {
                        id: advancedButtonText
                        text: advancedBtn.text
                        width: parent.width
                        font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        color: textColour
                    }
                }
                background: Rectangle {
                    width: parent.width
                    color: advancedBtn.down ? buttonPressedColour : (advancedBtn.hovered || settingsGroupView == "Advanced" ? buttonHoverColour : backgroundColour)
                }
            }
            Button {
                id: profileBtn
                text: "Profile"
                width: parent.width
                contentItem: Item {

                    implicitWidth: profileButtonText.implicitWidth
                    implicitHeight: profileButtonText.implicitHeight

                    Label {
                        id: profileButtonText
                        text: profileBtn.text
                        width: parent.width
                        font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        color: textColour
                    }
                }
                background: Rectangle {
                    width: parent.width
                    color: profileBtn.down ? buttonPressedColour : (profileBtn.hovered || settingsGroupView == "Profile" ? buttonHoverColour : backgroundColour)
                }
            }
        }

        Column {
            id: appVersion
            width: parent.width
            spacing: 24 * virtualstudio.uiScale
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom

            Text {
                text: "Version " + virtualstudio.versionString
                font { family: "Poppins"; pixelSize: 9 * virtualstudio.fontScale * virtualstudio.uiScale}
                color: textColour
                opacity: 0.8
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    Rectangle {
        id: audioSettingsView
        width: 0.8 * parent.width
        height: parent.height - header.height
        x: 0.2 * window.width
        y: header.height
        color: backgroundColour
        visible: settingsGroupView == "Audio"

        property bool isUsingJack: virtualstudio.audioBackend == "JACK"
        property bool isUsingRtAudio: virtualstudio.audioBackend == "RtAudio"
        property bool hasNoBackend: !isUsingJack && !isUsingRtAudio && !virtualstudio.backendAvailable;

        Item {
            id: usingRtAudio
            anchors.top: parent.top
            anchors.topMargin: 32 * virtualstudio.uiScale
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.leftMargin: 24 * virtualstudio.uiScale
            anchors.right: parent.right

            visible: parent.isUsingRtAudio

            Text {
                id: outputLabel
                x: 0; y: 0
                width: 144 * virtualstudio.uiScale
                text: "Output Device"
                font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                color: textColour
            }

            Image {
                id: headphonesIcon
                anchors.left: outputLabel.left
                anchors.verticalCenter: outputDeviceMeters.verticalCenter
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
                anchors.left: outputLabel.right
                anchors.verticalCenter: outputLabel.verticalCenter
                anchors.rightMargin: rightMargin * virtualstudio.uiScale
                width: parent.width - outputLabel.width - rightMargin * virtualstudio.uiScale
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
                model: outputMeterModel
                clipped: outputClipped
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

            Image {
                id: outputQuieterIcon
                anchors.left: outputCombo.left
                anchors.verticalCenter: outputSlider.verticalCenter
                source: "quiet.svg"
                sourceSize: Qt.size(16 * virtualstudio.uiScale, 16 * virtualstudio.uiScale)
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

            Image {
                id: outputLouderIcon
                anchors.right: testOutputAudioButton.left
                anchors.rightMargin: 16 * virtualstudio.uiScale
                anchors.verticalCenter: outputSlider.verticalCenter
                source: "loud.svg"
                sourceSize: Qt.size(16 * virtualstudio.uiScale, 16 * virtualstudio.uiScale)
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
                anchors.verticalCenter: outputSlider.verticalCenter
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
                anchors.right: outputLabel.right
                anchors.top: divider1.bottom
                anchors.topMargin: 32 * virtualstudio.uiScale
                text: "Input Device"
                font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                color: textColour
            }

            Image {
                id: microphoneIcon
                anchors.left: outputLabel.left
                anchors.verticalCenter: inputDeviceMeters.verticalCenter
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
                model: inputMeterModel
                clipped: inputClipped
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

                Colorize {
                    anchors.fill: parent
                    source: parent
                    hue: 0
                    saturation: 0
                    lightness: virtualstudio.darkMode ? 1 : 0
                }
            }

            Image {
                id: inputLouderIcon
                anchors.right: hiddenInputButton.left
                anchors.rightMargin: rightMargin * virtualstudio.uiScale
                anchors.verticalCenter: inputSlider.verticalCenter
                source: "loud.svg"
                sourceSize: Qt.size(16 * virtualstudio.uiScale, 16 * virtualstudio.uiScale)
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
                anchors.topMargin: 24 * virtualstudio.uiScale
                textFormat: Text.RichText
                text: "Input Channel(s)"
                font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                color: textColour
            }

            ComboBox {
                id: inputChannelsCombo
                anchors.left: inputCombo.left
                anchors.right: inputCombo.horizontalCenter
                anchors.rightMargin: 8 * virtualstudio.uiScale
                anchors.top: inputChannelsLabel.bottom
                anchors.topMargin: 16 * virtualstudio.uiScale
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
                anchors.topMargin: 24 * virtualstudio.uiScale
                textFormat: Text.RichText
                text: "Mono / Stereo"
                font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                color: textColour
            }

            ComboBox {
                id: inputMixModeCombo
                anchors.left: inputCombo.horizontalCenter
                anchors.right: inputCombo.right
                anchors.rightMargin: 8 * virtualstudio.uiScale
                anchors.top: inputMixModeLabel.bottom
                anchors.topMargin: 16 * virtualstudio.uiScale
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
                color: warningTextColour
                font { family: "Poppins"; pixelSize: fontExtraSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                visible: Boolean(virtualstudio.devicesError) || Boolean(virtualstudio.devicesWarning);
            }
        }

        Item {
            id: usingJACK
            anchors.top: parent.top
            anchors.topMargin: 32 * virtualstudio.uiScale
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.leftMargin: leftMargin * virtualstudio.uiScale
            anchors.right: parent.right

            visible: parent.isUsingJack

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

            Image {
                id: jackHeadphonesIcon
                anchors.left: jackOutputLabel.left
                anchors.verticalCenter: jackOutputVolumeSlider.verticalCenter
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

            Meter {
                id: jackOutputMeters
                anchors.left: jackOutputLabel.right
                anchors.right: parent.right
                anchors.rightMargin: rightMargin * virtualstudio.uiScale
                anchors.verticalCenter: jackOutputLabel.verticalCenter
                height: 24 * virtualstudio.uiScale
                model: outputMeterModel
                clipped: outputClipped
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
                    color: sliderTrackColour

                    Rectangle {
                        width: jackOutputVolumeSlider.visualPosition * parent.width
                        height: parent.height
                        color: sliderActiveTrackColour
                        radius: 4
                    }
                }

                handle: Rectangle {
                    x: jackOutputVolumeSlider.leftPadding + jackOutputVolumeSlider.visualPosition * (jackOutputVolumeSlider.availableWidth - width)
                    y: jackOutputVolumeSlider.topPadding + jackOutputVolumeSlider.availableHeight / 2 - height / 2
                    implicitWidth: 26 * virtualstudio.uiScale
                    implicitHeight: 26 * virtualstudio.uiScale
                    radius: 13 * virtualstudio.uiScale
                    color: jackOutputVolumeSlider.pressed ? sliderPressedColour : sliderColour
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

                Colorize {
                    anchors.fill: parent
                    source: parent
                    hue: 0
                    saturation: 0
                    lightness: virtualstudio.darkMode ? 1 : 0
                }
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

                Colorize {
                    anchors.fill: parent
                    source: parent
                    hue: 0
                    saturation: 0
                    lightness: virtualstudio.darkMode ? 1 : 0
                }
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

            Image {
                id: jackMicrophoneIcon
                anchors.left: jackInputLabel.left
                anchors.verticalCenter: jackInputVolumeSlider.verticalCenter
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

            Meter {
                id: jackInputMeters
                anchors.left: jackInputLabel.right
                anchors.right: parent.right
                anchors.rightMargin: rightMargin * virtualstudio.uiScale
                anchors.verticalCenter: jackInputLabel.verticalCenter
                height: 24 * virtualstudio.uiScale
                model: inputMeterModel
                clipped: inputClipped
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
                    color: sliderTrackColour

                    Rectangle {
                        width: jackInputVolumeSlider.visualPosition * parent.width
                        height: parent.height
                        color: sliderActiveTrackColour
                        radius: 4
                    }
                }

                handle: Rectangle {
                    x: jackInputVolumeSlider.leftPadding + jackInputVolumeSlider.visualPosition * (jackInputVolumeSlider.availableWidth - width)
                    y: jackInputVolumeSlider.topPadding + jackInputVolumeSlider.availableHeight / 2 - height / 2
                    implicitWidth: 26 * virtualstudio.uiScale
                    implicitHeight: 26 * virtualstudio.uiScale
                    radius: 13 * virtualstudio.uiScale
                    color: jackInputVolumeSlider.pressed ? sliderPressedColour : sliderColour
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

                Colorize {
                    anchors.fill: parent
                    source: parent
                    hue: 0
                    saturation: 0
                    lightness: virtualstudio.darkMode ? 1 : 0
                }
            }

            Image {
                id: jackInputLouderIcon
                anchors.right: jackHiddenInputButton.left
                anchors.rightMargin: rightMargin * virtualstudio.uiScale
                anchors.verticalCenter: jackInputVolumeSlider.verticalCenter
                source: "loud.svg"
                sourceSize: Qt.size(16 * virtualstudio.uiScale, 16 * virtualstudio.uiScale)
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

            Button {
                id: jackHiddenInputButton
                anchors.right: parent.right
                anchors.rightMargin: rightMargin * virtualstudio.uiScale
                anchors.verticalCenter: jackInputVolumeSlider.verticalCenter
                width: 144 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
                visible: false
            }

        }

        Item {
            id: noBackend
            anchors.top: parent.top
            anchors.topMargin: 32 * virtualstudio.uiScale
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.leftMargin: leftMargin * virtualstudio.uiScale
            anchors.right: parent.right
            
            visible: parent.hasNoBackend

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

    Rectangle {
        id: appearanceSettingsView
        width: 0.8 * parent.width
        height: parent.height - header.height
        x: 0.2 * window.width
        y: header.height
        color: backgroundColour
        visible: settingsGroupView == "Appearance"

        Slider {
            id: scaleSlider
            x: 234 * virtualstudio.uiScale; y: 100 * virtualstudio.uiScale
            width: backendCombo.width
            from: 1; to: 2; value: virtualstudio.uiScale
            onMoved: { virtualstudio.uiScale = value }

            background: Rectangle {
                x: scaleSlider.leftPadding
                y: scaleSlider.topPadding + scaleSlider.availableHeight / 2 - height / 2
                implicitWidth: parent.width
                implicitHeight: 6
                width: scaleSlider.availableWidth
                height: implicitHeight
                radius: 4
                color: sliderTrackColour

                Rectangle {
                    width: scaleSlider.visualPosition * parent.width
                    height: parent.height
                    color: sliderActiveTrackColour
                    radius: 4
                }
            }

            handle: Rectangle {
                x: scaleSlider.leftPadding + scaleSlider.visualPosition * (scaleSlider.availableWidth - width)
                y: scaleSlider.topPadding + scaleSlider.availableHeight / 2 - height / 2
                implicitWidth: 26 * virtualstudio.uiScale
                implicitHeight: 26 * virtualstudio.uiScale
                radius: 13 * virtualstudio.uiScale
                color: scaleSlider.pressed ? sliderPressedColour : sliderColour
                border.color: buttonStroke
            }
        }

        Text {
            anchors.verticalCenter: scaleSlider.verticalCenter
            x: leftMargin * virtualstudio.uiScale
            text: "Scale Interface"
            font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
        }

        Button {
            id: darkButton
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: darkButton.down ? buttonPressedColour : (darkButton.hovered ? buttonHoverColour : buttonColour)
                border.width: 1
                border.color: darkButton.down ? buttonPressedStroke : (darkButton.hovered ? buttonHoverStroke : buttonStroke)
            }
            onClicked: { virtualstudio.darkMode = !virtualstudio.darkMode; }
            x: parent.width - (232 * virtualstudio.uiScale); y: modeButton.y + (56 * virtualstudio.uiScale)
            width: 216 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
            Text {
                text: virtualstudio.darkMode ? "Switch to Light Mode" : "Switch to Dark Mode"
                font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
                color: textColour
            }
        }

        Text {
            anchors.verticalCenter: darkButton.verticalCenter
            x: leftMargin * virtualstudio.uiScale
            text: "Color Theme"
            font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
        }
    }

    Rectangle {
        id: advancedSettingsView
        width: 0.8 * parent.width
        height: parent.height - header.height
        x: 0.2 * window.width
        y: header.height
        color: backgroundColour
        visible: settingsGroupView == "Advanced"

        Button {
            id: modeButton
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: modeButton.down ? buttonPressedColour : (modeButton.hovered ? buttonHoverColour : buttonColour)
                border.width: 1
                border.color: modeButton.down ? buttonPressedStroke : (modeButton.hovered ? buttonHoverStroke : buttonStroke)
            }
            onClicked: { virtualstudio.windowState = "login"; virtualstudio.toStandard(); }
            x: 234 * virtualstudio.uiScale; y: 100 * virtualstudio.uiScale
            width: 216 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
            Text {
                text: virtualstudio.psiBuild ? "Switch to Standard Mode" : "Switch to Classic Mode"
                font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
                color: textColour
            }
        }

        Text {
            anchors.verticalCenter: modeButton.verticalCenter
            x: leftMargin * virtualstudio.uiScale
            text: "Display Mode"
            font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
        }

        ComboBox {
            id: updateChannelCombo
            x: 234 * virtualstudio.uiScale; y: modeButton.y + (48 * virtualstudio.uiScale)
            width: parent.width - x - (16 * virtualstudio.uiScale); height: 36 * virtualstudio.uiScale
            model: updateChannelComboModel
            currentIndex: virtualstudio.updateChannel == "stable" ? 0 : 1
            onActivated: { virtualstudio.updateChannel = currentIndex == 0 ? "stable": "edge" }
            font.family: "Poppins"
            visible: !virtualstudio.noUpdater
        }

        Text {
            anchors.verticalCenter: updateChannelCombo.verticalCenter
            x: leftMargin * virtualstudio.uiScale
            text: "Update Channel"
            font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            visible: !virtualstudio.noUpdater
        }

        ComboBox {
            id: backendCombo
            model: backendComboModel
            currentIndex: virtualstudio.audioBackend == "JACK" ? 0 : 1
            onActivated: { virtualstudio.audioBackend = currentText }
            x: 234 * virtualstudio.uiScale; y: updateChannelCombo.y + (48 * virtualstudio.uiScale)
            width: updateChannelCombo.width; height: updateChannelCombo.height
            visible: virtualstudio.selectableBackend
        }

        Text {
            id: backendLabel
            anchors.verticalCenter: backendCombo.verticalCenter
            x: leftMargin * virtualstudio.uiScale
            text: "Audio Backend"
            font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            visible: virtualstudio.selectableBackend
            color: textColour
        }

        ComboBox {
            id: bufferCombo
            x: 234 * virtualstudio.uiScale; y: backendCombo.y + (48 * virtualstudio.uiScale)
            width: backendCombo.width; height: updateChannelCombo.height
            model: bufferComboModel
            currentIndex: virtualstudio.bufferSize
            onActivated: { virtualstudio.bufferSize = currentIndex }
            font.family: "Poppins"
            visible: virtualstudio.audioBackend != "JACK"
        }

        Text {
            anchors.verticalCenter: bufferCombo.verticalCenter
            x: 48 * virtualstudio.uiScale
            text: "Buffer Size"
            font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            visible: virtualstudio.audioBackend != "JACK"
            color: textColour
        }

        ComboBox {
            id: bufferStrategyCombo
            x: updateChannelCombo.x; y: bufferCombo.y + (48 * virtualstudio.uiScale)
            width: updateChannelCombo.width; height: updateChannelCombo.height
            model: bufferStrategyComboModel
            currentIndex: virtualstudio.bufferStrategy
            onActivated: { virtualstudio.bufferStrategy = currentIndex }
            font.family: "Poppins"
            visible: virtualstudio.audioBackend != "JACK"
        }

        Text {
            anchors.verticalCenter: bufferStrategyCombo.verticalCenter
            x: 48 * virtualstudio.uiScale
            text: "Buffer Strategy"
            font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            visible: virtualstudio.audioBackend != "JACK"
            color: textColour
        }
    }

    Rectangle {
        id: profileSettingsView
        width: 0.8 * parent.width
        height: parent.height - header.height
        x: 0.2 * window.width
        y: header.height
        color: backgroundColour
        visible: settingsGroupView == "Profile"

        Image {
            id: profilePicture
            width: 96; height: 96
            y: 60 * virtualstudio.uiScale
            source: virtualstudio.userMetadata.picture ? virtualstudio.userMetadata.picture : ""
            anchors.horizontalCenter: parent.horizontalCenter
            fillMode: Image.PreserveAspectCrop
        }

        Text {
            id: displayName
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: profilePicture.bottom
            text: virtualstudio.userMetadata.user_metadata ? ( virtualstudio.userMetadata.user_metadata.display_name ? virtualstudio.userMetadata.user_metadata.display_name : virtualstudio.userMetadata.nickname ) : virtualstudio.userMetadata.name || ""
            font { family: "Poppins"; weight: Font.Bold; pixelSize: fontBig * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
        }

        Text {
            id: email
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: displayName.bottom
            text: virtualstudio.userMetadata.email ? virtualstudio.userMetadata.email : ""
            font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
        }

        Button {
            id: editButton
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: editButton.down ? buttonPressedColour : (editButton.hovered ? buttonHoverColour : buttonColour)
                border.width: 1
                border.color: editButton.down ? buttonPressedStroke : (editButton.hovered ? buttonHoverStroke : buttonStroke)
            }
            onClicked: { virtualstudio.editProfile(); }
            anchors.horizontalCenter: parent.horizontalCenter
            y: email.y + (56 * virtualstudio.uiScale)
            width: 260 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
            Text {
                text: "Edit Profile"
                font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
                color: textColour
            }
        }

        Button {
            id: logoutButton
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: logoutButton.down ? buttonPressedColour : (logoutButton.hovered ? buttonHoverColour : buttonColour)
                border.width: 1
                border.color: logoutButton.down ? buttonPressedStroke : (logoutButton.hovered ? buttonHoverStroke : buttonStroke)
            }
            onClicked: { virtualstudio.windowState = "login"; virtualstudio.logout() }
            anchors.horizontalCenter: parent.horizontalCenter
            y: editButton.y + (48 * virtualstudio.uiScale)
            width: 260 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
            Text {
                text: "Log Out"
                font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
                color: textColour
            }
        }

        Button {
            id: testModeButton
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: testModeButton.down ? buttonPressedColour : (testModeButton.hovered ? buttonHoverColour : buttonColour)
                border.width: 1
                border.color: testModeButton.down ? buttonPressedStroke : (testModeButton.hovered ? buttonHoverStroke : buttonStroke)
            }
            onClicked: { virtualstudio.testMode = !virtualstudio.testMode; virtualstudio.windowState = "login"; virtualstudio.logout() }
            anchors.horizontalCenter: parent.horizontalCenter
            y: logoutButton.y + (48 * virtualstudio.uiScale)
            width: 260 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
            visible: virtualstudio.userMetadata.email ? ( virtualstudio.userMetadata.email.endsWith("@jacktrip.org") ? true : false ) : false
            Text {
                text: virtualstudio.testMode ? "Switch to Prod Mode" : "Switch to Test Mode"
                font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
                color: textColour
            }
        }
    }

    Rectangle {
        x: -1; y: parent.height - (36 * virtualstudio.uiScale)
        width: parent.width; height: (36 * virtualstudio.uiScale)
        border.color: "#33979797"
        color: backgroundColour

        Button {
            id: cancelButton
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: cancelButton.down ? buttonPressedColour : (cancelButton.hovered ? buttonHoverColour : buttonColour)
                border.width: 1
                border.color: cancelButton.down ? buttonPressedStroke : (cancelButton.hovered ? buttonHoverStroke : buttonStroke)
            }
            onClicked: {
                virtualstudio.windowState = "browse";
                inputCombo.currentIndex = virtualstudio.previousInput;
                outputCombo.currentIndex = virtualstudio.previousOutput;
                virtualstudio.revertSettings()
            }
            anchors.verticalCenter: parent.verticalCenter
            x: parent.width - (230 * virtualstudio.uiScale)
            width: buttonWidth * virtualstudio.uiScale; height: buttonHeight * virtualstudio.uiScale
            Text {
                text: "Cancel"
                font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
                color: textColour
            }
        }

        Button {
            id: saveButton
            enabled: !Boolean(virtualstudio.devicesError)
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: saveButton.down ? buttonPressedColour : (saveButton.hovered ? buttonHoverColour : buttonColour)
                border.width: 1
                border.color: saveButton.down ? buttonPressedStroke : (saveButton.hovered ? buttonHoverStroke : buttonStroke)
            }
            onClicked: { virtualstudio.windowState = "browse"; virtualstudio.applySettings() }
            anchors.verticalCenter: parent.verticalCenter
            x: parent.width - (119 * virtualstudio.uiScale)
            width: buttonWidth * virtualstudio.uiScale; height: buttonHeight * virtualstudio.uiScale
            Text {
                text: "Save"
                font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
                color: Boolean(virtualstudio.devicesError) ? disabledButtonTextColour : textColour
            }
        }
    }
}
