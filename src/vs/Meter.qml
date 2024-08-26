import QtQuick
import QtQuick.Controls

Item {
    required property var model
    property int bins: Math.max(15, width/20)
    property int innerMargin: 2 * virtualstudio.uiScale
    property int boxRadius: 3 * virtualstudio.uiScale
    property int boxThickness: 12
    required property bool clipped
    property bool enabled: true
    property string meterColor: enabled ? (virtualstudio.darkMode ? "#5B5858" : "#D3D4D4") : (virtualstudio.darkMode ? "#7b7979" : "#EAECEC")
    property string meterRed: "#F21B1B"

    Item {
        id: meters
        width: parent.width - boxThickness - innerMargin
        height: parent.height

        MeterBars {
            id: leftchannel
            x: 0
            y: 0
            width: parent.width
            height: boxThickness
            level: parent.parent.model[0]
            enabled: parent.parent.enabled
        }

        MeterBars {
            id: rightchannel
            x: 0;
            anchors.top: leftchannel.bottom
            anchors.topMargin: innerMargin
            width: parent.width
            height: boxThickness
            level: parent.parent.model[1]
            enabled: parent.parent.enabled
        }
    }

    Rectangle {
        id: clipIndicator
        y: 0
        anchors.left: meters.right
        anchors.leftMargin: innerMargin

        width: boxThickness
        height: leftchannel.height + rightchannel.height + innerMargin
        radius: boxRadius
        color: clipped ? meterRed : meterColor
    }
}