import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.12

Rectangle {
    width: 664; height: 83
    radius: 6
    color: "#F6F8F8"
    border.width: 0.3
    border.color: "#34979797"

    layer.enabled: true
    layer.effect: DropShadow {
        horizontalOffset: 1
        verticalOffset: 1
        radius: 8.0
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
        horizontalOffset: -1
        verticalOffset: -1
        radius: 8.0
        samples: 17
        color: "#80A1A1A1"
        source: shadow
    }
    
    Rectangle {
        width: 12; height: parent.height
        radius: width / 2
        color: available ? "#0C1424" : "#B3B3B3"
    }
    
    Image {
        source: available ? "wedge.svg" : "wedge_inactive.svg"
        x: 6; y: 0; width: 52; height: 83
    }
    
    Image {
        source: "logo.svg"
        x: 8; y: 11; width: 32; height: 59
    }
    
    Rectangle {
        x: 33; y: 8; width: 32; height: width
        radius: width / 2
        color: available ? "#0C1424" : "#B3B3B3"
    }
    
    Image {
        id: flag
        source: flagImage
        x: 30; y: 9; width: 40; height: width / 4 * 3
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
            x: 7; y: 3; width:24; height: width
            radius: width / 2
        }
    }
    
    Text {
        x: leftMargin; y: 11; width: manageable ? parent.width - 233 : parent.width - 156
        text: studioName
        font { family: "Poppins"; weight: Font.Bold; pixelSize: fontBig * virtualstudio.fontScale }
        elide: Text.ElideRight
    }
    
    Rectangle {
        id: publicRect
        x: leftMargin; y: 52; width: 14; height: width
        radius: 2
        color: publicStudio ? "#0095FF" : "#FF9800"
        Image {
            source: publicStudio ? "public.svg" : "private.svg"
            x: 1; y: x; width: 12; height: width
        }
    }
    
    Text {
        anchors.verticalCenter: publicRect.verticalCenter
        x: 103
        text: publicStudio ? "Public hub studio in " + serverLocation : "Private hub studio in " + serverLocation
        font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale }
    }
    
    Button {
        id: joinButton
        x: manageable ? parent.width - 142 : parent.width - 65
        y: topMargin; width: 40; height: width
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
            width: 22; height: 20
            anchors { verticalCenter: parent.verticalCenter; horizontalCenter: parent.horizontalCenter }
            source: connected ? "leave.svg" : "join.svg"
        }
    }
    
    Text {
        anchors.horizontalCenter: joinButton.horizontalCenter
        y: 56
        text: connected ? "Leave" : available ? "Join" : "Start"
        font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale }
        visible: connected || canConnect || canStart
    }
    
    Button {
        id: manageButton
        x: parent.width - 65; y: topMargin; width: 40; height: width
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
            width: 20; height: width
            anchors { verticalCenter: parent.verticalCenter; horizontalCenter: parent.horizontalCenter }
            source: "cog.svg"
        }
    }
    
    Text {
        anchors.horizontalCenter: manageButton.horizontalCenter
        y: 56
        text: "Manage"
        font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale }
        visible: manageable
    }
}
