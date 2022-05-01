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
        text: "Which JackTrip experience do you need?"
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
        width: 234 * virtualstudio.uiScale; height: 197 * virtualstudio.uiScale
        Image {
            source: "jacktrip.png"
            anchors.horizontalCenter: parent.horizontalCenter
            y: 29 * virtualstudio.uiScale
            width: 180 * virtualstudio.uiScale; height: 42 * virtualstudio.uiScale;
        }
        Text {
            text: "Virtual Studio"
            font.family: "Poppins"
            font.pixelSize: 17 * virtualstudio.fontScale * virtualstudio.uiScale
            anchors.horizontalCenter: parent.horizontalCenter
            y: 73 * virtualstudio.uiScale
        }
        Text {
            text: "Simply connect to a JackTrip Virtual Studio"
            font.family: "Poppins"
            font.pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale
            anchors.horizontalCenter: parent.horizontalCenter
            y:130 * virtualstudio.uiScale; width: 190 * virtualstudio.uiScale
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
        }
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
        width: 234 * virtualstudio.uiScale; height: 197 * virtualstudio.uiScale
        Image {
            source: "jacktrip.png"
            anchors.horizontalCenter: parent.horizontalCenter
            y: 29 * virtualstudio.uiScale
            width: 180 * virtualstudio.uiScale; height: 42 * virtualstudio.uiScale;
        }
        Text {
            text: "Standard"
            font.family: "Poppins"
            font.pixelSize: 20 * virtualstudio.fontScale * virtualstudio.uiScale
            anchors.horizontalCenter: parent.horizontalCenter
            y: 73 * virtualstudio.uiScale
        }
        Text {
            text: "A more flexible, customizable JackTrip"
            font.family: "Poppins"
            font.pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale
            anchors.horizontalCenter: parent.horizontalCenter
            y:130 * virtualstudio.uiScale; width: 190 * virtualstudio.uiScale
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
        }
    }
}
