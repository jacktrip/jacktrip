import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15

Rectangle {
    width: 664; height: 83
    radius: 6
    color: "#F6F8F8"
    border.width: 0.3
    border.color: "#34979797"
    
    property string serverLocation: "Germany - Berlin"
    property string flagImage: "flags/DE.svg"
    property string studioName: "Test Studio"
    property bool publicStudio: false
    property bool manageable: false
    property bool available: true
    property bool connected: false
    
    Rectangle {
        width: 12; height: 83
        radius: 6
        color: "#0C1424"
    }
    
    Image {
        source: "wedge.svg"
        x: 6; y: 0; width: 52; height: 83
    }
    
    Image {
        source: "logo.svg"
        x: 8; y: 11; width: 32; height: 59
    }
    
    Rectangle {
        x: 33; y: 8; width: 32; height: 32
        radius: 16
        color: "#0C1424"
    }
    
    Image {
        id: flag
        source: flagImage
        x: 30; y: 9; width: 40; height: 30
        fillMode: Image.PreserveAspectCrop
        layer.enabled: true
        layer.effect: OpacityMask {
            maskSource: mask
        }
    }

    Rectangle {
        id: mask
        x: 0 ; y: 0 ; width: 40; height: 30
        visible: false
        color: "#00000000"
        Rectangle {
            x: 7; y: 3; width:24; height: 24
            radius: 12
        }
    }
    
    Text {
        x: 81; y: 11; width: manageable ? 431 : 508
        text: studioName
        font.family: "Poppins"
        font.pointSize: 18
        font.weight: Font.Bold
        elide: Text.ElideRight
    }
    
    Rectangle {
        id: publicRect
        x: 81; y: 52; width: 14; height: 14
        radius: 2
        color: publicStudio ? "#0095FF" : "#FF9800"
        Image {
            source: publicStudio ? "public.svg" : "private.svg"
            x: 1; y: 1; width: 12; height: 12
        }
    }
    
    Text {
        anchors.verticalCenter: publicRect.verticalCenter
        x: 103
        text: publicStudio ? "Public hub studio in " + serverLocation : "Private hub studio in " + serverLocation
        font.family: "Poppins"
        font.pointSize: 7
    }
    
    Button {
        id: joinButton
        x: manageable ? 522 : 599
        y: 13; width: 40; height: 40
        background: Rectangle {
            radius: 20
            color: connected ? "#FCB6B6" : "#C4F4BE"
        }
        visible: connected || canConnect
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
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            source: connected ? "leave.svg" : "join.svg"
        }
    }
    
    Text {
        anchors.horizontalCenter: joinButton.horizontalCenter
        y: 56
        text: connected ? "Leave" : "Join"
        font.family: "Poppins"
        font.pointSize: 11
        visible: connected || canConnect
    }
    
    Button {
        id: manageButton
        x: 599; y: 13; width: 40; height: 40
        background: Rectangle {
            radius: 20
            color: "#EAEBEB"
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
            width: 20; height: 20
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            source: "cog.svg"
        }
    }
    
    Text {
        anchors.horizontalCenter: manageButton.horizontalCenter
        y: 56
        text: "Manage"
        font.family: "Poppins"
        font.pointSize: 11
        visible: manageable
    }
}
