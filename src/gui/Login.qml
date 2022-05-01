import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.12

Item {
    width: parent.width; height: parent.height
    clip: true
    
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
        x: parent.width / 2 - (150 * virtualstudio.uiScale); y: 110 * virtualstudio.uiScale
        width: 42 * virtualstudio.uiScale; height: 76 * virtualstudio.uiScale
    }

    Image {
        source: "jacktrip.png"
        anchors.bottom: loginLogo.bottom
        x: parent.width / 2 - (88 * virtualstudio.uiScale)
        width: 238 * virtualstudio.uiScale; height: 56 * virtualstudio.uiScale
    }

    Text {
        text: "Virtual Studio"
        font.family: "Poppins"
        font.pixelSize: 28 * virtualstudio.fontScale * virtualstudio.uiScale
        anchors.horizontalCenter: parent.horizontalCenter
        y: 208 * virtualstudio.uiScale
    }

    Text {
        id: loggingInText
        text: "Logging in..."
        font.family: "Poppins"
        font.pixelSize: 18 * virtualstudio.fontScale * virtualstudio.uiScale
        anchors.horizontalCenter: parent.horizontalCenter
        y: 282 * virtualstudio.uiScale
        visible: virtualstudio.hasRefreshToken
    }

    Text {
        id: authFailedText
        text: "Log in failed. Please try again."
        font.family: "Poppins"
        font.pixelSize: 16 * virtualstudio.fontScale * virtualstudio.uiScale
        anchors.horizontalCenter: parent.horizontalCenter
        y: 272 * virtualstudio.uiScale
        visible: failTextVisible
    }

    Button {
        id: loginButton
        background: Rectangle {
            radius: 6 * virtualstudio.uiScale
            color: loginButton.down ? "#E5E6E6" : (loginButton.hovered ? "#EBECEC" : "#F0F1F1")
            border.width: loginButton.down ? 1 : 0
            border.color: "#A4A7A7"
            layer.enabled: !loginButton.down
            layer.effect: DropShadow {
                horizontalOffset: 1 * virtualstudio.uiScale
                verticalOffset: 1 * virtualstudio.uiScale
                radius: 8.0 * virtualstudio.uiScale
                samples: 17
                color: "#80A1A1A1"
            }
        }
        onClicked: { failTextVisible = false; virtualstudio.login() }
        anchors.horizontalCenter: parent.horizontalCenter
        y: 321 * virtualstudio.uiScale
        width: 263 * virtualstudio.uiScale; height: 64 * virtualstudio.uiScale
        Text {
            text: "Sign In"
            font.family: "Poppins"
            font.pixelSize: 18 * virtualstudio.fontScale * virtualstudio.uiScale
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
            radius: 6 * virtualstudio.uiScale
            color: backButton.down ? "#E5E6E6" : (backButton.hovered ? "#EBECEC" : "#F0F1F1")
            border.width: backButton.down ? 1 : 0
            border.color: "#A4A7A7"
            layer.enabled: !backButton.down
            layer.effect: DropShadow {
                horizontalOffset: 1 * virtualstudio.uiScale
                verticalOffset: 1 * virtualstudio.uiScale
                radius: 8.0 * virtualstudio.uiScale
                samples: 17
                color: "#80A1A1A1"
            }
        }
        onClicked: { window.state = "start" }
        anchors.horizontalCenter: parent.horizontalCenter
        y: 401 * virtualstudio.uiScale
        width: 263 * virtualstudio.uiScale; height: 64 * virtualstudio.uiScale
        Text {
            text: "Back"
            font.family: "Poppins"
            font.pixelSize: 18 * virtualstudio.fontScale * virtualstudio.uiScale
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
        }
        visible: !virtualstudio.hasRefreshToken
    }
}
