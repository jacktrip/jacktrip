import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    required property var model
    property int bins: Math.max(15, width/20)
    property int innerMargin: 2 * virtualstudio.uiScale
    property int clipWidth: 8 * virtualstudio.uiScale
    property int boxRadius: 4 * virtualstudio.uiScale
    required property bool clipped
    property bool enabled: true
    property string meterColor: enabled ? (virtualstudio.darkMode ? "#5B5858" : "#D3D4D4") : (virtualstudio.darkMode ? "#7b7979" : "#EAECEC")
    property string meterRed: "#F21B1B"

    Item {
        id: meters
        width: parent.width - clipWidth
        height: parent.height

        MeterBars {
            id: leftchannel
            x: 0;
            y: 0;
            width: parent.width
            height: Math.min((parent.height/2 - innerMargin/2), 12) * virtualstudio.uiScale
            level: parent.parent.model[0]
            enabled: parent.parent.enabled
        }

        MeterBars {
            id: rightchannel
            x: 0;
            anchors.top: leftchannel.bottom
            anchors.topMargin: innerMargin
            width: parent.width
            height: Math.min((parent.height/2 - innerMargin/2), 12) * virtualstudio.uiScale
            level: parent.parent.model[1]
            enabled: parent.parent.enabled
        }
    }

    Rectangle {
        id: clipIndicator
        y: 0
        anchors.left: meters.right
        anchors.leftMargin: innerMargin

        width: Math.min(leftchannel.height, ((parent.width - clipWidth) / bins) - innerMargin)
        height: leftchannel.height + rightchannel.height + innerMargin
        radius: boxRadius
        color: clipped ? meterRed : meterColor
    }
}