import QtQuick
import QtQuick.Controls

Button {
    id: refreshButton
    text: "Refresh Devices"

    property int fontExtraSmall: 8
    property string textColour: virtualstudio.darkMode ? "#FAFBFB" : "#0F0D0D"
    property string buttonColour: virtualstudio.darkMode ? "#494646" : "#EAECEC"
    property string buttonHoverColour: virtualstudio.darkMode ? "#5B5858" : "#D3D4D4"
    property string buttonPressedColour: virtualstudio.darkMode ? "#524F4F" : "#DEE0E0"
    property string buttonStroke: virtualstudio.darkMode ? "#80827D7D" : "#34979797"
    property string buttonHoverStroke: virtualstudio.darkMode ? "#7B7777" : "#BABCBC"
    property string buttonPressedStroke: virtualstudio.darkMode ? "#827D7D" : "#BABCBC"
    property var onDeviceRefresh: function () { audio.refreshDevices(); };

    width: 144 * virtualstudio.uiScale;
    height: 30 * virtualstudio.uiScale
    palette.buttonText: textColour
    display: AbstractButton.TextBesideIcon

    background: Rectangle {
        radius: 6 * virtualstudio.uiScale
        color: refreshButton.down ? buttonPressedColour : (refreshButton.hovered ? buttonHoverColour : buttonColour)
        border.width: 1
        border.color: refreshButton.down ? buttonPressedStroke : (refreshButton.hovered ? buttonHoverStroke : buttonStroke)
    }
    icon {
        source: "refresh.svg";
        color: textColour;
    }
    onClicked: { onDeviceRefresh(); }
    font {
        family: "Poppins"
        pixelSize: fontExtraSmall * virtualstudio.fontScale * virtualstudio.uiScale
    }
}
