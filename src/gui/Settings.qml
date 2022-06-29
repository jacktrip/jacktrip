import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    width: parent.width; height: parent.height
    clip: true
    
    Rectangle {
        width: parent.width; height: parent.height
        color: backgroundColour
    }

    property int fontBig: 28
    property int fontMedium: 13
    property int fontSmall: 11
    
    property int leftMargin: 48
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

    property string settingsGroupView: "Audio"

    ToolBar {
        id: header
        width: parent.width

        background: Rectangle {
            border.color: "#33979797"
            color: backgroundColour
            width: parent.width
        }

        contentItem: Label {
            text: "Settings"
            elide: Label.ElideRight
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font { family: "Poppins"; weight: Font.Bold; pixelSize: fontBig * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
        }
    }

    Drawer {
        id: drawer
        width: 0.2 * parent.width
        height: parent.height - header.height
        y: header.height-1
        modal: false
        interactive: false
        visible: window.state == "settings"

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
                contentItem: Label {
                    text: audioBtn.text
                    font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    color: textColour
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
                contentItem: Label {
                    text: appearanceBtn.text
                    font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    color: textColour
                }
                background: Rectangle {
                    width: parent.width
                    color: appearanceBtn.down ? buttonPressedColour : (appearanceBtn.hovered || settingsGroupView == "Appearance" ? buttonHoverColour : backgroundColour)
                }
            }
            Button {
                id: profileBtn
                text: "Profile"
                width: parent.width
                contentItem: Label {
                    text: profileBtn.text
                    font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    color: textColour
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

        ComboBox {
            id: backendCombo
            model: backendComboModel
            currentIndex: virtualstudio.audioBackend == "JACK" ? 0 : 1
            onActivated: { virtualstudio.audioBackend = currentText }
            x: 234 * virtualstudio.uiScale; y: 100 * virtualstudio.uiScale
            width: parent.width - x - (16 * virtualstudio.uiScale); height: 36 * virtualstudio.uiScale
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
        
        Text {
            id: jackLabel
            x: leftMargin * virtualstudio.uiScale; y: 100 * virtualstudio.uiScale
            width: parent.width - x - (16 * virtualstudio.uiScale)
            text: "Using JACK for audio input and output. Use QjackCtl to adjust your sample rate, buffer, and device settings."
            font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            wrapMode: Text.WordWrap
            visible: virtualstudio.audioBackend == "JACK" && !virtualstudio.selectableBackend
            color: textColour
        }
        
        ComboBox {
            id: inputCombo
            model: inputComboModel
            currentIndex: virtualstudio.inputDevice
            onActivated: { virtualstudio.inputDevice = currentIndex }
            x: 234 * virtualstudio.uiScale; y: virtualstudio.uiScale * (virtualstudio.selectableBackend ? 148 : 100)
            width: parent.width - x - (16 * virtualstudio.uiScale); height: 36 * virtualstudio.uiScale
            visible: virtualstudio.audioBackend != "JACK"
        }
        
        ComboBox {
            id: outputCombo
            model: outputComboModel
            currentIndex: virtualstudio.outputDevice
            onActivated: { virtualstudio.outputDevice = currentIndex }
            x: backendCombo.x; y: inputCombo.y + (48 * virtualstudio.uiScale)
            width: backendCombo.width; height: backendCombo.height
            visible: virtualstudio.audioBackend != "JACK"
        }
        
        Text {
            anchors.verticalCenter: inputCombo.verticalCenter
            x: leftMargin * virtualstudio.uiScale
            text: "Input Device"
            font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            visible: virtualstudio.audioBackend != "JACK"
            color: textColour
        }
        
        Text {
            anchors.verticalCenter: outputCombo.verticalCenter
            x: leftMargin * virtualstudio.uiScale
            text: "Output Device"
            font { family: "Poppins"; pixelSize: 13 * virtualstudio.fontScale * virtualstudio.uiScale }
            visible: virtualstudio.audioBackend != "JACK"
            color: textColour
        }

        Button {
            id: refreshButton
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: refreshButton.down ? buttonPressedColour : (refreshButton.hovered ? buttonHoverColour : buttonColour)
                border.width: 1
                border.color: refreshButton.down ? buttonPressedStroke : (refreshButton.hovered ? buttonHoverStroke : buttonStroke)
            }
            onClicked: { virtualstudio.refreshDevices() }
            x: parent.width - (232 * virtualstudio.uiScale); y: inputCombo.y + (100 * virtualstudio.uiScale)
            width: 216 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
            visible: virtualstudio.audioBackend != "JACK"
            Text {
                text: "Refresh Device List"
                font { family: "Poppins"; pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale }
                anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
                color: textColour
            }
        }
        
        Rectangle {
            x: leftMargin * virtualstudio.uiScale; y: inputCombo.y + (146 * virtualstudio.uiScale)
            width: parent.width - x - (16 * virtualstudio.uiScale); height: 1 * virtualstudio.uiScale
            color: textColour
            visible: virtualstudio.audioBackend != "JACK"
        }
        
        ComboBox {
            id: bufferCombo
            x: backendCombo.x; y: inputCombo.y + (162 * virtualstudio.uiScale)
            width: backendCombo.width; height: backendCombo.height
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
        }
        
        Text {
            anchors.verticalCenter: scaleSlider.verticalCenter
            x: leftMargin * virtualstudio.uiScale
            text: "Scale Interface"
            font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
        }
        
        Button {
            id: modeButton
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: modeButton.down ? buttonPressedColour : (modeButton.hovered ? buttonHoverColour : buttonColour)
                border.width: 1
                border.color: modeButton.down ? buttonPressedStroke : (modeButton.hovered ? buttonHoverStroke : buttonStroke)
            }
            onClicked: { window.state = "login"; virtualstudio.toStandard(); }
            x: parent.width - (232 * virtualstudio.uiScale); y: scaleSlider.y + (56 * virtualstudio.uiScale)
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

        ComboBox {
            id: updateChannelCombo
            x: 234 * virtualstudio.uiScale; y: darkButton.y + (56 * virtualstudio.uiScale)
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
            onClicked: { window.state = "login"; virtualstudio.logout() }
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
            onClicked: { window.state = "browse"; virtualstudio.revertSettings() }
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
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: saveButton.down ? buttonPressedColour : (saveButton.hovered ? buttonHoverColour : buttonColour)
                border.width: 1
                border.color: saveButton.down ? buttonPressedStroke : (saveButton.hovered ? buttonHoverStroke : buttonStroke)
            }
            onClicked: { window.state = "browse"; virtualstudio.applySettings() }
            anchors.verticalCenter: parent.verticalCenter
            x: parent.width - (119 * virtualstudio.uiScale)
            width: buttonWidth * virtualstudio.uiScale; height: buttonHeight * virtualstudio.uiScale
            Text {
                text: "Save"
                font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
                color: textColour
            }
        }
    }
}
