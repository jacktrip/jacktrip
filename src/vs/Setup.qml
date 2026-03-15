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
    property string checkboxStroke: "#0062cc"
    property string checkboxPressedStroke: "#007AFF"

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

            StyledButton {
                id: backButton
                text: "Go Back"
                onClicked: { virtualstudio.windowState = "browse"; virtualstudio.studioToJoin = ""; audio.stopAudio(); }
                anchors.left: parent.left
                anchors.leftMargin: 16 * virtualstudio.uiScale
                anchors.bottomMargin: rightMargin * virtualstudio.uiScale
                anchors.bottom: parent.bottom
                width: 150 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
            }

            DeviceWarning {
                id: deviceWarning
                anchors.left: backButton.right
                anchors.leftMargin: 16 * virtualstudio.uiScale
                anchors.verticalCenter: backButton.verticalCenter
                visible: Boolean(audio.devicesError) || Boolean(audio.devicesWarning)
            }

            StyledButton {
                id: saveButton
                text: "Connect to Session"
                primary: true
                enabled: !Boolean(audio.devicesError) && audio.backendAvailable && audio.audioReady
                onClicked: {
                    if (Boolean(audio.devicesWarning)) {
                        deviceWarningModal.open();
                    } else {
                        audio.stopAudio(true);
                        virtualstudio.windowState = "connected";
                        virtualstudio.saveSettings();
                        virtualstudio.joinStudio();
                    }
                }
                anchors.right: parent.right
                anchors.rightMargin: rightMargin * virtualstudio.uiScale
                anchors.bottomMargin: rightMargin * virtualstudio.uiScale
                anchors.bottom: parent.bottom
                width: 160 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
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

        DeviceWarningModal {
            id: deviceWarningModal
        }
    }
}
