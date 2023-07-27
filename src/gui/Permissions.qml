import QtQuick
import QtQuick.Controls

Item {
    width: parent.width; height: parent.height
    clip: true

    property int fontBig: 20
    property int fontMedium: 13
    property int fontSmall: 11
    property int fontExtraSmall: 8

    property int buttonWidth: 103
    property int buttonHeight: 25

    property int leftMargin: 48
    property int rightMargin: 16

    property string backgroundColour: virtualstudio.darkMode ? "#272525" : "#FAFBFB"
    property string textColour: virtualstudio.darkMode ? "#FAFBFB" : "#0F0D0D"
    property string buttonColour: virtualstudio.darkMode ? "#494646" : "#EAECEC"
    property string buttonHoverColour: virtualstudio.darkMode ? "#5B5858" : "#D3D4D4"
    property string buttonPressedColour: virtualstudio.darkMode ? "#524F4F" : "#DEE0E0"
    property string buttonStroke: virtualstudio.darkMode ? "#80827D7D" : "#34979797"
    property string buttonHoverStroke: virtualstudio.darkMode ? "#7B7777" : "#BABCBC"
    property string buttonPressedStroke: virtualstudio.darkMode ? "#827D7D" : "#BABCBC"
    property string saveButtonBackgroundColour: "#F2F3F3"
    property string saveButtonPressedColour: "#E7E8E8"
    property string saveButtonStroke: "#EAEBEB"
    property string saveButtonPressedStroke: "#B0B5B5"
    property string warningText: "#DB0A0A"
    property string saveButtonText: "#DB0A0A"
    property string checkboxStroke: "#0062cc"
    property string checkboxPressedStroke: "#007AFF"
    property string disabledButtonText: "#D3D4D4"
    property string linkText: virtualstudio.darkMode ? "#8B8D8D" : "#272525"

    Item {
        id: requestMicPermissionsItem
        width: parent.width; height: parent.height
        visible: permissions.micPermission == "unknown"

        AppIcon {
            id: microphonePrompt
            y: 60
            anchors.horizontalCenter: parent.horizontalCenter
            width: 260
            height: 250
            icon.source: "Prompt.svg"
        }

        Image {
            id: micLogo
            source: "logo.svg"
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: microphonePrompt.top
            anchors.topMargin: 18 * virtualstudio.uiScale
            width: 32 * virtualstudio.uiScale; height: 59 * virtualstudio.uiScale
            sourceSize: Qt.size(micLogo.width,micLogo.height)
            fillMode: Image.PreserveAspectFit
            smooth: true
        }

        Button {
            id: showPromptButton
            width: 112 * virtualstudio.uiScale
            height: 30 * virtualstudio.uiScale
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: showPromptButton.down ? saveButtonPressedColour : saveButtonBackgroundColour
                border.width: 2
                border.color: showPromptButton.down || showPromptButton.hovered ? saveButtonPressedStroke : saveButtonStroke
                layer.enabled: showPromptButton.hovered && !showPromptButton.down
            }
            onClicked: {
                permissions.getMicPermission();
            }
            anchors.right: microphonePrompt.right
            anchors.rightMargin: 13.5 * virtualstudio.uiScale
            anchors.bottomMargin: 17 * virtualstudio.uiScale
            anchors.bottom: microphonePrompt.bottom
            Text {
                text: "OK"
                font.pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale
                font.weight: Font.Bold
                color: saveButtonText
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Text {
            id: micPermissionsHeader
            text: "JackTrip needs your sounds!"
            font { family: "Poppins"; weight: Font.Bold; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: microphonePrompt.bottom
            anchors.topMargin: 48 * virtualstudio.uiScale
        }

        Text {
            id: micPermissionsSubheader1
            text: "JackTrip requires permission to use your microphone."
            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            width: 400
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: micPermissionsHeader.bottom
            anchors.topMargin: 32 * virtualstudio.uiScale
        }

        Text {
            id: micPermissionsSubheader2
            text: "Click ‘OK’ to give JackTrip access to your microphone, instrument, or other audio device."
            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            width: 400
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: micPermissionsSubheader1.bottom
            anchors.topMargin: 24 * virtualstudio.uiScale
        }
    }

    Item {
        id: noMicItem
        width: parent.width; height: parent.height
        visible: permissions.micPermission == "denied"

        AppIcon {
            id: noMic
            y: 60
            anchors.horizontalCenter: parent.horizontalCenter
            width: 109.27
            height: 170
            icon.source: "micoff.svg"
        }

        Button {
            id: openSettingsButton
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: openSettingsButton.down ? saveButtonPressedColour : saveButtonBackgroundColour
                border.width: 1
                border.color: openSettingsButton.down || openSettingsButton.hovered ? saveButtonPressedStroke : saveButtonStroke
                layer.enabled: openSettingsButton.hovered && !openSettingsButton.down
            }
            onClicked: {
                permissions.openSystemPrivacy();
            }
            anchors.right: parent.right
            anchors.rightMargin: 16 * virtualstudio.uiScale
            anchors.bottomMargin: 16 * virtualstudio.uiScale
            anchors.bottom: parent.bottom
            width: 200 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
            Text {
                text: "Open Privacy Settings"
                font.family: "Poppins"
                font.pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale
                font.weight: Font.Bold
                color: saveButtonText
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Text {
            id: noMicHeader
            text: "JackTrip can't hear you!"
            font { family: "Poppins"; weight: Font.Bold; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: noMic.bottom
            anchors.topMargin: 48 * virtualstudio.uiScale
        }

        Text {
            id: noMicSubheader1
            text: "JackTrip requires permission to use your microphone."
            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            width: 400
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: noMicHeader.bottom
            anchors.topMargin: 32 * virtualstudio.uiScale
        }

        Text {
            id: noMicSubheader2
            text: "Click 'Open Privacy Settings' to give JackTrip permission to access your microphone, instrument, or other audio device."
            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            width: 400
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: noMicSubheader1.bottom
            anchors.topMargin: 24 * virtualstudio.uiScale
        }
    }

    Connections {
        target: permissions

        function onMicPermissionUpdated() {
            if (permissions.micPermission === "granted") {
                virtualstudio.windowState = "browse"
            }
        }
    }

}
