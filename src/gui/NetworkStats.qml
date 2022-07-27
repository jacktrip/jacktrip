import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.12

Rectangle {
    width: 664; height: 83 * virtualstudio.uiScale
    property int packetsSent: 0
    property int packetsReceived: 0
    property double minRtt: 0.0
    property double maxRtt: 0.0
    property double avgRtt: 0.0
    property double stdDevRtt: 0.0

    property int leftMargin: 81
    property int topMargin: 13
    
    property real fontBig: 18
    property real fontMedium: 11
    property real fontSmall: 8
    
    property string backgroundColour: virtualstudio.darkMode ? "#494646" : "#F4F6F6"
    property string textColour: virtualstudio.darkMode ? "#FAFBFB" : "#0F0D0D"
    property string shadowColour: virtualstudio.darkMode ? "40000000" : "#80A1A1A1"
    property string joinColour: virtualstudio.darkMode ? (connected ? "#FCB6B6" : "#E2EBE0") : (connected ? "#FCB6B6" : "#C4F4BE")
    property string joinHoverColour: virtualstudio.darkMode ? (connected ? "#D49696" : "#BAC7B8") : (connected ? "#E3A4A4" : "#B0DCAB")
    property string joinPressedColour: virtualstudio.darkMode ? (connected ? "#F2AEAE" : "#D8E2D6") : (connected ? "#EFADAD" : "#BAE8B5")
    property string joinStroke: virtualstudio.darkMode ? (connected ? "#A65959" : "#748F70") : (connected ? "#C95E5E" : "#5DB752")
    property string manageColour: virtualstudio.darkMode ? "#F0F1F1" : "#EAEBEB"
    property string manageHoverColour: virtualstudio.darkMode ? "#CCCDCD" : "#D3D3D3"
    property string managePressedColour: virtualstudio.darkMode ? "#E4E5E5" : "#EAEBEB"
    property string manageStroke: virtualstudio.darkMode ? "#8B8D8D" : "#949494"

    Rectangle {
        id: shadow
        anchors.fill: parent
        color: "transparent"
        radius: 6
    }
    
    Rectangle {
        width: 12 * virtualstudio.uiScale; height: parent.height
        radius: width / 2
        color: available ? "#0C1424" : "#B3B3B3"
    }
    
    Rectangle {
        id: publicRect
        x: leftMargin * virtualstudio.uiScale; y: 52 * virtualstudio.uiScale
        width: 14 * virtualstudio.uiScale; height: width
    }
    
    Text {
        anchors.left: publicRect.left
        x: (leftMargin + 22) * virtualstudio.uiScale
        width: manageable ? parent.width - (255 * virtualstudio.uiScale) : parent.width - (178 * virtualstudio.uiScale)
        text: minRtt > 0 && maxRtt > 0 ? minRtt + " ms - " + maxRtt + " ms, avg " + avgRtt + " ms round-trip time" : "Measuring stats..."
        font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
        elide: Text.ElideRight
        color: textColour
    }

    Text {
        anchors.left: publicRect.left
        x: (leftMargin + 22) * virtualstudio.uiScale
        y: (24) * virtualstudio.uiScale
        width: manageable ? parent.width - (255 * virtualstudio.uiScale) : parent.width - (178 * virtualstudio.uiScale)
        text: minRtt > 0 && maxRtt > 0 ? (100 - (packetsReceived * 100 / packetsSent)) + "% packet loss" : ""
        font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
        elide: Text.ElideRight
        color: textColour
    }
}
