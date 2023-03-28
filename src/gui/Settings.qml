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

    Rectangle {
        id: audioSettingsView
        width: 0.8 * parent.width
        height: parent.height - header.height
        x: 0.2 * window.width
        y: header.height
        visible: settingsGroupView == "Audio"

        AudioSettings{
            id: audioSettings
        }
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
                    inputCurrIndex = getCurrentInputDeviceIndex();
                    outputCurrIndex = getCurrentOutputDeviceIndex();
                }
                width: 144 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
                font {
                    family: "Poppins"
                    pixelSize: fontExtraSmall * virtualstudio.fontScale * virtualstudio.uiScale
                }
                visible: parent.isUsingRtAudio && settingsGroupView == "Audio"
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
                inputCurrIndex = virtualstudio.previousInput;
                outputCurrIndex = virtualstudio.previousOutput;
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
