import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.12
import VS 1.0

Rectangle {
    width: 664; height: 83 * virtualstudio.uiScale
    radius: 6 * virtualstudio.uiScale
    color: backgroundColour
    
    property string serverLocation: "Germany - Berlin"
    property string flagImage: "flags/DE.svg"
    property string hostname: "app.jacktrip.org"
    property string studioName: "Test Studio"
    property string studioId: ""
    property string inviteKeyString: ""
    property bool publicStudio: false
    property bool admin: false
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
    property string tooltipStroke: virtualstudio.darkMode ? "#80827D7D" : "#34979797"

    property string baseButtonColour: virtualstudio.darkMode ? "#F0F1F1" : "#EAEBEB"
    property string baseButtonHoverColour: virtualstudio.darkMode ? "#CCCDCD" : "#D3D3D3"
    property string baseButtonPressedColour: virtualstudio.darkMode ? "#E4E5E5" : "#EAEBEB"
    property string baseButtonStroke: virtualstudio.darkMode ? "#8B8D8D" : "#949494"

    property string joinAvailableColour: virtualstudio.darkMode ? "#E2EBE0" : "#C4F4BE"
    property string joinAvailableHoverColour: virtualstudio.darkMode ? "#BAC7B8" : "#B0DCAB"
    property string joinAvailablePressedColour: virtualstudio.darkMode ? "#D8E2D6" : "#BAE8B5"
    property string joinAvailableStroke: virtualstudio.darkMode ? "#748F70" : "#5DB752"
    
    property string joinUnavailableColour: baseButtonColour
    property string joinUnavailableHoverColour: baseButtonHoverColour
    property string joinUnavailablePressedColour: baseButtonPressedColour
    property string joinUnavailableStroke: baseButtonStroke

    property string startColour: virtualstudio.darkMode ? "#E2EBE0" : "#C4F4BE"
    property string startHoverColour: virtualstudio.darkMode ? "#BAC7B8" : "#B0DCAB"
    property string startPressedColour: virtualstudio.darkMode ? "#D8E2D6" : "#BAE8B5"
    property string startStroke: virtualstudio.darkMode ? "#748F70" : "#5DB752"

    property string manageColour: baseButtonColour
    property string manageHoverColour: baseButtonHoverColour
    property string managePressedColour: baseButtonPressedColour
    property string manageStroke: baseButtonStroke

    property string leaveColour: virtualstudio.darkMode ? "#FCB6B6" : "#FCB6B6"
    property string leaveHoverColour: virtualstudio.darkMode ? "#D49696" : "#E3A4A4"
    property string leavePressedColour: virtualstudio.darkMode ? "#F2AEAE" : "#EFADAD"
    property string leaveStroke: virtualstudio.darkMode ? "#A65959" : "#C95E5E"

    property string studioStroke: virtualstudio.darkMode ? "#80827D7D" : "#34979797"

    border.width: 1
    border.color: studioStroke

    Clipboard {
        id: clipboard
    }

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

    Image {
        id: wedge
        source: available ? "wedge.svg" : "wedge_inactive.svg"
        x: 6; y: 0; width: 52 * virtualstudio.uiScale; height: 83 * virtualstudio.uiScale
        sourceSize: Qt.size(wedge.width,wedge.height)
        fillMode: Image.PreserveAspectFit
        smooth: true
    }

    Image {
        id: studioLogo
        source: "logo.svg"
        x: 8; y: 11; width: 32 * virtualstudio.uiScale; height: 59 * virtualstudio.uiScale
        sourceSize: Qt.size(studioLogo.width,studioLogo.height)
        fillMode: Image.PreserveAspectFit
        smooth: true
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
        width: (admin || connected) ? parent.width - (310 * virtualstudio.uiScale) : parent.width - (233 * virtualstudio.uiScale)
        text: studioName
        fontSizeMode: Text.HorizontalFit
        font { family: "Poppins"; weight: Font.Bold; pixelSize: fontBig * virtualstudio.fontScale * virtualstudio.uiScale }
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter
        color: textColour
    }
    
    Rectangle {
        id: publicRect
        x: leftMargin * virtualstudio.uiScale; y: 52 * virtualstudio.uiScale
        width: 14 * virtualstudio.uiScale; height: width
        radius: 2 * virtualstudio.uiScale
        color: publicStudio ? "#0095FF" : "#FF9800"
        Image {
            id: pubPriv
            source: publicStudio ? "public.svg" : "private.svg"
            x: 1 * virtualstudio.uiScale; y: x; width: 12 * virtualstudio.uiScale; height: width
            sourceSize: Qt.size(pubPriv.width,pubPriv.height)
            fillMode: Image.PreserveAspectFit
            smooth: true
        }
    }
    
    Text {
        anchors.verticalCenter: publicRect.verticalCenter
        x: (leftMargin + 22) * virtualstudio.uiScale
        width: (admin || connected) ? parent.width - (255 * virtualstudio.uiScale) : parent.width - (178 * virtualstudio.uiScale)
        text: publicStudio ? "Public hub studio " + serverLocation : "Private hub studio " + serverLocation
        font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
        elide: Text.ElideRight
        color: textColour
    }
    
    Button {
        id: joinButton
        x: (admin || connected) ? parent.width - (219 * virtualstudio.uiScale) : parent.width - (142 * virtualstudio.uiScale)
        y: topMargin * virtualstudio.uiScale; width: 40 * virtualstudio.uiScale; height: width
        background: Rectangle {
            radius: width / 2
            color: available ? (joinButton.down ? joinAvailablePressedColour : (joinButton.hovered ? joinAvailableHoverColour : joinAvailableColour))
                : (joinButton.down ? joinUnavailablePressedColour : (joinButton.hovered ? joinUnavailableHoverColour : joinUnavailableColour))
            border.width: joinButton.down ? 1 : 0
            border.color: available ? joinAvailableStroke : joinUnavailableStroke
        }
        visible: !connected
        onClicked: {
            virtualstudio.windowState = "connected";
            virtualstudio.connectToStudio(index);
        }
        Image {
            id: join
            width: 22 * virtualstudio.uiScale; height: 20 * virtualstudio.uiScale
            anchors { verticalCenter: parent.verticalCenter; horizontalCenter: parent.horizontalCenter }
            source: "join.svg"
            sourceSize: Qt.size(join.width,join.height)
            fillMode: Image.PreserveAspectFit
            smooth: true
        }
    }

    Button {
        id: leaveButton
        x: (admin || connected) ? parent.width - (219 * virtualstudio.uiScale) : parent.width - (142 * virtualstudio.uiScale)
        y: topMargin * virtualstudio.uiScale; width: 40 * virtualstudio.uiScale; height: width
        background: Rectangle {
            radius: width / 2
            color: leaveButton.down ? leavePressedColour : (leaveButton.hovered ? leaveHoverColour : leaveColour)
            border.width: leaveButton.down ? 1 : 0
            border.color: leaveStroke
        }
        visible: connected
        onClicked: {
            virtualstudio.disconnect();
        }
        Image {
            id: leave
            width: 22 * virtualstudio.uiScale; height: 20 * virtualstudio.uiScale
            anchors { verticalCenter: parent.verticalCenter; horizontalCenter: parent.horizontalCenter }
            source: "leave.svg"
            sourceSize: Qt.size(leave.width,leave.height)
            fillMode: Image.PreserveAspectFit
            smooth: true
        }
    }
    
    Text {
        anchors.horizontalCenter: joinButton.horizontalCenter
        y: 56 * virtualstudio.uiScale
        text: connected ? "Leave" : "Join"
        font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale}
        visible: true
        color: textColour
    }

    Button {
        id: inviteButton
        x: (admin || connected) ? parent.width - (142 * virtualstudio.uiScale) : parent.width - (65 * virtualstudio.uiScale)
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
            if (virtualstudio.testMode) {
                hostname = "test.jacktrip.org";
            }
            if (!inviteKeyString) {
                clipboard.setText(qsTr("https://" + hostname + "/studios/" + studioId + "?invited=true"));
            } else {
                clipboard.setText(qsTr("https://" + hostname + "/studios/" + studioId + "?invited=" + inviteKeyString));
            }
            copiedResetTimer.restart()
        }
        visible: true
        Image {
            id: shareImg
            width: 24 * virtualstudio.uiScale; height: width
            anchors { verticalCenter: parent.verticalCenter; horizontalCenter: parent.horizontalCenter }
            source: "share.svg"
            sourceSize: Qt.size(shareImg.width,shareImg.height)
            fillMode: Image.PreserveAspectFit
            smooth: true
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
                border.width: 1
                border.color: tooltipStroke

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
        visible: true
        color: textColour
    }
    
    Button {
        id: manageOrVideoButton
        x: parent.width - (65 * virtualstudio.uiScale); y: topMargin * virtualstudio.uiScale
        width: 40 * virtualstudio.uiScale; height: width
        background: Rectangle {
            radius: width / 2
            color: manageOrVideoButton.down ? managePressedColour : (manageOrVideoButton.hovered ? manageHoverColour : manageColour)
            border.width:  manageOrVideoButton.down ? 1 : 0
            border.color: manageStroke
        }
        onClicked: { 
            if (connected) {
                virtualstudio.launchVideo(-1)
            } else {
                virtualstudio.manageStudio(index);
            }
        }
        visible: admin || connected
        Image {
            id: manageImg
            width: 20 * virtualstudio.uiScale; height: width
            anchors { verticalCenter: parent.verticalCenter; horizontalCenter: parent.horizontalCenter }
            source: connected ? "video.svg" : "manage.svg"
            sourceSize: Qt.size(manageImg.width,manageImg.height)
            fillMode: Image.PreserveAspectFit
            smooth: true
        }
    }
    
    Text {
        anchors.horizontalCenter: manageOrVideoButton.horizontalCenter
        y: 56 * virtualstudio.uiScale
        text: connected ? "Video" : "Manage"
        font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
        visible: admin || connected
        color: textColour
    }
}
