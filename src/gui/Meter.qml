import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects

Item {
    required property var model
    property int bins: 15
    property int innerMargin: 2 * virtualstudio.uiScale
    property int clipWidth: 10 * virtualstudio.uiScale
    required property bool clipped
    property bool enabled: true
    property string meterColor: enabled ? (virtualstudio.darkMode ? "#5B5858" : "#D3D4D4") : "#EAECEC"
    property string meterRed: "#F21B1B"

    Item {
        id: meters
        x: 0; y: 0
        width: parent.width - clipWidth
        height: parent.height

        MeterBars {
            id: leftchannel
            x: 0;
            y: 0;
            width: parent.width - clipWidth
            height: 14 * virtualstudio.uiScale
            level: parent.parent.model[0]
            enabled: parent.parent.enabled
        }

        MeterBars {
            id: rightchannel
            x: 0;
            y: leftchannel.height
            width: parent.width - clipWidth
            height: 14 * virtualstudio.uiScale
            level: parent.parent.model[1]
            enabled: parent.parent.enabled
        }
    }

    Rectangle {
        id: clipIndicator
        x: meters.x + meters.width; y: 0
        width: Math.min(clipWidth, ((parent.width - clipWidth) / bins) - innerMargin)
        height: 24 * virtualstudio.uiScale
        radius: 4 * virtualstudio.uiScale
        color: clipped ? meterRed : meterColor
    }
}