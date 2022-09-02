import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.12

Rectangle {
    width: 664; height: 83 * virtualstudio.uiScale
    radius: 6 * virtualstudio.uiScale
    color: backgroundColour
    border.width: 0.3
    border.color: "#40979797"

    layer.enabled: true
    layer.effect: DropShadow {
        horizontalOffset: 1 * virtualstudio.uiScale
        verticalOffset: 1 * virtualstudio.uiScale
        radius: 8.0 * virtualstudio.uiScale
        samples: 17
        color: shadowColour
    }
    
    property string serverLocation: "Germany - Berlin"
    property string flagImage: "flags/DE.svg"
    property string studioName: "Test Studio"
    property bool publicStudio: false
    property bool manageable: false
    property bool available: true
    property bool connected: false
    property bool inviteCopied: false
    
    property int leftMargin: 81
    property int topMargin: 13
    property int bottomToolTipMargin: 8
    property int rightToolTipMargin: 4
    
    property real fontBig: 18
    property real fontMedium: 11
    property real fontSmall: 8
    
    property string backgroundColour: virtualstudio.darkMode ? "#494646" : "#F4F6F6"
    property string textColour: virtualstudio.darkMode ? "#FAFBFB" : "#0F0D0D"
    property string shadowColour: virtualstudio.darkMode ? "#40000000" : "#80A1A1A1"
    property string toolTipBackgroundColour: inviteCopied ? "#57B147" : (virtualstudio.darkMode ? "#323232" : "#F3F3F3")
    property string toolTipTextColour: inviteCopied ? "#FAFBFB" : textColour
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

    DropShadow {
        horizontalOffset: -1 * virtualstudio.uiScale
        verticalOffset: -1 * virtualstudio.uiScale
        radius: 8.0 * virtualstudio.uiScale
        samples: 17
        color: shadowColour
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
        width: manageable ? parent.width - (310 * virtualstudio.uiScale) : parent.width - (233 * virtualstudio.uiScale)
        text: studioName
        fontSizeMode: Text.HorizontalFit
        font { family: "Poppins"; weight: Font.Bold; pixelSize: fontBig * virtualstudio.fontScale * virtualstudio.uiScale }
        elide: Text.ElideRight
        color: textColour
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
        text: publicStudio ? "Public hub studio " + serverLocation : "Private hub studio " + serverLocation
        font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
        elide: Text.ElideRight
        color: textColour
    }
    
    Button {
        id: joinButton
        x: manageable ? parent.width - (219 * virtualstudio.uiScale) : parent.width - (142 * virtualstudio.uiScale)
        y: topMargin * virtualstudio.uiScale; width: 40 * virtualstudio.uiScale; height: width
        background: Rectangle {
            radius: width / 2
            color: joinButton.down ? joinPressedColour : (joinButton.hovered ? joinHoverColour : joinColour)
            border.width: joinButton.down ? 1 : 0
            border.color: joinStroke
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
        color: textColour
    }

    Button {
        id: inviteButton
        x: manageable ? parent.width - (142 * virtualstudio.uiScale) : parent.width - (65 * virtualstudio.uiScale)
        y: topMargin * virtualstudio.uiScale; width: 40 * virtualstudio.uiScale; height: width
        background: Rectangle {
            radius: width / 2
            color: inviteButton.down ? managePressedColour : (inviteButton.hovered ? manageHoverColour : manageColour)
            border.width:  inviteButton.down ? 1 : 0
            border.color: manageStroke
        }
        Timer {
            id: copiedResetTimer
            interval: 2000; running: false; repeat: false
            onTriggered: inviteCopied = false;
        }
        onClicked: { 
            inviteCopied = true;
            copiedResetTimer.restart()
        }
        visible: connected || canConnect
        Image {
            width: 20 * virtualstudio.uiScale; height: width
            anchors { verticalCenter: parent.verticalCenter; horizontalCenter: parent.horizontalCenter }
            source: "share.svg"
        }
        ToolTip {
            parent: inviteButton
            visible: inviteButton.hovered || inviteCopied
            bottomPadding: bottomToolTipMargin * virtualstudio.uiScale
            rightPadding: rightToolTipMargin * virtualstudio.uiScale
            delay: 100
            contentItem: Rectangle {
                color: toolTipBackgroundColour
                radius: 3
                anchors.fill: parent
                anchors.bottomMargin: bottomToolTipMargin * virtualstudio.uiScale
                anchors.rightMargin: rightToolTipMargin * virtualstudio.uiScale
                layer.enabled: true
                layer.effect: DropShadow {
                    horizontalOffset: 1 * virtualstudio.uiScale
                    verticalOffset: 1 * virtualstudio.uiScale
                    radius: 10.0 * virtualstudio.uiScale
                    samples: 21
                    color: shadowColour
                }

                Text {
                    anchors.centerIn: parent
                    font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale}
                    text: inviteCopied ?  qsTr("📋 Copied invitation link to Clipboard") : qsTr("Copy invite link for Studio")
                    color: toolTipTextColour
                }
            }
            background: Rectangle {
                color: "transparent"
            }
        }
    }
    
    Text {
        anchors.horizontalCenter: inviteButton.horizontalCenter
        y: 56 * virtualstudio.uiScale
        text: "Invite"
        font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
        visible: connected || canConnect
        color: textColour
    }
    
    Button {
        id: manageButton
        x: parent.width - (65 * virtualstudio.uiScale); y: topMargin * virtualstudio.uiScale
        width: 40 * virtualstudio.uiScale; height: width
        background: Rectangle {
            radius: width / 2
            color: manageButton.down ? managePressedColour : (manageButton.hovered ? manageHoverColour : manageColour)
            border.width:  manageButton.down ? 1 : 0
            border.color: manageStroke
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
            source: "manage.svg"
        }
    }
    
    Text {
        anchors.horizontalCenter: manageButton.horizontalCenter
        y: 56 * virtualstudio.uiScale
        text: "Manage"
        font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
        visible: manageable
        color: textColour
    }
}
