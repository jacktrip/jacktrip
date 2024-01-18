import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.jacktrip.jacktrip 1.0

Item {
    width: parent.width; height: parent.height
    clip: true

    property bool connecting: false

    property int leftHeaderMargin: 16
    property int fontBig: 28
    property int fontMedium: 12
    property int fontSmall: 10
    property int fontTiny: 8

    property int bodyMargin: 60
    property int rightMargin: 16
    property int bottomToolTipMargin: 8
    property int rightToolTipMargin: 4

    property string studioStatus: (virtualstudio.currentStudio.id === "" ? "" : virtualstudio.currentStudio.status)
    property bool showReadyScreen: studioStatus === "Ready"
    property bool showStartingScreen: studioStatus === "Starting"
    property bool showStoppingScreen: (virtualstudio.currentStudio.id === "" ? false : (virtualstudio.currentStudio.isAdmin && !virtualstudio.currentStudio.enabled && virtualstudio.currentStudio.cloudId !== ""))
    property bool showWaitingScreen: !showStoppingScreen && !showStartingScreen && !showReadyScreen

    property string buttonColour: virtualstudio.darkMode ? "#494646" : "#EAECEC"
    property string strokeColor: virtualstudio.darkMode ? "#80827D7D" : "#34979797"

    property string browserButtonColour: virtualstudio.darkMode ? "#494646" : "#EAECEC"
    property string browserButtonHoverColour: virtualstudio.darkMode ? "#5B5858" : "#D3D4D4"
    property string browserButtonPressedColour: virtualstudio.darkMode ? "#524F4F" : "#DEE0E0"
    property string browserButtonStroke: virtualstudio.darkMode ? "#80827D7D" : "#40979797"
    property string browserButtonHoverStroke: virtualstudio.darkMode ? "#7B7777" : "#BABCBC"
    property string browserButtonPressedStroke: virtualstudio.darkMode ? "#827D7D" : "#BABCBC"
    property string saveButtonBackgroundColour: "#F2F3F3"
    property string saveButtonPressedColour: "#E7E8E8"
    property string saveButtonStroke: "#EAEBEB"
    property string saveButtonPressedStroke: "#B0B5B5"
    property string saveButtonText: "#DB0A0A"

    property string muteButtonMutedColor: "#FCB6B6"
    property string textColour: virtualstudio.darkMode ? "#FAFBFB" : "#0F0D0D"
    property string meterColor: virtualstudio.darkMode ? "gray" : "#E0E0E0"
    property real muteButtonLightnessValue: virtualstudio.darkMode ? 1.0 : 0.0
    property real muteButtonMutedLightnessValue: 0.24
    property real muteButtonMutedSaturationValue: 0.73
    property string buttonStroke: virtualstudio.darkMode ? "#80827D7D" : "#34979797"
    property string shadowColour: virtualstudio.darkMode ? "#40000000" : "#80A1A1A1"
    property string toolTipBackgroundColour: virtualstudio.darkMode ? "#323232" : "#F3F3F3"
    property string toolTipTextColour: textColour
    property string warningTextColour: "#DB0A0A"
    property string linkText: virtualstudio.darkMode ? "#8B8D8D" : "#272525"

    property string meterGreen: "#61C554"
    property string meterYellow: "#F5BF4F"
    property string meterRed: "#F21B1B"

    property bool isUsingRtAudio: audio.audioBackend == "RtAudio"

    Loader {
        id: studioWebLoader
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.bottom: deviceControlsGroup.top

        property string accessToken: auth.isAuthenticated && Boolean(auth.accessToken) ? auth.accessToken : ""
        property string studioId: virtualstudio.currentStudio.id

        source: accessToken && studioId ? "Web.qml" : "WebNull.qml"
    }

    DeviceControlsGroup {
        id: deviceControlsGroup
        anchors.bottom: footer.top
    }

    Footer {
        id: footer
        anchors.bottom: parent.bottom
    }
}
