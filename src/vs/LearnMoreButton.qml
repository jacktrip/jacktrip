import QtQuick
import QtQuick.Controls

Button {
    property string url
    property string buttonText: "Learn more"

    width: 150 * virtualstudio.uiScale;
    height: 30 * virtualstudio.uiScale

    property string textColour: virtualstudio.darkMode ? "#FAFBFB" : "#0F0D0D"
    property string buttonColour: virtualstudio.darkMode ? "#494646" : "#EAECEC"
    property string buttonPressedStroke: virtualstudio.darkMode ? "#827D7D" : "#BABCBC"
    property string buttonPressedColour: virtualstudio.darkMode ? "#524F4F" : "#DEE0E0"
    property string buttonStroke: virtualstudio.darkMode ? "#80827D7D" : "#34979797"
    property string buttonHoverStroke: virtualstudio.darkMode ? "#7B7777" : "#BABCBC"
    property string buttonHoverColour: virtualstudio.darkMode ? "#5B5858" : "#D3D4D4"

    onClicked: {
        virtualstudio.openLink(url);
    }

    background: Rectangle {
        radius: 6 * virtualstudio.uiScale
        color: parent.down ? buttonPressedColour : (parent.hovered ? buttonHoverColour : buttonColour)
        border.width: 1
        border.color: parent.down ? buttonPressedStroke : (parent.hovered ? buttonHoverStroke : buttonStroke)
    }

    Text {
        text: buttonText
        font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
        color: textColour
        horizontalAlignment: Text.AlignHCenter
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
    }
}
