import QtQuick
import QtQuick.Controls

Button {
    property string url
    property string buttonText: "Learn more"

    width: 150 * virtualstudio.uiScale;
    height: 30 * virtualstudio.uiScale

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
