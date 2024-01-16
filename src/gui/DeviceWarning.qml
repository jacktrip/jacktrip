import QtQuick
import QtQuick.Controls

Item {
    height: 28 * virtualstudio.uiScale
    property string devicesWarningColour: "#F21B1B"

    AppIcon {
        id: devicesWarningIcon
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        width: parent.height
        height: parent.height
        icon.source: "warning.svg"
        color: devicesWarningColour
        visible: Boolean(audio.devicesError) || Boolean(audio.devicesWarning)
    }
    
    Text {
        id: warningOrErrorText
        text: Boolean(audio.devicesError) ? "Audio Configuration Error" : "Audio Configuration Warning"
        anchors.left: devicesWarningIcon.right
        anchors.leftMargin: 4 * virtualstudio.uiScale
        anchors.verticalCenter: devicesWarningIcon.verticalCenter
        visible: Boolean(audio.devicesError) || Boolean(audio.devicesWarning)
        font { family: "Poppins"; pixelSize: 9 * virtualstudio.fontScale * virtualstudio.uiScale }
        color: devicesWarningColour
    }

    InfoTooltip {
        id: devicesWarningTooltip
        anchors.left: warningOrErrorText.right
        anchors.leftMargin: 2 * virtualstudio.uiScale
        anchors.top: devicesWarningIcon.top
        content: qsTr(audio.devicesError || audio.devicesWarning)
        iconColor: devicesWarningColour
        size: 16 * virtualstudio.uiScale
        visible: Boolean(audio.devicesError) || Boolean(audio.devicesWarning)
    }

    MouseArea {
        id: devicesWarningToolTipArea
        anchors.top: devicesWarningIcon.top
        anchors.bottom: devicesWarningIcon.bottom
        anchors.left: devicesWarningIcon.left
        anchors.right: warningOrErrorText.right
        hoverEnabled: true
        onEntered: devicesWarningTooltip.showToolTip = true
        onExited: devicesWarningTooltip.showToolTip = false
    }
}
