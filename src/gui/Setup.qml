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

    property string backgroundColour: virtualstudio.darkMode ? "#272525" : "#FAFBFB"
    property string textColour: virtualstudio.darkMode ? "#FAFBFB" : "#0F0D0D"
    property string buttonColour: virtualstudio.darkMode ? "#494646" : "#EAECEC"
    property string buttonHoverColour: virtualstudio.darkMode ? "#5B5858" : "#D3D4D4"
    property string buttonPressedColour: virtualstudio.darkMode ? "#524F4F" : "#DEE0E0"
    property string buttonStroke: virtualstudio.darkMode ? "#80827D7D" : "#34979797"
    property string buttonHoverStroke: virtualstudio.darkMode ? "#7B7777" : "#BABCBC"
    property string buttonPressedStroke: virtualstudio.darkMode ? "#827D7D" : "#BABCBC"
    property string saveButtonShadow: "#80A1A1A1"
    property string saveButtonBackgroundColour: "#F2F3F3"
    property string saveButtonPressedColour: "#E7E8E8"
    property string saveButtonStroke: "#EAEBEB"
    property string saveButtonPressedStroke: "#B0B5B5"
    property string warningText: "#DB0A0A"
    property string saveButtonText: "#DB0A0A"
    property string checkboxStroke: "#0062cc"
    property string checkboxPressedStroke: "#007AFF"
    
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
        x: 234 * virtualstudio.uiScale; y: 150 * virtualstudio.uiScale
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
        x: leftMargin * virtualstudio.uiScale; y: 150 * virtualstudio.uiScale
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
        x: 234 * virtualstudio.uiScale; y: virtualstudio.uiScale * (virtualstudio.selectableBackend ? 198 : 150)
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
        id: inputLabel
        anchors.verticalCenter: inputCombo.verticalCenter
        x: leftMargin * virtualstudio.uiScale
        text: "Input Device"
        font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
        visible: virtualstudio.audioBackend != "JACK"
    }
    
    Text {
        id: outputLabel
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
        }
    }

    Text {
        anchors.left: outputLabel.left
        anchors.right: outputCombo.right
        anchors.leftMargin: 16 * virtualstudio.uiScale
        anchors.rightMargin: 16 * virtualstudio.uiScale
        y: inputCombo.y + (160 * virtualstudio.uiScale)
        text: "JackTrip on Windows requires use of an audio device with ASIO drivers. If you do not see your device, you may need to install drivers from your manufacturer."
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        color: warningText
        font { family: "Poppins"; pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale }
        visible: Qt.platform.os == "windows" && virtualstudio.audioBackend != "JACK"
    }

    Button {
        id: saveButton
        background: Rectangle {
            radius: 6 * virtualstudio.uiScale
            color: saveButton.down ? saveButtonPressedColour : saveButtonBackgroundColour
            border.width: 1
            border.color: saveButton.down ? saveButtonPressedStroke : saveButtonStroke
            layer.enabled: saveButton.hovered && !saveButton.down
            layer.effect: DropShadow {
                horizontalOffset: 1 * virtualstudio.uiScale
                verticalOffset: 1 * virtualstudio.uiScale
                radius: 8.0 * virtualstudio.uiScale
                samples: 17
                color: saveButtonShadow
            }
        }
        onClicked: { window.state = "browse"; virtualstudio.applySettings() }
        anchors.right: parent.right
        anchors.rightMargin: 16 * virtualstudio.uiScale
        anchors.bottomMargin: 16 * virtualstudio.uiScale
        anchors.bottom: parent.bottom
        width: 150 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
        Text {
            text: "Save Settings"
            font.family: "Poppins"
            font.pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale
            font.weight: Font.Bold
            color: saveButtonText
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
            border.color: showAgainCheckbox.down ? checkboxPressedStroke : checkboxStroke

            Rectangle {
                width: 10 * virtualstudio.uiScale
                height: 10 * virtualstudio.uiScale
                x: 3 * virtualstudio.uiScale
                y: 3 * virtualstudio.uiScale
                radius: 2 * virtualstudio.uiScale
                color: showAgainCheckbox.down ? checkboxPressedStroke : checkboxStroke
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
