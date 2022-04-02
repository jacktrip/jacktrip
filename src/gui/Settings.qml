import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    width: 696; height: 577

    Text {
        x: 16; y: 32
        text: "Settings"
        font.family: "Poppins"
        font.weight: Font.Bold
        font.pointSize: 28
    }
    
    Text {
        id: jackLabel
        x:48; y: 100; width: 632
        text: "Using JACK for audio input and output. Use QjackCtl to adjust your sample rate, buffer, and device settings."
        font.family: "Poppins"
        font.pointSize: 13
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
        x: 234; y: 148; width: 446; height: 36
        visible: virtualstudio.audioBackend != "JACK"
    }
    
    Text {
        anchors.verticalCenter: inputCombo.verticalCenter
        x: 48
        text: "Input Device"
        font.family: "Poppins"
        font.pointSize: 13
        visible: virtualstudio.audioBackend != "JACK"
    }
    
    Text {
        anchors.verticalCenter: outputCombo.verticalCenter
        x: 48
        text: "Output Device"
        font.family: "Poppins"
        font.pointSize: 13
        visible: virtualstudio.audioBackend != "JACK"
    }

    Button {
        id: refreshButton
        background: Rectangle {
            radius: 6
            color: refreshButton.down ? "#4E979797" : (refreshButton.hovered ? "#34979797" : "#1A979797")
            border.width: 0.3
            border.color: "#34979797"
        }
        onClicked: { virtualstudio.refreshDevices() }
        x: 464; y: 200; width: 216; height: 30
        visible: virtualstudio.audioBackend != "JACK"
        Text {
            text: "Refresh Device List"
            font.family: "Poppins"
            font.pointSize: 11
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
        }
    }
    
    Rectangle {
        x: 48; y: 246;  width: 632; height: 1
        color: "#7F0F0D0D"
        visible: virtualstudio.audioBackend != "JACK"
    }
    
    ComboBox {
        id: bufferCombo
        x: 234; y: 262; width: 446; height: 36
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
        font.family: "Poppins"
        font.pointSize: 13
        visible: virtualstudio.audioBackend != "JACK"
    }
    
    Rectangle {
        id: separator
        x: 48;  width: 632; height: 1
        y: virtualstudio.audioBackend == "JACK" ? jackLabel.y + 64 : bufferCombo.y + 52 
        color: "#7F0F0D0D"
    }
    
    Button {
        id: modeButton
        background: Rectangle {
            radius: 6
            color: modeButton.down ? "#4E979797" : (modeButton.hovered ? "#34979797" : "#1A979797")
            border.width: 0.3
            border.color: "#34979797"
        }
        onClicked: { window.state = "login"; virtualstudio.toStandard(); }
        x: 464; y: separator.y + 16; width: 216; height: 30
        Text {
            text: "Switch to Standard Mode"
            font.family: "Poppins"
            font.pointSize: 11
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
        }
    }
    
    Text {
        anchors.verticalCenter: modeButton.verticalCenter
        x: 48
        text: "Change Mode"
        font.family: "Poppins"
        font.pointSize: 13
    }
    
    Text {
        x: 48; y: 465
        text: "Audio Backend: " + virtualstudio.audioBackend
        font.family: "Poppins"
        font.pointSize: 13
    }
    
    Text {
        x: 48; y: 502
        text: "JackTrip version " + virtualstudio.versionString
        font.family: "Poppins"
        font.pointSize: 13
    }

    Rectangle {
        x: 0; y: parent.height - 36; width: parent.width; height: 36
        border.color: "#33979797"

        Button {
            id: cancelButton
            background: Rectangle {
                radius: 6
                color: cancelButton.down ? "#4E979797" : (cancelButton.hovered ? "#34979797" : "#1A979797")
                border.width: 0.3
                border.color: "#34979797"
            }
            onClicked: { window.state = "browse"; virtualstudio.revertSettings() }
            anchors.verticalCenter: parent.verticalCenter
            x: 466; width: 103; height: 25
            visible: virtualstudio.audioBackend != "JACK"
            Text {
                text: "Cancel"
                font.family: "Poppins"
                font.pointSize: 11
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Button {
            id: okButton
            background: Rectangle {
                radius: 6
                color: okButton.down ? "#4E979797" : (okButton.hovered ? "#34979797" : "#1A979797")
                border.width: 0.3
                border.color: "#34979797"
            }
            onClicked: { window.state = "browse"; virtualstudio.applySettings() }
            anchors.verticalCenter: parent.verticalCenter
            x: 577; width: 103; height: 25
            Text {
                text: "OK"
                font.family: "Poppins"
                font.pointSize: 11
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }
}
