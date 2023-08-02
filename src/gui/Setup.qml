import QtQuick
import QtQuick.Controls

Item {
    width: parent.width; height: parent.height
    clip: true

    property int fontBig: 20
    property int fontMedium: 13
    property int fontSmall: 11
    property int fontExtraSmall: 8

    property int leftMargin: 48
    property int rightMargin: 16

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
    property string saveButtonText: "#DB0A0A"
    property string checkboxStroke: "#0062cc"
    property string checkboxPressedStroke: "#007AFF"
    property string disabledButtonText: virtualstudio.darkMode ? "#827D7D" : "#BABCBC"

    property bool currShowWarnings: virtualstudio.showWarnings
    property string warningScreen: virtualstudio.showWarnings ? "ethernet" : ( permissions.micPermission == "unknown" ? "microphone" : "acknowledged")

    Item {
        id: setupItem
        width: parent.width; height: parent.height

        property bool isUsingRtAudio: virtualstudio.audioBackend == "RtAudio"

        Text {
            id: pageTitle
            x: 16 * virtualstudio.uiScale; y: 32 * virtualstudio.uiScale
            text: "Choose your audio devices"
            font { family: "Poppins"; weight: Font.Bold; pixelSize: fontBig * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
        }

        Button {
            id: refreshButton
            text: "Refresh Devices"
            palette.buttonText: textColour
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: refreshButton.down ? buttonPressedColour : (refreshButton.hovered ? buttonHoverColour : buttonColour)
                border.width: 1
                border.color: refreshButton.down ? buttonPressedStroke : (refreshButton.hovered ? buttonHoverStroke : buttonStroke)
            }
            icon {
                source: "refresh.svg";
                color: textColour;
            }
            display: AbstractButton.TextBesideIcon
            onClicked: {
                virtualstudio.refreshDevices();
            }
            anchors.right: parent.right
            anchors.rightMargin: rightMargin * virtualstudio.uiScale
            anchors.verticalCenter: pageTitle.verticalCenter
            width: 144 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
            font {
                family: "Poppins"
                pixelSize: fontExtraSmall * virtualstudio.fontScale * virtualstudio.uiScale
            }
            visible: parent.isUsingRtAudio
        }

        AudioSettings {
            id: audioSettings
            width: parent.width
            anchors.top: pageTitle.bottom
            anchors.topMargin: 24 * virtualstudio.uiScale
        }

        Button {
            id: backButton
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: backButton.down ? buttonPressedColour : buttonColour
                border.width: 1
                border.color: backButton.down || backButton.hovered ? buttonPressedStroke : buttonStroke
            }
            onClicked: { virtualstudio.windowState = "browse"; virtualstudio.studioToJoin = ""; }
            anchors.left: parent.left
            anchors.leftMargin: 16 * virtualstudio.uiScale
            anchors.bottomMargin: rightMargin * virtualstudio.uiScale
            anchors.bottom: parent.bottom
            width: 150 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
            Text {
                text: "Back"
                font.family: "Poppins"
                font.pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale
                color: textColour
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        AppIcon {
            id: devicesWarningIcon
            anchors.left: backButton.right
            anchors.leftMargin: 16 * virtualstudio.uiScale
            anchors.verticalCenter: backButton.verticalCenter
            width: 28 * virtualstudio.uiScale
            height: 28 * virtualstudio.uiScale
            icon.source: "warning.svg"
            color: "#F21B1B"
            visible: Boolean(virtualstudio.devicesError) || Boolean(virtualstudio.devicesWarning)
        }

        Text {
            id: warningOrErrorText
            text: Boolean(virtualstudio.devicesError) ? "Audio Configuration Error" : "Audio Configuration Warning"
            anchors.left: devicesWarningIcon.right
            anchors.leftMargin: 4 * virtualstudio.uiScale
            anchors.verticalCenter: devicesWarningIcon.verticalCenter
            visible: Boolean(virtualstudio.devicesError) || Boolean(virtualstudio.devicesWarning)
            font { family: "Poppins"; pixelSize: 10 * virtualstudio.fontScale * virtualstudio.uiScale }
            color: "#F21B1B"
        }

        InfoTooltip {
            id: devicesWarningTooltip
            anchors.left: warningOrErrorText.right
            anchors.leftMargin: 2 * virtualstudio.uiScale
            anchors.bottom: warningOrErrorText.bottom
            anchors.bottomMargin: 6 * virtualstudio.uiScale
            content: qsTr(virtualstudio.devicesError || virtualstudio.devicesWarning)
            iconColor: "#F21B1B"
            size: 16 * virtualstudio.uiScale
            visible: Boolean(virtualstudio.devicesError) || Boolean(virtualstudio.devicesWarning)
        }

        Button {
            id: saveButton
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: saveButton.down ? saveButtonPressedColour : saveButtonBackgroundColour
                border.width: 1
                border.color: saveButton.down || saveButton.hovered ? saveButtonPressedStroke : saveButtonStroke
            }
            enabled: !Boolean(virtualstudio.devicesError) && virtualstudio.backendAvailable
            onClicked: { virtualstudio.windowState = "connected"; virtualstudio.applySettings() }
            anchors.right: parent.right
            anchors.rightMargin: rightMargin * virtualstudio.uiScale
            anchors.bottomMargin: rightMargin * virtualstudio.uiScale
            anchors.bottom: parent.bottom
            width: 150 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
            Text {
                text: "Connect to Studio"
                font.family: "Poppins"
                font.pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale
                font.weight: Font.Bold
                color: !Boolean(virtualstudio.devicesError) && virtualstudio.backendAvailable ? saveButtonText : disabledButtonText
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        CheckBox {
            id: showAgainCheckbox
            checked: virtualstudio.showDeviceSetup
            visible: virtualstudio.backendAvailable
            text: qsTr("Ask again next time")
            anchors.right: saveButton.left
            anchors.rightMargin: 16 * virtualstudio.uiScale
            anchors.verticalCenter: saveButton.verticalCenter
            onClicked: { virtualstudio.showDeviceSetup = showAgainCheckbox.checkState == Qt.Checked }
            indicator: Rectangle {
                implicitWidth: 16 * virtualstudio.uiScale
                implicitHeight: 16 * virtualstudio.uiScale
                x: showAgainCheckbox.leftPadding
                y: parent.height / 2 - height / 2
                radius: 3 * virtualstudio.uiScale
                border.color: showAgainCheckbox.down || showAgainCheckbox.hovered ? checkboxPressedStroke : checkboxStroke

                Rectangle {
                    width: 10 * virtualstudio.uiScale
                    height: 10 * virtualstudio.uiScale
                    x: 3 * virtualstudio.uiScale
                    y: 3 * virtualstudio.uiScale
                    radius: 2 * virtualstudio.uiScale
                    color: showAgainCheckbox.down || showAgainCheckbox.hovered ? checkboxPressedStroke : checkboxStroke
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
                color: textColour
            }
        }
    }
}
