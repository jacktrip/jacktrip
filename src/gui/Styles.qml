pragma Singleton
import QtQuick 2.12

QtObject {
    property QtObject font: QtObject {
        property int xs: 8 * virtualstudio.fontScale * virtualstudio.uiScale
        property int sm: 11 * virtualstudio.fontScale * virtualstudio.uiScale
        property int md: 13 * virtualstudio.fontScale * virtualstudio.uiScale
        property int lg: 20 * virtualstudio.fontScale * virtualstudio.uiScale
        property int xl: 28 * virtualstudio.fontScale * virtualstudio.uiScale
    }

    property QtObject background: QtObject {
        property string defaultColor: virtualstudio.darkMode ? "#272525" : "#FAFBFB"
    }

    property QtObject text: QtObject {
        property QtObject color: QtObject {
            property string standard: virtualstudio.darkMode ? "#FAFBFB" : "#0F0D0D"
            property string warning: "#DB0A0A"
            property string link: virtualstudio.darkMode ? "#8B8D8D" : "#272525"
            property string disabled: virtualstudio.darkMode ? "#827D7D" : "#BABCBC"
        }
    }

    property QtObject button: QtObject {
        property string bgColor: virtualstudio.darkMode ? "#494646" : "#EAECEC"
        property string bgHoverColor: virtualstudio.darkMode ? "#5B5858" : "#D3D4D4"
        property string bgPressedColor: virtualstudio.darkMode ? "#524F4F" : "#DEE0E0"
        property string strokeColor: virtualstudio.darkMode ? "#494646" : "#EAECEC"
        property string strokeHoverColor: virtualstudio.darkMode ? "#7B7777" : "#BABCBC"
        property string strokePressedColor: virtualstudio.darkMode ? "#827D7D" : "#BABCBC"
    }

    property QtObject slider: QtObject {
        property string bgColor: virtualstudio.darkMode ? "#BABCBC" :  "#EAECEC"
        property string bgPressedColor: virtualstudio.darkMode ? "#ACAFAF" : "#DEE0E0"
        property string trackInactiveColor: virtualstudio.darkMode ? "#5B5858" : "light gray"
        property string trackActiveColor: virtualstudio.darkMode ? "light gray" : "black"
    }

    property QtObject image: QtObject {
        property string lightness: virtualstudio.darkMode ? 0.8 : 0.2
    }
}
