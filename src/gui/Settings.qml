import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    width: 696; height: 577

    property int fontBig: 28
    property int fontMedium: 13
    property int fontSmall: 11
    
    property int leftMargin: 48
    property int buttonWidth: 103
    property int buttonHeight: 25
    
    Text {
        x: 16; y: 32
        text: "Settings"
        font { family: "Poppins"; weight: Font.Bold; pointSize: fontBig * virtualstudio.fontScale }
    }
    
    Text {
        id: jackLabel
        x:leftMargin; y: 100; width: 632
        text: "Using JACK for audio input and output. Use QjackCtl to adjust your sample rate, buffer, and device settings."
        font { family: "Poppins"; pointSize: fontMedium * virtualstudio.fontScale }
        wrapMode: Text.WordWrap
        visible: virtualstudio.audioBackend == "JACK"
    }
    
    ComboBox {
        id: inputCombo
        model: inputComboModel
        currentIndex: virtualstudio.inputDevice
        onActivated: { virtualstudio.inputDevice = currentIndex }
        x: 234; y: 100; width: 446; height: 36
        visible: virtualstudio.audioBackend != "JACK"
    }
    
    ComboBox {
        id: outputCombo
        model: outputComboModel
        currentIndex: virtualstudio.outputDevice
        onActivated: { virtualstudio.outputDevice = currentIndex }
        x: inputCombo.x; y: 148; width: inputCombo.width; height: inputCombo.height
        visible: virtualstudio.audioBackend != "JACK"
    }
    
    Text {
        anchors.verticalCenter: inputCombo.verticalCenter
        x: leftMargin
        text: "Input Device"
        font { family: "Poppins"; pointSize: fontMedium * virtualstudio.fontScale }
        visible: virtualstudio.audioBackend != "JACK"
    }
    
    Text {
        anchors.verticalCenter: outputCombo.verticalCenter
        x: leftMargin
        text: "Output Device"
        font { family: "Poppins"; pointSize: 13 * virtualstudio.fontScale }
        visible: virtualstudio.audioBackend != "JACK"
    }

    Button {
        id: refreshButton
        background: Rectangle {
            radius: 6
            color: refreshButton.down ? "#4E979797" : (refreshButton.hovered ? "#34979797" : "#1A979797")
            border { width: 0.3; color: "#34979797" }
        }
        onClicked: { virtualstudio.refreshDevices() }
        x: parent.width - 232; y: 200; width: 216; height: 30
        visible: virtualstudio.audioBackend != "JACK"
        Text {
            text: "Refresh Device List"
            font { family: "Poppins"; pointSize: 11 * virtualstudio.fontScale }
            anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
        }
    }
    
    Rectangle {
        x: leftMargin; y: 246;  width: parent.width - leftMargin - 16; height: 1
        color: "#7F0F0D0D"
        visible: virtualstudio.audioBackend != "JACK"
    }
    
    ComboBox {
        id: bufferCombo
        x: inputCombo.x; y: 262; width: inputCombo.width; height: inputCombo.height
        model: bufferComboModel
        currentIndex: virtualstudio.bufferSize
        onActivated: { virtualstudio.bufferSize = currentIndex }
        font.family: "Poppins"
        visible: virtualstudio.audioBackend != "JACK"
    }

    Text {
        anchors.verticalCenter: bufferCombo.verticalCenter
        x: 48
        text: "Buffer Size"
        font { family: "Poppins"; pointSize: fontMedium * virtualstudio.fontScale }
        visible: virtualstudio.audioBackend != "JACK"
    }
    
    Rectangle {
        id: separator
        x: leftMargin;  width: parent.width - leftMargin - 16; height: 1
        y: virtualstudio.audioBackend == "JACK" ? jackLabel.y + 64 : bufferCombo.y + 52 
        color: "#7F0F0D0D"
    }
    
    Button {
        id: modeButton
        background: Rectangle {
            radius: 6
            color: modeButton.down ? "#4E979797" : (modeButton.hovered ? "#34979797" : "#1A979797")
            border { width: 0.3; color: "#34979797" }
        }
        onClicked: { window.state = "login"; virtualstudio.toStandard(); }
        x: parent.width - 232; y: separator.y + 16; width: 216; height: 30
        Text {
            text: "Switch to Standard Mode"
            font { family: "Poppins"; pointSize: fontSmall * virtualstudio.fontScale }
            anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
        }
    }
    
    Text {
        anchors.verticalCenter: modeButton.verticalCenter
        x: leftMargin
        text: "Change Mode"
        font { family: "Poppins"; pointSize: fontMedium * virtualstudio.fontScale }
    }
    
    Text {
        x: leftMargin; y: 465
        text: "Audio Backend: " + virtualstudio.audioBackend
        font { family: "Poppins"; pointSize: fontMedium * virtualstudio.fontScale }
    }
    
    Text {
        x: leftMargin; y: 502
        text: "JackTrip version " + virtualstudio.versionString
        font { family: "Poppins"; pointSize: fontMedium * virtualstudio.fontScale }
    }

    Rectangle {
        x: 0; y: parent.height - 36; width: parent.width; height: 36
        border.color: "#33979797"

        Button {
            id: cancelButton
            background: Rectangle {
                radius: 6
                color: cancelButton.down ? "#4E979797" : (cancelButton.hovered ? "#34979797" : "#1A979797")
                border { width: 0.3; color: "#34979797" }
            }
            onClicked: { window.state = "browse"; virtualstudio.revertSettings() }
            anchors.verticalCenter: parent.verticalCenter
            x: parent.width - 230; width: buttonWidth; height: buttonHeight
            visible: virtualstudio.audioBackend != "JACK"
            Text {
                text: "Cancel"
                font { family: "Poppins"; pointSize: fontSmall * virtualstudio.fontScale }
                anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
            }
        }

        Button {
            id: okButton
            background: Rectangle {
                radius: 6
                color: okButton.down ? "#4E979797" : (okButton.hovered ? "#34979797" : "#1A979797")
                border {width: 0.3; color: "#34979797" }
            }
            onClicked: { window.state = "browse"; virtualstudio.applySettings() }
            anchors.verticalCenter: parent.verticalCenter
            x: parent.width - 119; width: buttonWidth; height: buttonHeight
            Text {
                text: "OK"
                font { family: "Poppins"; pointSize: fontSmall * virtualstudio.fontScale }
                anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
            }
        }
    }
}
