import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.12

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
    property bool showContents: true

    visible: showContents
    
    Text {
        x: 16 * virtualstudio.uiScale; y: 32 * virtualstudio.uiScale
        text: "Choose your audio devices"
        font { family: "Poppins"; weight: Font.Bold; pixelSize: fontBig * virtualstudio.fontScale * virtualstudio.uiScale }
    }
    
    ComboBox {
        id: backendCombo
        model: backendComboModel
        currentIndex: virtualstudio.audioBackend == "JACK" ? 0 : 1
        onActivated: { virtualstudio.audioBackend = currentText }
        x: 234 * virtualstudio.uiScale; y: 200 * virtualstudio.uiScale
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
        x: leftMargin * virtualstudio.uiScale; y: 200 * virtualstudio.uiScale
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
        x: 234 * virtualstudio.uiScale; y: virtualstudio.uiScale * (virtualstudio.selectableBackend ? 248 : 200)
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

    Button {
        id: saveButton
        background: Rectangle {
            radius: 6 * virtualstudio.uiScale
            color: saveButton.down ? "#E7E8E8" : "#F2F3F3"
            border.width: 1
            border.color: saveButton.down ? "#B0B5B5" : "#EAEBEB"
            layer.enabled: saveButton.hovered && !saveButton.down
            layer.effect: DropShadow {
                horizontalOffset: 1 * virtualstudio.uiScale
                verticalOffset: 1 * virtualstudio.uiScale
                radius: 8.0 * virtualstudio.uiScale
                samples: 17
                color: "#80A1A1A1"
            }
        }
        onClicked: { window.state = "browse"; virtualstudio.applySettings() }
        anchors.right: parent.right
        anchors.rightMargin: 16 * virtualstudio.uiScale
        anchors.bottomMargin: 16 * virtualstudio.uiScale
        anchors.bottom: parent.bottom
        width: 150 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
        Text {
            text: "Save Changes"
            font.family: "Poppins"
            font.pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale
            font.weight: Font.Bold
            color: "#DB0A0A"
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    CheckBox {
        id: showAgainCheckbox
        checked: virtualstudio.showDeviceSetup
        text: qsTr("Ask again next time")
        anchors.right: saveButton.left
        anchors.rightMargin: 16 * virtualstudio.uiScale
        anchors.verticalCenter: saveButton.verticalCenter
        onClicked: { virtualstudio.toggleShowDeviceSetup() }
        indicator: Rectangle {
            implicitWidth: 16 * virtualstudio.uiScale
            implicitHeight: 16 * virtualstudio.uiScale
            x: showAgainCheckbox.leftPadding
            y: parent.height / 2 - height / 2
            radius: 3 * virtualstudio.uiScale
            border.color: showAgainCheckbox.down ? "#007AFF" : "#0062cc"

            Rectangle {
                width: 10 * virtualstudio.uiScale
                height: 10 * virtualstudio.uiScale
                x: 3 * virtualstudio.uiScale
                y: 3 * virtualstudio.uiScale
                radius: 2 * virtualstudio.uiScale
                color: showAgainCheckbox.down ? "#007AFF" : "#0062cc"
                visible: showAgainCheckbox.checked
            }
        }
        contentItem: Text {
            text: showAgainCheckbox.text
            font.family: "Poppins"
            font.pixelSize: 10 * virtualstudio.fontScale * virtualstudio.uiScale
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            leftPadding: showAgainCheckbox.indicator.width + showAgainCheckbox.spacing
        }
    }
}
