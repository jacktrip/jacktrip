import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.12

Rectangle {
    width: 664; height: 83 * virtualstudio.uiScale
    radius: 6 * virtualstudio.uiScale
    color: "#F6F8F8"
    border.width: 0.3
    border.color: "#34979797"

    layer.enabled: true
    layer.effect: DropShadow {
        horizontalOffset: 1 * virtualstudio.uiScale
        verticalOffset: 1 * virtualstudio.uiScale
        radius: 8.0 * virtualstudio.uiScale
        samples: 17
        color: "#80A1A1A1"
    }
    
    property string serverLocation: "Germany - Berlin"
    property string flagImage: "flags/DE.svg"
    property string studioName: "Test Studio"
    property bool publicStudio: false
    property bool manageable: false
    property bool available: true
    property bool connected: false
    
    property int leftMargin: 81
    property int topMargin: 13
    
    property real fontBig: 18
    property real fontMedium: 11
    property real fontSmall: 8

    Rectangle {
        id: shadow
        anchors.fill: parent
        color: "transparent"
        radius: 6
    }

    DropShadow {
        horizontalOffset: -1 * virtualstudio.uiScale
        verticalOffset: -1 * virtualstudio.uiScale
        radius: 8.0 * virtualstudio.uiScale
        samples: 17
        color: "#80A1A1A1"
        source: shadow
    }
    
    Rectangle {
        width: 12 * virtualstudio.uiScale; height: parent.height
        radius: width / 2
        color: available ? "#0C1424" : "#B3B3B3"
    }
    
    Image {
        source: available ? "wedge.svg" : "wedge_inactive.svg"
        x: 6; y: 0; width: 52 * virtualstudio.uiScale; height: 83 * virtualstudio.uiScale
    }
    
    Image {
        source: "logo.svg"
        x: 8; y: 11; width: 32 * virtualstudio.uiScale; height: 59 * virtualstudio.uiScale
    }
    
    Rectangle {
        x: 33 * virtualstudio.uiScale; y: 8 * virtualstudio.uiScale
        width: 32 * virtualstudio.uiScale; height: width
        radius: width / 2
        color: available ? "#0C1424" : "#B3B3B3"
    }
    
    Image {
        id: flag
        source: flagImage
        x: 30 * virtualstudio.uiScale; y: 9 * virtualstudio.uiScale
        width: 40 * virtualstudio.uiScale; height: width / 4 * 3
        fillMode: Image.PreserveAspectCrop
        layer.enabled: true
        layer.effect: OpacityMask {
            maskSource: mask
        }
    }

    Rectangle {
        id: mask
        x: 0 ; y: 0 ; width: flag.width; height: flag.height
        visible: false
        color: "#00000000"
        Rectangle {
            x: 7 * virtualstudio.uiScale; y: 3 * virtualstudio.uiScale
            width:24 * virtualstudio.uiScale; height: width
            radius: width / 2
        }
    }
    
    Text {
        x: leftMargin * virtualstudio.uiScale; y: 11 * virtualstudio.uiScale;
        width: manageable ? parent.width - (233 * virtualstudio.uiScale) : parent.width - (156 * virtualstudio.uiScale)
        text: studioName
        font { family: "Poppins"; weight: Font.Bold; pixelSize: fontBig * virtualstudio.fontScale * virtualstudio.uiScale}
        elide: Text.ElideRight
    }
    
    Rectangle {
        id: publicRect
        x: leftMargin * virtualstudio.uiScale; y: 52 * virtualstudio.uiScale
        width: 14 * virtualstudio.uiScale; height: width
        radius: 2 * virtualstudio.uiScale
        color: publicStudio ? "#0095FF" : "#FF9800"
        Image {
            source: publicStudio ? "public.svg" : "private.svg"
            x: 1 * virtualstudio.uiScale; y: x; width: 12 * virtualstudio.uiScale; height: width
        }
    }
    
    Text {
        anchors.verticalCenter: publicRect.verticalCenter
        x: (leftMargin + 22) * virtualstudio.uiScale
        width: manageable ? parent.width - (255 * virtualstudio.uiScale) : parent.width - (178 * virtualstudio.uiScale)
        text: publicStudio ? "Public hub studio in " + serverLocation : "Private hub studio in " + serverLocation
        font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
        elide: Text.ElideRight
    }
    
    Button {
        id: joinButton
        x: manageable ? parent.width - (142 * virtualstudio.uiScale) : parent.width - (65 * virtualstudio.uiScale)
        y: topMargin * virtualstudio.uiScale; width: 40 * virtualstudio.uiScale; height: width
        background: Rectangle {
            radius: width / 2
            color: connected ? (joinButton.down ? "#F6B2B2" : (joinButton.hovered ? "#F1AEAE" : "#FCB6B6")) :
                (joinButton.down ? "#C0EFBA" : (joinButton.hovered ? "#BBE9B5" :"#C4F4BE"))
            border.width: joinButton.down ? 1 : 0
            border.color: connected ? "#F92755" : "#5CB752"
        }
        visible: connected || canConnect || canStart
        onClicked: {
            if (!connected) {
                window.state = "connected";
                virtualstudio.connectToStudio(index);
            } else {
                virtualstudio.disconnect();
            }
        }
        Image {
            width: 22 * virtualstudio.uiScale; height: 20 * virtualstudio.uiScale
            anchors { verticalCenter: parent.verticalCenter; horizontalCenter: parent.horizontalCenter }
            source: connected ? "leave.svg" : "join.svg"
        }
    }
    
    Text {
        anchors.horizontalCenter: joinButton.horizontalCenter
        y: 56 * virtualstudio.uiScale
        text: connected ? "Leave" : available ? "Join" : "Start"
        font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale}
        visible: connected || canConnect || canStart
    }
    
    Button {
        id: manageButton
        x: parent.width - (65 * virtualstudio.uiScale); y: topMargin * virtualstudio.uiScale
        width: 40 * virtualstudio.uiScale; height: width
        background: Rectangle {
            radius: width / 2
            color: (manageButton.hovered && !manageButton.pressed) ? "#DFE0E0" : "#EAEBEB"
            border.width:  manageButton.down ? 1 : 0
            border.color: "#949494"
        }
        onClicked: { 
            if (!connected) {
                virtualstudio.manageStudio(index)
            } else {
                virtualstudio.manageStudio(-1)
            }
        }
        visible: manageable
        Image {
            width: 20 * virtualstudio.uiScale; height: width
            anchors { verticalCenter: parent.verticalCenter; horizontalCenter: parent.horizontalCenter }
            source: "cog.svg"
        }
    }
    
    Text {
        anchors.horizontalCenter: manageButton.horizontalCenter
        y: 56 * virtualstudio.uiScale
        text: "Manage"
        font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
        visible: manageable
    }
}
