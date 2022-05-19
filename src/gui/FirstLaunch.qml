import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.12

Item {
    width: parent.width; height: parent.height
    clip: true
    
    Image {
        source: "logo.svg"
        anchors.horizontalCenter: parent.horizontalCenter
        y: 35 * virtualstudio.uiScale
        width: 50 * virtualstudio.uiScale; height: 92 * virtualstudio.uiScale
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 168 * virtualstudio.uiScale
        text: "Sign in with a Virtual Studio account?"
        font.family: "Poppins"
        font.pixelSize: 17 * virtualstudio.fontScale * virtualstudio.uiScale
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 219 * virtualstudio.uiScale
        text: "You'll be able to change your mind later"
        font.family: "Poppins"
        font.pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale
    }

    Button {
        id: vsButton
        background: Rectangle {
            radius: 10 * virtualstudio.uiScale
            color: vsButton.down ? "#E7E8E8" : "#F2F3F3"
            border.width: 1
            border.color: vsButton.down ? "#B0B5B5" : "#EAEBEB"
            layer.enabled: vsButton.hovered && !vsButton.down
            layer.effect: DropShadow {
                horizontalOffset: 1 * virtualstudio.uiScale
                verticalOffset: 1 * virtualstudio.uiScale
                radius: 8.0 * virtualstudio.uiScale
                samples: 17
                color: "#80A1A1A1"
            }
        }
        onClicked: { window.state = "login"; virtualstudio.toVirtualStudio(); }
        x: parent.width / 2 - (265 * virtualstudio.uiScale); y: 290 * virtualstudio.uiScale
        width: 234 * virtualstudio.uiScale; height: 49 * virtualstudio.uiScale
        Text {
            text: "Yes"
            font.family: "Poppins"
            font.pixelSize: 18 * virtualstudio.fontScale * virtualstudio.uiScale
            font.weight: Font.Bold
            color: "#DB0A0A"
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
        }
    }
    Text {
        text: "• Connect to Virtual Studios<br>• Broadcast on JackTrip Radio<br>• Apply FX with Soundscapes"
        textFormat: Text.StyledText
        font.family: "Poppins"
        font.pixelSize: 10 * virtualstudio.fontScale * virtualstudio.uiScale
        x: parent.width / 2 - (265 * virtualstudio.uiScale);
        y: 355 * virtualstudio.uiScale;
        width: 230 * virtualstudio.uiScale
        padding: 0
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter
    }
    Image {
        source: "JTVS.png"
        x: parent.width / 2 - (265 * virtualstudio.uiScale); y: 420 * virtualstudio.uiScale
        width: 234 * virtualstudio.uiScale; height: 201.48 * virtualstudio.uiScale;
    }

    Button {
        id: standardButton
        background: Rectangle {
            radius: 10 * virtualstudio.uiScale
            color: standardButton.down ? "#E7E8E8" : "#F2F3F3"
            border.width: 1
            border.color: standardButton.down ? "#B0B5B5" : "#EAEBEB"
            layer.enabled: standardButton.hovered && !standardButton.down
            layer.effect: DropShadow {
                horizontalOffset: 1 * virtualstudio.uiScale
                verticalOffset: 1 * virtualstudio.uiScale
                radius: 8.0 * virtualstudio.uiScale
                samples: 17
                color: "#80A1A1A1"
            }
        }
        onClicked: { window.state = "login"; virtualstudio.toStandard(); }
        x: parent.width / 2 + (32 * virtualstudio.uiScale); y: 290 * virtualstudio.uiScale
        width: 234 * virtualstudio.uiScale; height: 49 * virtualstudio.uiScale
        Text {
            text: "No"
            font.family: "Poppins"
            font.pixelSize: 18 * virtualstudio.fontScale * virtualstudio.uiScale
            font.weight: Font.Bold
            color: "#DB0A0A"
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
        }
    }
    Image {
        source: "JTOriginal.png"
        x: parent.width / 2 + (32 * virtualstudio.uiScale); y: 420 * virtualstudio.uiScale
        width: 234 * virtualstudio.uiScale; height: 337.37 * virtualstudio.uiScale;
    }
    Text {
        text: "• Connect via IP address<br>• Run a local hub server<br>• The Classic JackTrip experience"
        textFormat: Text.StyledText
        font.family: "Poppins"
        font.pixelSize: 10 * virtualstudio.fontScale * virtualstudio.uiScale
        x: parent.width / 2 + (32 * virtualstudio.uiScale);
        y: 355 * virtualstudio.uiScale;
        width: 230 * virtualstudio.uiScale
        padding: 0
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter
    }
}
