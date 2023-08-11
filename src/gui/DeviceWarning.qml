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
        color: devicesWarningColour
    }

    InfoTooltip {
        id: devicesWarningTooltip
        anchors.left: warningOrErrorText.right
        anchors.leftMargin: 2 * virtualstudio.uiScale
        anchors.bottom: warningOrErrorText.bottom
        anchors.bottomMargin: 6 * virtualstudio.uiScale
        content: qsTr(virtualstudio.devicesError || virtualstudio.devicesWarning)
        iconColor: devicesWarningColour
        size: 16 * virtualstudio.uiScale
        visible: Boolean(virtualstudio.devicesError) || Boolean(virtualstudio.devicesWarning)
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