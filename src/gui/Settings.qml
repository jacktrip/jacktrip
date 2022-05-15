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
    
    Text {
        x: 16 * virtualstudio.uiScale; y: 32 * virtualstudio.uiScale
        text: "Settings"
        font { family: "Poppins"; weight: Font.Bold; pixelSize: fontBig * virtualstudio.fontScale * virtualstudio.uiScale }
        color: textColour
    }
    
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
    
    Rectangle {
        id: separator
        x: leftMargin * virtualstudio.uiScale
        width: parent.width - x - (16 * virtualstudio.uiScale); height: 1 * virtualstudio.uiScale
        y: virtualstudio.audioBackend == "JACK" ? 
            (virtualstudio.selectableBackend ? backendCombo.y + (48 * virtualstudio.uiScale) : jackLabel.y + (64 * virtualstudio.uiScale)) : bufferCombo.y + (52 * virtualstudio.uiScale) 
        color: textColour
    }
    
    Slider {
        id: scaleSlider
        x: backendCombo.x; y: separator.y + (16 * virtualstudio.uiScale)
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
        x: parent.width - (232 * virtualstudio.uiScale); y: scaleSlider.y + (40 * virtualstudio.uiScale)
        width: 216 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
        Text {
            text: "Switch to Standard Mode"
            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
            color: textColour
        }
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
        x: parent.width -(464 * virtualstudio.uiScale)
        width: 216 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
        anchors.verticalCenter: modeButton.verticalCenter
        Text {
            text: virtualstudio.darkMode ? "Switch to Light Mode" : "Switch to Dark Mode"
            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
            color: textColour
        }
    }
    
    Text {
        anchors.verticalCenter: modeButton.verticalCenter
        x: leftMargin * virtualstudio.uiScale
        text: "Change Mode"
        font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
        color: textColour
    }
    
    Text {
        x: leftMargin * virtualstudio.uiScale; y: parent.height - (75 * virtualstudio.uiScale)
        text: "JackTrip version " + virtualstudio.versionString
        font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale}
        color: textColour
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
        x: parent.width - ((16 + buttonWidth) * virtualstudio.uiScale); y: modeButton.y + (46 * virtualstudio.uiScale)
        width: buttonWidth * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
        Text {
            text: "Log Out"
            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
            color: textColour
        }
    }

    Rectangle {
        x: 0; y: parent.height - (36 * virtualstudio.uiScale)
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
