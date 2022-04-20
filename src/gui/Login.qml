import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    width: 696; height: 577
    
    Rectangle {
        width: parent.width; height: parent.height
        color: "#FAFBFB"
    }

    property bool failTextVisible: false
    
    onFailTextVisibleChanged: {
        authFailedText.visible = failTextVisible;
        loginButton.visible = failTextVisible || !virtualstudio.hasRefreshToken;
        backButton.visible = failTextVisible || !virtualstudio.hasRefreshToken;
        loggingInText.visible = !failTextVisible && virtualstudio.hasRefreshToken;
    }
    
    Image {
        id: loginLogo
        source: "logo.svg"
        x: 198; y: 110; width: 42; height: 76
    }

    Image {
        source: "jacktrip.png"
        anchors.bottom: loginLogo.bottom
        x: 260; width: 238; height: 56
    }

    Text {
        text: "Virtual Studio"
        font.family: "Poppins"
        font.pointSize: 28 * virtualstudio.fontScale
        anchors.horizontalCenter: parent.horizontalCenter
        y: 208
    }

    Text {
        id: loggingInText
        text: "Logging in..."
        font.family: "Poppins"
        font.pointSize: 18 * virtualstudio.fontScale
        anchors.horizontalCenter: parent.horizontalCenter
        y: 282
        visible: virtualstudio.hasRefreshToken
    }

    Text {
        id: authFailedText
        text: "Log in failed. Please try again."
        font.family: "Poppins"
        font.pointSize: 16 * virtualstudio.fontScale
        anchors.horizontalCenter: parent.horizontalCenter
        y: 272
        visible: failTextVisible
    }

    Button {
        id: loginButton
        background: Rectangle {
            radius: 6
            color: loginButton.down ? "#E5E6E6" : (loginButton.hovered ? "#EBECEC" : "#F0F1F1")
            border.width: loginButton.down ? 1 : 0
            border.color: "#A4A7A7"
            layer.enabled: !loginButton.down
            layer.effect: DropShadow {
                horizontalOffset: 1
                verticalOffset: 1
                radius: 8.0
                samples: 17
                color: "#80A1A1A1"
            }
        }
        onClicked: { failTextVisible = false; virtualstudio.login() }
        anchors.horizontalCenter: parent.horizontalCenter
        y: 321; width: 263; height: 64
        Text {
            text: "Sign In"
            font.family: "Poppins"
            font.pointSize: 18 * virtualstudio.fontScale
            font.weight: Font.Bold
            color: "#DB0A0A"
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
        }
        visible: !virtualstudio.hasRefreshToken
    }

    Button {
        id: backButton
        background: Rectangle {
            radius: 6
            color: backButton.down ? "#E5E6E6" : (backButton.hovered ? "#EBECEC" : "#F0F1F1")
            border.width: backButton.down ? 1 : 0
            border.color: "#A4A7A7"
            layer.enabled: !backButton.down
            layer.effect: DropShadow {
                horizontalOffset: 1
                verticalOffset: 1
                radius: 8.0
                samples: 17
                color: "#80A1A1A1"
            }
        }
        onClicked: { window.state = "start" }
        anchors.horizontalCenter: parent.horizontalCenter
        y: 401; width: 263; height: 64
        Text {
            text: "Back"
            font.family: "Poppins"
            font.pointSize: 18 * virtualstudio.fontScale
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
        }
        visible: !virtualstudio.hasRefreshToken
    }
}
