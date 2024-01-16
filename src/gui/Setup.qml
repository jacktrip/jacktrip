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

    property string strokeColor: virtualstudio.darkMode ? "#80827D7D" : "#34979797"
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

    Item {
        id: setupItem
        width: parent.width; height: parent.height

        property bool isUsingRtAudio: audio.audioBackend == "RtAudio"

        Text {
            id: pageTitle
            x: 16 * virtualstudio.uiScale;
            y: 16 * virtualstudio.uiScale
            text: "Choose your audio devices"
            font { family: "Poppins"; weight: Font.Bold; pixelSize: fontBig * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
        }

        DeviceRefreshButton {
            id: refreshButton
            anchors.right: parent.right
            anchors.rightMargin: rightMargin * virtualstudio.uiScale
            anchors.verticalCenter: pageTitle.verticalCenter
            visible: parent.isUsingRtAudio
            enabled: audio.audioReady && !audio.scanningDevices
        }

        Text {
            text: "Restarting Audio"
            anchors.verticalCenter: pageTitle.verticalCenter;
            anchors.right: refreshButton.left;
            anchors.rightMargin: 16 * virtualstudio.uiScale;
            font { family: "Poppins"; pixelSize: fontExtraSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            visible: !audio.audioReady
        }

        AudioSettings {
            id: audioSettings
            width: parent.width
            anchors.top: pageTitle.bottom
            anchors.topMargin: 16 * virtualstudio.uiScale
        }

        Rectangle {
            id: headerBorder
            width: parent.width
            height: 1
            anchors.top: audioSettings.top
            color: strokeColor
        }

        Rectangle {
            id: footerBorder
            width: parent.width
            height: 1
            anchors.top: audioSettings.bottom
            color: strokeColor
        }

        Rectangle {
            property int footerHeight: (30 + (rightMargin * 2)) * virtualstudio.uiScale;
            x: -1; y: parent.height - footerHeight;
            width: parent.width; height: footerHeight;
            border.color: "#33979797"
            color: backgroundColour

            Button {
                id: backButton
                background: Rectangle {
                    radius: 6 * virtualstudio.uiScale
                    color: backButton.down ? buttonPressedColour : buttonColour
                    border.width: 1
                    border.color: backButton.down || backButton.hovered ? buttonPressedStroke : buttonStroke
                }
                onClicked: { virtualstudio.windowState = "browse"; virtualstudio.studioToJoin = ""; audio.stopAudio(); }
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

            DeviceWarning {
                id: deviceWarning
                anchors.left: backButton.right
                anchors.leftMargin: 16 * virtualstudio.uiScale
                anchors.verticalCenter: backButton.verticalCenter
                visible: Boolean(audio.devicesError) || Boolean(audio.devicesWarning)
            }

            Button {
                id: saveButton
                background: Rectangle {
                    radius: 6 * virtualstudio.uiScale
                    color: saveButton.down ? saveButtonPressedColour : saveButtonBackgroundColour
                    border.width: 1
                    border.color: saveButton.down || saveButton.hovered ? saveButtonPressedStroke : saveButtonStroke
                }
                enabled: !Boolean(audio.devicesError) && audio.backendAvailable && audio.audioReady
                onClicked: {
                    audio.stopAudio(true);
                    virtualstudio.windowState = "connected";
                    virtualstudio.saveSettings();
                    virtualstudio.joinStudio();
                }
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
                    color: !Boolean(audio.devicesError) && audio.backendAvailable && audio.audioReady ? saveButtonText : disabledButtonText
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            CheckBox {
                id: showAgainCheckbox
                checked: virtualstudio.showDeviceSetup
                visible: audio.backendAvailable
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
}
