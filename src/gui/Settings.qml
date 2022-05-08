import QtQuick
import QtQuick.Controls

Item {
    width: parent.width; height: parent.height
    clip: true
    
    Rectangle {
        width: parent.width; height: parent.height
        color: "#FAFBFB"
    }

    property int fontBig: 28
    property int fontMedium: 13
    property int fontSmall: 11
    
    property int leftMargin: 48
    property int buttonWidth: 103
    property int buttonHeight: 25
    
    Text {
        x: 16 * virtualstudio.uiScale; y: 32 * virtualstudio.uiScale
        text: "Settings"
        font { family: "Poppins"; weight: Font.Bold; pixelSize: fontBig * virtualstudio.fontScale * virtualstudio.uiScale }
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
    }
    
    Text {
        id: jackLabel
        x: leftMargin * virtualstudio.uiScale; y: 100 * virtualstudio.uiScale
        width: parent.width - x - (16 * virtualstudio.uiScale)
        text: "Using JACK for audio input and output. Use QjackCtl to adjust your sample rate, buffer, and device settings."
        font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
        wrapMode: Text.WordWrap
        visible: virtualstudio.audioBackend == "JACK" && !virtualstudio.selectableBackend
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
    }
    
    Text {
        anchors.verticalCenter: outputCombo.verticalCenter
        x: leftMargin * virtualstudio.uiScale
        text: "Output Device"
        font { family: "Poppins"; pixelSize: 13 * virtualstudio.fontScale * virtualstudio.uiScale }
        visible: virtualstudio.audioBackend != "JACK"
    }

    Button {
        id: refreshButton
        background: Rectangle {
            radius: 6 * virtualstudio.uiScale
            color: refreshButton.down ? "#DEE0E0" : (refreshButton.hovered ? "#D3D4D4" : "#EAECEC")
            border.width: 1
            border.color: refreshButton.down || refreshButton.hovered ? "#BABCBC" : "#34979797"
        }
        onClicked: { virtualstudio.refreshDevices() }
        x: parent.width - (232 * virtualstudio.uiScale); y: inputCombo.y + (100 * virtualstudio.uiScale)
        width: 216 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
        visible: virtualstudio.audioBackend != "JACK"
        Text {
            text: "Refresh Device List"
            font { family: "Poppins"; pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale }
            anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
        }
    }
    
    Rectangle {
        x: leftMargin * virtualstudio.uiScale; y: inputCombo.y + (146 * virtualstudio.uiScale)
        width: parent.width - x - (16 * virtualstudio.uiScale); height: 1 * virtualstudio.uiScale
        color: "#7F0F0D0D"
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
    }
    
    Rectangle {
        id: separator
        x: leftMargin * virtualstudio.uiScale
        width: parent.width - x - (16 * virtualstudio.uiScale); height: 1 * virtualstudio.uiScale
        y: virtualstudio.audioBackend == "JACK" ? 
            (virtualstudio.selectableBackend ? backendCombo.y + (48 * virtualstudio.uiScale) : jackLabel.y + (64 * virtualstudio.uiScale)) : bufferCombo.y + (52 * virtualstudio.uiScale) 
        color: "#7F0F0D0D"
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
    }
    
    Button {
        id: modeButton
        background: Rectangle {
            radius: 6 * virtualstudio.uiScale
            color: modeButton.down ? "#DEE0E0" : (modeButton.hovered ? "#D3D4D4" : "#EAECEC")
            border.width: 1
            border.color: modeButton.down || modeButton.hovered ? "#BABCBC" : "#34979797"
        }
        onClicked: { window.state = "login"; virtualstudio.toStandard(); }
        x: parent.width - (232 * virtualstudio.uiScale); y: scaleSlider.y + (40 * virtualstudio.uiScale)
        width: 216 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
        Text {
            text: "Switch to Standard Mode"
            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
        }
    }
    
    // Button {
    //     id: darkButton
    //     background: Rectangle {
    //         radius: 6 * virtualstudio.uiScale
    //         color: darkButton.down ? "#DEE0E0" : (darkButton.hovered ? "#D3D4D4" : "#EAECEC")
    //         border.width: 1
    //         border.color: darkButton.down || darkButton.hovered ? "#BABCBC" : "#34979797"
    //     }
    //     x: parent.width -(464 * virtualstudio.uiScale)
    //     width: 216 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
    //     anchors.verticalCenter: modeButton.verticalCenter
    //     Text {
    //         text: "Switch to Dark Mode"
    //         font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
    //         anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
    //     }
    // }
    
    Text {
        anchors.verticalCenter: modeButton.verticalCenter
        x: leftMargin * virtualstudio.uiScale
        text: "Change Mode"
        font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
    }
    
    Text {
        x: leftMargin * virtualstudio.uiScale; y: parent.height - (75 * virtualstudio.uiScale)
        text: "JackTrip version " + virtualstudio.versionString
        font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale}
    }

    Button {
        id: logoutButton
        background: Rectangle {
            radius: 6 * virtualstudio.uiScale
            color: logoutButton.down ? "#DEE0E0" : (logoutButton.hovered ? "#D3D4D4" : "#EAECEC")
            border.width: 1
            border.color: logoutButton.down || logoutButton.hovered ? "#BABCBC" : "#34979797"
        }
        onClicked: { window.state = "login"; virtualstudio.logout() }
        x: parent.width - ((16 + buttonWidth) * virtualstudio.uiScale); y: modeButton.y + (46 * virtualstudio.uiScale)
        width: buttonWidth * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
        Text {
            text: "Log Out"
            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
        }
    }

    Rectangle {
        x: 0; y: parent.height - (36 * virtualstudio.uiScale)
        width: parent.width; height: (36 * virtualstudio.uiScale)
        border.color: "#33979797"

        Button {
            id: backButton
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: backButton.down ? "#DEE0E0" : (backButton.hovered ? "#D3D4D4" : "#EAECEC")
                border.width: 1
                border.color: backButton.down || backButton.hovered ? "#BABCBC" : "#34979797"
            }
            onClicked: { window.state = "browse"; virtualstudio.revertSettings() }
            anchors.verticalCenter: parent.verticalCenter
            x: parent.width - (230 * virtualstudio.uiScale)
            width: buttonWidth * virtualstudio.uiScale; height: buttonHeight * virtualstudio.uiScale
            Text {
                text: "Back"
                font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
            }
        }

        Button {
            id: okButton
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: okButton.down ? "#DEE0E0" : (okButton.hovered ? "#D3D4D4" : "#EAECEC")
                border.width: 1
                border.color: okButton.down || okButton.hovered ? "#BABCBC" : "#34979797"
            }
            onClicked: { window.state = "browse"; virtualstudio.applySettings() }
            anchors.verticalCenter: parent.verticalCenter
            x: parent.width - (119 * virtualstudio.uiScale)
            width: buttonWidth * virtualstudio.uiScale; height: buttonHeight * virtualstudio.uiScale
            Text {
                text: "OK"
                font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
            }
        }
    }
}
