import QtQuick
import QtQuick.Controls

Window {
    id: aboutWindow
    title: "About JackTrip"
    visible: false

    width: 600 * virtualstudio.uiScale
    height: 570 * virtualstudio.uiScale

    property int fontTitle: 20
    property int fontMedium: 12
    property int fontSmall: 10
    property int fontTiny: 8

    property string buttonColour: "#F2F3F3"
    property string buttonHoverColour: "#E7E8E8"
    property string buttonPressedColour: "#E7E8E8"
    property string buttonStroke: "#EAEBEB"
    property string buttonHoverStroke: "#B0B5B5"
    property string buttonPressedStroke: "#B0B5B5"

    property string textColour: virtualstudio.darkMode ? "#FAFBFB" : "#0F0D0D"
    property string textAreaTextColour: virtualstudio.darkMode ? "#A6A6A6" : "#757575"
    property string textAreaColour: virtualstudio.darkMode ? "#494646" : "#EAECEC"

    Rectangle {
        width: parent.width
        height: parent.height
        color: backgroundColour

        Rectangle {
            id: aboutJackTripLogo
            x: 0; y: 0;
            width: 122 * virtualstudio.uiScale
            height: 108 * virtualstudio.uiScale
            color: backgroundColour
            Image {
                id: aboutLogoImage
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                source: "logo.svg"
                width: 42 * virtualstudio.uiScale; height: 76 * virtualstudio.uiScale
                sourceSize: Qt.size(aboutLogoImage.width,aboutLogoImage.height)
                fillMode: Image.PreserveAspectFit
                smooth: true
            }
        }

        Item {
            id: aboutContent
            x: 122 * virtualstudio.uiScale;
            y: 0;
            width: parent.width - aboutJackTripLogo.width - (32 * virtualstudio.uiScale)
            height: parent.height

            Text {
                id: aboutHeader
                anchors.top: parent.top
                anchors.topMargin: 16 * virtualstudio.uiScale
                anchors.left: parent.left
                width: parent.width
                text: "JackTrip Desktop App"
                font {family: "Poppins"; pixelSize: fontTitle * virtualstudio.fontScale * virtualstudio.uiScale; bold: true }
                color: textColour
                elide: Text.ElideRight
                wrapMode: Text.WordWrap
            }

            Text {
                id: aboutVersion
                anchors.top: aboutHeader.bottom
                anchors.topMargin: 16 * virtualstudio.uiScale
                anchors.left: parent.left
                width: parent.width
                text: "Version " + virtualstudio.versionString
                font {family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale; bold: true }
                color: textColour
                elide: Text.ElideRight
                wrapMode: Text.WordWrap
            }

            Text {
                id: aboutBuildInfo
                anchors.top: aboutVersion.bottom
                anchors.topMargin: 16 * virtualstudio.uiScale
                anchors.left: parent.left
                width: parent.width
                text: virtualstudio.buildString
                font {family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale; }
                color: textColour
                elide: Text.ElideRight
                wrapMode: Text.WordWrap
            }

            Text {
                id: aboutCopyright
                anchors.top: aboutBuildInfo.bottom
                anchors.topMargin: 16 * virtualstudio.uiScale
                anchors.left: parent.left
                width: parent.width
                text: virtualstudio.copyrightString
                font {family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale; }
                color: textColour
                elide: Text.ElideRight
                wrapMode: Text.WordWrap
            }

            Button {
                id: aboutCloseButton
                anchors.top: aboutCopyright.bottom
                anchors.topMargin: 16 * virtualstudio.uiScale
                anchors.left: parent.left
                width: 150 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
                onClicked: () => {
                    aboutWindow.visible = false;
                }

                background: Rectangle {
                    radius: 6 * virtualstudio.uiScale
                    color: aboutCloseButton.down ? buttonPressedColour : (aboutCloseButton.hovered ? buttonHoverColour : buttonColour)
                    border.width: 1
                    border.color: aboutCloseButton.down ? buttonPressedStroke : (aboutCloseButton.hovered ? buttonHoverStroke : buttonStroke)
                }

                Text {
                    text: "Close"
                    font.family: "Poppins"
                    font.pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }
    }

    Connections {
      target: virtualstudio

      function onOpenAboutWindow() {
        aboutWindow.visible = true;
      }
    }
}
