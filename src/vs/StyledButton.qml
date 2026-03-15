import QtQuick
import QtQuick.Controls

Button {
    id: styledButton
    property bool primary: false
    property int  fontSize: 11
    property int  buttonRadius: 6
    property bool showBorder: true

    readonly property color resolvedTextColor:
        !enabled    ? (virtualstudio.darkMode ? "#827D7D" : "#BABCBC")
        : primary   ? "#FFFFFF"
                    : (virtualstudio.darkMode ? "#FAFBFB" : "#0F0D0D")

    palette.buttonText: resolvedTextColor

    font {
        family: "Poppins"
        pixelSize: fontSize * virtualstudio.fontScale * virtualstudio.uiScale
        weight: Font.Normal
    }

    background: Rectangle {
        radius: buttonRadius * virtualstudio.uiScale
        color: "transparent"

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            visible: !styledButton.primary || !styledButton.enabled
            color: styledButton.down
                ? (virtualstudio.darkMode ? "#524F4F" : "#DEE0E0")
                : styledButton.hovered
                    ? (virtualstudio.darkMode ? "#5B5858" : "#D3D4D4")
                    : (virtualstudio.darkMode ? "#494646" : "#EAECEC")
            border.width: styledButton.showBorder ? 1 : 0
            border.color: styledButton.down || styledButton.hovered
                ? (virtualstudio.darkMode ? "#827D7D" : "#BABCBC")
                : (virtualstudio.darkMode ? "#80827D7D" : "#34979797")
        }

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            visible: styledButton.primary && styledButton.enabled
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: styledButton.down ? "#5B21B6" : styledButton.hovered ? "#6D28D9" : "#7C3AED" }
                GradientStop { position: 1.0; color: styledButton.down ? "#3730A3" : styledButton.hovered ? "#4338CA" : "#4F46E5" }
            }
        }
    }
}
