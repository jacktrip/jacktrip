import QtQuick
import QtQuick.Controls

Item {
    anchors.centerIn: parent
    width: 480 * virtualstudio.uiScale

    property int fontMedium: 12
    property int fontSmall: 10

    property string textColour: virtualstudio.darkMode ? "#FAFBFB" : "#0F0D0D"
    property string devicesWarningColour: "#F21B1B"

    Popup {
        id: deviceWarningPopup
        padding: 1
        width: parent.width
        height: 350 * virtualstudio.uiScale
        anchors.centerIn: parent
        modal: true
        focus: true

        background: Rectangle {
            anchors.fill: parent
            color: "transparent"
            radius: 6 * virtualstudio.uiScale
            border.width: 1
            border.color: virtualstudio.darkMode ? "#80827D7D" : "#34979797"
            clip: true
        }

        contentItem: Rectangle {
            width: parent.width
            height: parent.height
            color: backgroundColour
            radius: 6 * virtualstudio.uiScale

            Item {
                id: deviceWarningPopupContent
                anchors.top: parent.top
                anchors.topMargin: 24 * virtualstudio.uiScale
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right

                AppIcon {
                    id: devicesWarningIcon
                    anchors.top: parent.top
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: 56 * virtualstudio.uiScale
                    height: 56 * virtualstudio.uiScale
                    icon.source: "warning.svg"
                    color: devicesWarningColour
                }

                Text {
                    id: deviceWarningPopupHeader
                    anchors.top: devicesWarningIcon.bottom
                    anchors.topMargin: 16 * virtualstudio.uiScale
                    width: parent.width
                    text: "Audio Configuration Warning"
                    font {family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale; bold: true }
                    horizontalAlignment: Text.AlignHCenter
                    color: textColour
                    elide: Text.ElideRight
                    wrapMode: Text.WordWrap
                }

                Text {
                    id: devicesWarningText
                    anchors.top: deviceWarningPopupHeader.bottom
                    anchors.topMargin: 16 * virtualstudio.uiScale
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - (32 * virtualstudio.uiScale)
                    text: qsTr(audio.devicesWarning)
                    font {family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                    horizontalAlignment: Text.AlignHCenter
                    color: textColour
                    elide: Text.ElideRight
                    wrapMode: Text.WordWrap
                }

                LearnMoreButton {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: devicesWarningText.bottom
                    anchors.topMargin: 24 * virtualstudio.uiScale
                    url: Boolean(audio.devicesErrorHelpUrl) ? audio.devicesErrorHelpUrl : audio.devicesWarningHelpUrl
                    visible: Boolean(audio.devicesErrorHelpUrl) || Boolean(audio.devicesWarningHelpUrl)
                }

                StyledButton {
                    id: backButton
                    text: "Back to Settings"
                    anchors.left: parent.left
                    anchors.leftMargin: 24 * virtualstudio.uiScale
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 24 * virtualstudio.uiScale
                    width: 160 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
                    onClicked: () => {
                        deviceWarningPopup.close();
                    }
                }

                StyledButton {
                    id: connectButton
                    text: "Connect to Session"
                    primary: true
                    anchors.right: parent.right
                    anchors.rightMargin: 24 * virtualstudio.uiScale
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 24 * virtualstudio.uiScale
                    width: 160 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
                    onClicked: () => {
                        deviceWarningPopup.close();
                        audio.stopAudio(true);
                        virtualstudio.windowState = "connected";
                        virtualstudio.saveSettings();
                        virtualstudio.joinStudio();
                    }
                }
            }
        }
    }

    function open () {
        deviceWarningPopup.open();
    }
}
