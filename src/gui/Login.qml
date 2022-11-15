import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.12

Item {
    width: parent.width; height: parent.height
    clip: true
    
    Rectangle {
        width: parent.width; height: parent.height
        color: backgroundColour
    }

    property bool failTextVisible: false
    
    property string backgroundColour: virtualstudio.darkMode ? "#272525" : "#FAFBFB"
    property string textColour: virtualstudio.darkMode ? "#FAFBFB" : "#0F0D0D"
    property string buttonColour: virtualstudio.darkMode ? "#FAFBFB" : "#F0F1F1"
    property string buttonHoverColour: virtualstudio.darkMode ? "#E9E9E9" : "#E4E5E5"
    property string buttonPressedColour: virtualstudio.darkMode ? "#FAFBFB" : "#E4E5E5"
    property string buttonStroke: virtualstudio.darkMode ? "#9C9C9C" : "#A4A7A7"
    property string buttonTextColour: virtualstudio.darkMode ? "#272525" : "#DB0A0A"
    property string buttonTextHover: virtualstudio.darkMode ? "#242222" : "#D00A0A"
    property string buttonTextPressed: virtualstudio.darkMode ? "#323030" : "#D00A0A"
    property string shadowColour: virtualstudio.darkMode ? "40000000" : "#80A1A1A1"
    
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
        sourceSize: Qt.size(loginLogo.width,loginLogo.height)
        fillMode: Image.PreserveAspectFit
        smooth: true
    }

    Image {
        source: virtualstudio.darkMode ? "jacktrip white.png" : "jacktrip.png"
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
        color: textColour
    }

    Text {
        id: loggingInText
        text: "Logging in..."
        font.family: "Poppins"
        font.pixelSize: 18 * virtualstudio.fontScale * virtualstudio.uiScale
        anchors.horizontalCenter: parent.horizontalCenter
        y: 282 * virtualstudio.uiScale
        visible: virtualstudio.hasRefreshToken
        color: textColour
    }

    Text {
        id: authFailedText
        text: "Log in failed. Please try again."
        font.family: "Poppins"
        font.pixelSize: 16 * virtualstudio.fontScale * virtualstudio.uiScale
        anchors.horizontalCenter: parent.horizontalCenter
        y: 272 * virtualstudio.uiScale
        visible: failTextVisible
        color: textColour
    }

    Button {
        id: loginButton
        background: Rectangle {
            radius: 6 * virtualstudio.uiScale
            color: loginButton.down ? buttonPressedColour : (loginButton.hovered ? buttonHoverColour : buttonColour)
            border.width: loginButton.down ? 1 : 0
            border.color: buttonStroke
            layer.enabled: !loginButton.down
            layer.effect: DropShadow {
                horizontalOffset: 1 * virtualstudio.uiScale
                verticalOffset: 1 * virtualstudio.uiScale
                radius: 8.0 * virtualstudio.uiScale
                samples: 17
                color: shadowColour
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
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            color: loginButton.down ? buttonTextPressed : (loginButton.hovered ? buttonTextHover : buttonTextColour)
        }
        visible: !virtualstudio.hasRefreshToken
    }

    Button {
        id: backButton
        background: Rectangle {
            radius: 6 * virtualstudio.uiScale
            color: backButton.down ? buttonPressedColour : (backButton.hovered ? buttonHoverColour : buttonColour)
            border.width: backButton.down ? 1 : 0
            border.color: buttonStroke
            layer.enabled: !backButton.down
            layer.effect: DropShadow {
                horizontalOffset: 1 * virtualstudio.uiScale
                verticalOffset: 1 * virtualstudio.uiScale
                radius: 8.0 * virtualstudio.uiScale
                samples: 17
                color: shadowColour
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
            color: backButton.down ? buttonTextPressed : (backButton.hovered ? buttonTextHover : buttonTextColour)
        }
        visible: true
    }
}
