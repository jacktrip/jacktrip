import QtQuick 2.12
import QtQuick.Controls 2.12
//import QtGraphicalEffects 1.15

Item {
    width: 696; height: 577
    
    Image {
        source: "logo.svg"
        anchors.horizontalCenter: parent.horizontalCenter
        y: 35; width: 50; height: 92
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 168
        text: "Which JackTrip experience do you need?"
        font.family: "Poppins"
        font.pointSize: 17 * virtualstudio.fontScale
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 219
        text: "You'll be able to change your mind later"
        font.family: "Poppins"
        font.pointSize: 11 * virtualstudio.fontScale
    }

    Button {
        id: vsButton
        background: Rectangle {
            radius: 10
            color: vsButton.down ? "#E7E8E8" : "#F2F3F3"
            border.width: vsButton.down ? 1 : 0.3
            border.color: vsButton.down ? "#B0B5B5" : "#EAEBEB"
            layer.enabled: vsButton.hovered && !vsButton.down
            layer.effect: DropShadow {
                horizontalOffset: 1
                verticalOffset: 1
                radius: 8.0
                samples: 17
                color: "#80A1A1A1"
            }
        }
        onClicked: { window.state = "login"; virtualstudio.toVirtualStudio(); }
        x: 83; y: 290; width: 234; height: 197
        Image {
            source: "jacktrip.png"
            anchors.horizontalCenter: parent.horizontalCenter
            y: 29; width: 180; height: 42;
        }
        Text {
            text: "Virtual Studio"
            font.family: "Poppins"
            font.pointSize: 17 * virtualstudio.fontScale
            anchors.horizontalCenter: parent.horizontalCenter
            y: 73
        }
        Text {
            text: "Simply connect to a JackTrip Virtual Studio"
            font.family: "Poppins"
            font.pointSize: 11 * virtualstudio.fontScale
            anchors.horizontalCenter: parent.horizontalCenter
            y:130; width: 190
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
        }
    }

    Button {
        id: standardButton
        background: Rectangle {
            radius: 10
            color: standardButton.down ? "#E7E8E8" : "#F2F3F3"
            border.width: standardButton.down ? 1 : 0.3
            border.color: standardButton.down ? "#B0B5B5" : "#EAEBEB"
            layer.enabled: standardButton.hovered && !standardButton.down
            layer.effect: DropShadow {
                horizontalOffset: 1
                verticalOffset: 1
                radius: 8.0
                samples: 17
                color: "#80A1A1A1"
            }
        }
        onClicked: { window.state = "login"; virtualstudio.toStandard(); }
        x: 380; y: 290; width: 234; height: 197
        Image {
            source: "jacktrip.png"
            anchors.horizontalCenter: parent.horizontalCenter
            y: 29; width: 180; height: 42;
        }
        Text {
            text: "Standard"
            font.family: "Poppins"
            font.pointSize: 20 * virtualstudio.fontScale
            anchors.horizontalCenter: parent.horizontalCenter
            y: 73
        }
        Text {
            text: "A more flexible, customizable JackTrip"
            font.family: "Poppins"
            font.pointSize: 11 * virtualstudio.fontScale
            anchors.horizontalCenter: parent.horizontalCenter
            y:130; width: 190
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
        }
    }
}
