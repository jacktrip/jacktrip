import QtQuick
import QtQuick.Controls

Rectangle {
    width: parent.width; height: parent.height
    color: backgroundColour
    clip: true

    property int fontBig: 28
    property int fontMedium: 12
    property int fontSmall: 10
    property int fontTiny: 8

    property int rightMargin: 16
    property int bottomToolTipMargin: 8
    property int rightToolTipMargin: 4

    property string saveButtonText: "#DB0A0A"
    property string textColour: virtualstudio.darkMode ? "#FAFBFB" : "#0F0D0D"
    property string meterColor: virtualstudio.darkMode ? "gray" : "#E0E0E0"
    property real muteButtonLightnessValue: virtualstudio.darkMode ? 1.0 : 0.0
    property real muteButtonMutedLightnessValue: 0.24
    property real muteButtonMutedSaturationValue: 0.73
    property string buttonStroke: virtualstudio.darkMode ? "#80827D7D" : "#34979797"
    property string shadowColour: virtualstudio.darkMode ? "#40000000" : "#80A1A1A1"
    property string toolTipBackgroundColour: virtualstudio.darkMode ? "#323232" : "#F3F3F3"
    property string toolTipTextColour: textColour

    property string browserButtonColour: virtualstudio.darkMode ? "#494646" : "#EAECEC"
    property string browserButtonHoverColour: virtualstudio.darkMode ? "#5B5858" : "#D3D4D4"
    property string browserButtonPressedColour: virtualstudio.darkMode ? "#524F4F" : "#DEE0E0"
    property string browserButtonStroke: virtualstudio.darkMode ? "#80827D7D" : "#40979797"
    property string browserButtonHoverStroke: virtualstudio.darkMode ? "#7B7777" : "#BABCBC"
    property string browserButtonPressedStroke: virtualstudio.darkMode ? "#827D7D" : "#BABCBC"

    property string linkText: virtualstudio.darkMode ? "#8B8D8D" : "#272525"

    MouseArea {
        anchors.fill: parent
        propagateComposedEvents: false
    }

    Rectangle {
        id: audioSettingsView
        width: parent.width;
        height: parent.height;
        color: backgroundColour
        radius: 6 * virtualstudio.uiScale

        DeviceRefreshButton {
            id: refreshButton
            anchors.top: parent.top;
            anchors.topMargin: 16 * virtualstudio.uiScale;
            anchors.right: parent.right;
            anchors.rightMargin: 16 * virtualstudio.uiScale;
            enabled: !audio.scanningDevices
            onDeviceRefresh: function () {
                virtualstudio.triggerReconnect(true);
            }
        }

        Text {
            text: "Restarting Audio"
            anchors.verticalCenter: refreshButton.verticalCenter
            anchors.right: refreshButton.left;
            anchors.rightMargin: 16 * virtualstudio.uiScale;
            font { family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            visible: audio.scanningDevices
        }

        AudioSettings {
            id: audioSettings
            showMeters: false
            showTestAudio: false
            connected: true
            height: 300 * virtualstudio.uiScale
            anchors.top: refreshButton.bottom;
            anchors.topMargin: 16 * virtualstudio.uiScale;
        }
    }

    Button {
        id: backButton
        background: Rectangle {
            radius: 6 * virtualstudio.uiScale
            color: backButton.down ? browserButtonPressedColour : (backButton.hovered ? browserButtonHoverColour : browserButtonColour)
        }
        onClicked: {
            virtualstudio.saveSettings();
            virtualstudio.windowState = "connected";
        }
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 16 * virtualstudio.uiScale;
        anchors.left: parent.left
        anchors.leftMargin: 16 * virtualstudio.uiScale;
        width: 150 * virtualstudio.uiScale; height: 36 * virtualstudio.uiScale

        Text {
            text: "Back"
            font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale}
            anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
            color: textColour
        }
    }

    DeviceWarning {
        id: deviceWarning
        anchors.left: backButton.right
        anchors.leftMargin: 24 * virtualstudio.uiScale
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 16 * virtualstudio.uiScale;
        visible: Boolean(audio.devicesError) || Boolean(audio.devicesWarning)
    }
}
