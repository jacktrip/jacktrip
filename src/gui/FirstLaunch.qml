import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    width: parent.width; height: parent.height
    clip: true
    
    property string textColour: virtualstudio.darkMode ? "#FAFBFB" : "#0F0D0D"
    property string shadowColour: virtualstudio.darkMode ? "40000000" : "#80A1A1A1"
    property string buttonColour: virtualstudio.darkMode ? "#FAFBFB" : "#F0F1F1"
    property string buttonHoverColour: virtualstudio.darkMode ? "#E9E9E9" : "#E4E5E5"
    property string buttonPressedColour: virtualstudio.darkMode ? "#FAFBFB" : "#E4E5E5"
    property string buttonStroke: virtualstudio.darkMode ? "#636060" : "#DEDFDF"
    property string buttonHoverStroke: virtualstudio.darkMode ? "#6F6C6C" : "#B0B5B5"
    property string buttonPressedStroke: virtualstudio.darkMode ? "#6F6C6C" : "#B0B5B5"
    
    Image {
        id: jtlogo
        source: "logo.svg"
        anchors.horizontalCenter: parent.horizontalCenter
        y: 35 * virtualstudio.uiScale
        width: 50 * virtualstudio.uiScale; height: 92 * virtualstudio.uiScale
        sourceSize: Qt.size(jtlogo.width,jtlogo.height)
        fillMode: Image.PreserveAspectFit
        smooth: true
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 168 * virtualstudio.uiScale
        text: "Sign in with a Virtual Studio account?"
        font.family: "Poppins"
        font.pixelSize: 17 * virtualstudio.fontScale * virtualstudio.uiScale
        color: textColour
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 219 * virtualstudio.uiScale
        text: "You'll be able to change your mind later"
        font.family: "Poppins"
        font.pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale
        color: textColour
    }

    Button {
        id: vsButton
        background: Rectangle {
            radius: 10 * virtualstudio.uiScale
            color: vsButton.down ? buttonPressedColour : (vsButton.hovered ? buttonHoverColour : buttonColour)
            border.width: 1
            border.color: vsButton.down ? buttonPressedStroke : (vsButton.hovered ? buttonHoverStroke : buttonStroke)
            layer.enabled: vsButton.hovered && !vsButton.down
        }
        onClicked: { virtualstudio.showFirstRun = false; virtualstudio.windowState = "login"; virtualstudio.toVirtualStudio(); }
        x: parent.width / 2 - (265 * virtualstudio.uiScale); y: 290 * virtualstudio.uiScale
        width: 234 * virtualstudio.uiScale; height: 49 * virtualstudio.uiScale
        Text {
            text: "Yes"
            font.family: "Poppins"
            font.pixelSize: 18 * virtualstudio.fontScale * virtualstudio.uiScale
            font.weight: Font.Bold
            color: "#DB0A0A"
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
        }
    }
    Text {
        text: "• Connect to Virtual Studios<br>• Broadcast on JackTrip Radio<br>• Apply FX with Soundscapes"
        textFormat: Text.StyledText
        font.family: "Poppins"
        font.pixelSize: 10 * virtualstudio.fontScale * virtualstudio.uiScale
        x: parent.width / 2 - (265 * virtualstudio.uiScale);
        y: 355 * virtualstudio.uiScale;
        width: 230 * virtualstudio.uiScale
        padding: 0
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter
        color: textColour
    }
    Image {
        source: "JTVS.png"
        x: parent.width / 2 - (265 * virtualstudio.uiScale); y: 420 * virtualstudio.uiScale
        width: 234 * virtualstudio.uiScale; height: 201.48 * virtualstudio.uiScale;
    }

    Button {
        id: standardButton
        background: Rectangle {
            radius: 10 * virtualstudio.uiScale
            color: standardButton.down ? buttonPressedColour : (standardButton.hovered ? buttonHoverColour : buttonColour)
            border.width: 1
            border.color: standardButton.down ? buttonPressedStroke : (standardButton.hovered ? buttonHoverStroke : buttonStroke)
            layer.enabled: standardButton.hovered && !standardButton.down
        }
        onClicked: { virtualstudio.windowState = "login"; virtualstudio.toStandard(); }
        x: parent.width / 2 + (32 * virtualstudio.uiScale); y: 290 * virtualstudio.uiScale
        width: 234 * virtualstudio.uiScale; height: 49 * virtualstudio.uiScale
        Text {
            text: "No"
            font.family: "Poppins"
            font.pixelSize: 18 * virtualstudio.fontScale * virtualstudio.uiScale
            font.weight: Font.Bold
            color: "#DB0A0A"
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
        }
    }
    Image {
        source: "JTOriginal.png"
        x: parent.width / 2 + (32 * virtualstudio.uiScale); y: 420 * virtualstudio.uiScale
        width: 234 * virtualstudio.uiScale; height: 337.37 * virtualstudio.uiScale;
    }
    Text {
        text: virtualstudio.psiBuild ? "• Connect via IP address<br>• Run a local hub server<br>• The Standard JackTrip experience" :
              "• Connect via IP address<br>• Run a local hub server<br>• The Classic JackTrip experience"
        textFormat: Text.StyledText
        font.family: "Poppins"
        font.pixelSize: 10 * virtualstudio.fontScale * virtualstudio.uiScale
        x: parent.width / 2 + (32 * virtualstudio.uiScale);
        y: 355 * virtualstudio.uiScale;
        width: 230 * virtualstudio.uiScale
        padding: 0
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter
        color: textColour
    }
}
