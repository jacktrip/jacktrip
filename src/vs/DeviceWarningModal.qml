import QtQuick
import QtQuick.Controls

Item {
    anchors.centerIn: parent
    width: 480 * virtualstudio.uiScale

    property int fontMedium: 12
    property int fontSmall: 10

    property string textColour: virtualstudio.darkMode ? "#FAFBFB" : "#0F0D0D"
    property string buttonColour: virtualstudio.darkMode ? "#494646" : "#EAECEC"
    property string buttonPressedStroke: virtualstudio.darkMode ? "#827D7D" : "#BABCBC"
    property string buttonPressedColour: virtualstudio.darkMode ? "#524F4F" : "#DEE0E0"
    property string buttonStroke: virtualstudio.darkMode ? "#80827D7D" : "#34979797"
    property string buttonHoverStroke: virtualstudio.darkMode ? "#7B7777" : "#BABCBC"
    property string buttonHoverColour: virtualstudio.darkMode ? "#5B5858" : "#D3D4D4"
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
            border.color: buttonStroke
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

                Button {
                    id: backButton
                    anchors.left: parent.left
                    anchors.leftMargin: 24 * virtualstudio.uiScale
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 24 * virtualstudio.uiScale
                    width: 160 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
                    onClicked: () => {
                        deviceWarningPopup.close();
                    }

                    background: Rectangle {
                        radius: 6 * virtualstudio.uiScale
                        color: backButton.down ? buttonPressedColour : (backButton.hovered ? buttonHoverColour : buttonColour)
                        border.width: 1
                        border.color: backButton.down ? buttonPressedStroke : (backButton.hovered ? buttonHoverStroke : buttonStroke)
                    }

                    Text {
                        text: "Back to Settings"
                        font.family: "Poppins"
                        font.pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale
                        color: textColour
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                Button {
                    id: connectButton
                    anchors.right: parent.right
                    anchors.rightMargin: 24 * virtualstudio.uiScale
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 24 * virtualstudio.uiScale
                    width: 160 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
                    onClicked: () => {
                        deviceWarningPopup.close();
                        audio.stopAudio(true);
                        virtualstudio.studioToJoin = virtualstudio.currentStudio.id;
                        virtualstudio.windowState = "connected";
                        virtualstudio.saveSettings();
                        virtualstudio.joinStudio();
                    }

                    background: Rectangle {
                        radius: 6 * virtualstudio.uiScale
                        color: connectButton.down ? buttonPressedColour : (connectButton.hovered ? buttonHoverColour : buttonColour)
                        border.width: 1
                        border.color: connectButton.down ? buttonPressedStroke : (connectButton.hovered ? buttonHoverStroke : buttonStroke)
                    }

                    Text {
                        text: "Connect to Session"
                        font.family: "Poppins"
                        font.pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale
                        color: textColour
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }
        }
    }

    function open () {
        deviceWarningPopup.open();
    }
}
