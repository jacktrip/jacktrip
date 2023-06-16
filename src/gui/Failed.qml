import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.12

Item {
    width: parent.width; height: parent.height
    clip: true

    property int leftMargin: 16

    property string buttonColour: virtualstudio.darkMode ? "#FAFBFB" : "#F0F1F1"
    property string buttonHoverColour: virtualstudio.darkMode ? "#E9E9E9" : "#E4E5E5"
    property string buttonPressedColour: virtualstudio.darkMode ? "#FAFBFB" : "#E4E5E5"
    property string buttonStroke: virtualstudio.darkMode ? "#9C9C9C" : "#A4A7A7"
    property string buttonTextColour: virtualstudio.darkMode ? "#272525" : "#DB0A0A"
    property string buttonTextHover: virtualstudio.darkMode ? "#242222" : "#D00A0A"
    property string buttonTextPressed: virtualstudio.darkMode ? "#323030" : "#D00A0A"


    Image {
        id: ohnoImage
        source: "ohno.png"
        width: 180
        height: 180
        y: 60
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Colorize {
        anchors.fill: ohnoImage
        source: ohnoImage
        hue: 0
        saturation: 0
        lightness: Styles.image.lightness
    }

    Text {
        id: ohnoHeader
        text: "Oh no!"
        font { family: "Poppins"; weight: Font.Bold; pixelSize: Styles.font.lg }
        color: Styles.text.color.standard
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: ohnoImage.bottom
        anchors.topMargin: 16 * virtualstudio.uiScale
    }

    Text {
        id: ohnoMessage
        text: virtualstudio.failedMessage || "Unable to process request - please try again later."
        font { family: "Poppins"; pixelSize: Styles.font.sm }
        color: Styles.text.color.standard
        width: 400
        wrapMode: Text.Wrap
        horizontalAlignment: Text.AlignHCenter
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: ohnoHeader.bottom
        anchors.topMargin: 32 * virtualstudio.uiScale
    }

    Button {
        id: backButton
        background: Rectangle {
            radius: 6 * virtualstudio.uiScale
            color: backButton.down ? buttonPressedColour : (backButton.hovered ? buttonHoverColour : buttonColour)
            border.width: backButton.down ? 1 : 0
            border.color: buttonStroke
            layer.enabled: !backButton.down
        }
        onClicked: { virtualstudio.windowState = "browse" }
        width: 256 * virtualstudio.uiScale
        height: 42 * virtualstudio.uiScale
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: ohnoMessage.bottom
        anchors.topMargin: 60 * virtualstudio.uiScale
        Text {
            text: "Back"
            font.family: "Poppins"
            font.pixelSize: Styles.font.lg
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            color: backButton.down ? buttonTextPressed : (backButton.hovered ? buttonTextHover : buttonTextColour)
        }
        visible: true
    }
}
