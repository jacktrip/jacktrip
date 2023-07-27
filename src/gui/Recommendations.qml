import QtQuick
import QtQuick.Controls

Item {
    width: parent.width; height: parent.height
    clip: true

    property int fontBig: 20
    property int fontMedium: 13
    property int fontSmall: 11
    property int fontExtraSmall: 8

    property int buttonWidth: 103
    property int buttonHeight: 25

    property int leftMargin: 48
    property int rightMargin: 16

    property string backgroundColour: virtualstudio.darkMode ? "#272525" : "#FAFBFB"
    property string textColour: virtualstudio.darkMode ? "#FAFBFB" : "#0F0D0D"
    property string buttonColour: virtualstudio.darkMode ? "#494646" : "#EAECEC"
    property string buttonHoverColour: virtualstudio.darkMode ? "#5B5858" : "#D3D4D4"
    property string buttonPressedColour: virtualstudio.darkMode ? "#524F4F" : "#DEE0E0"
    property string buttonStroke: virtualstudio.darkMode ? "#80827D7D" : "#34979797"
    property string buttonHoverStroke: virtualstudio.darkMode ? "#7B7777" : "#BABCBC"
    property string buttonPressedStroke: virtualstudio.darkMode ? "#827D7D" : "#BABCBC"
    property string saveButtonBackgroundColour: "#F2F3F3"
    property string saveButtonPressedColour: "#E7E8E8"
    property string saveButtonStroke: "#EAEBEB"
    property string saveButtonPressedStroke: "#B0B5B5"
    property string recommendationText: "#DB0A0A"
    property string saveButtonText: "#DB0A0A"
    property string checkboxStroke: "#0062cc"
    property string checkboxPressedStroke: "#007AFF"
    property string disabledButtonText: "#D3D4D4"
    property string linkText: virtualstudio.darkMode ? "#8B8D8D" : "#272525"

    property bool currShowRecommendations: virtualstudio.showWarnings
    property string recommendationScreen: virtualstudio.showWarnings ? "ethernet" : ( permissions.micPermission == "unknown" ? "microphone" : "acknowledged")

    Item {
        id: ethernetRecommendationItem
        width: parent.width; height: parent.height
        visible: recommendationScreen == "ethernet"

        AppIcon {
            id: ethernetRecommendationLogo
            y: 60
            anchors.horizontalCenter: parent.horizontalCenter
            width: 179
            height: 128
            icon.source: "ethernet.svg"
        }

        Text {
            id: ethernetRecommendationHeader
            text: "Connect via Wired Ethernet"
            font { family: "Poppins"; weight: Font.Bold; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: ethernetRecommendationLogo.bottom
            anchors.topMargin: 32 * virtualstudio.uiScale
        }

        Text {
            id: ethernetRecommendationSubheader1
            text: "JackTrip works best when you connect directly to your router via wired ethernet."
            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            width: 400
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: ethernetRecommendationHeader.bottom
            anchors.topMargin: 32 * virtualstudio.uiScale
        }

        Text {
            id: ethernetRecommendationSubheader2
            text: "WiFi works OK for some people, but you will experience higher latency and audio glitches."
            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            width: 400
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: ethernetRecommendationSubheader1.bottom
            anchors.topMargin: 24 * virtualstudio.uiScale
        }

        Button {
            id: okButtonEthernet
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: okButtonEthernet.down ? saveButtonPressedColour : saveButtonBackgroundColour
                border.width: 1
                border.color: okButtonEthernet.down || okButtonEthernet.hovered ? saveButtonPressedStroke : saveButtonStroke
                layer.enabled: okButtonEthernet.hovered && !okButtonEthernet.down
            }
            onClicked: { recommendationScreen = "headphones" }
            anchors.right: parent.right
            anchors.rightMargin: 16 * virtualstudio.uiScale
            anchors.bottomMargin: 16 * virtualstudio.uiScale
            anchors.bottom: parent.bottom
            width: 150 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
            Text {
                text: "Continue"
                font.family: "Poppins"
                font.pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale
                font.weight: Font.Bold
                color: saveButtonText
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        CheckBox {
            id: showEthernetRecommendationCheckbox
            checked: currShowRecommendations
            text: qsTr("Show recommendations again next time")
            anchors.right: okButtonEthernet.left
            anchors.rightMargin: 16 * virtualstudio.uiScale
            anchors.verticalCenter: okButtonEthernet.verticalCenter
            onClicked: { currShowRecommendations = showEthernetRecommendationCheckbox.checkState == Qt.Checked }
            indicator: Rectangle {
                implicitWidth: 16 * virtualstudio.uiScale
                implicitHeight: 16 * virtualstudio.uiScale
                x: showEthernetRecommendationCheckbox.leftPadding
                y: parent.height / 2 - height / 2
                radius: 3 * virtualstudio.uiScale
                border.color: showEthernetRecommendationCheckbox.down || showEthernetRecommendationCheckbox.hovered  ? checkboxPressedStroke : checkboxStroke

                Rectangle {
                    width: 10 * virtualstudio.uiScale
                    height: 10 * virtualstudio.uiScale
                    x: 3 * virtualstudio.uiScale
                    y: 3 * virtualstudio.uiScale
                    radius: 2 * virtualstudio.uiScale
                    color: showEthernetRecommendationCheckbox.down ||  showEthernetRecommendationCheckbox.hovered ? checkboxPressedStroke : checkboxStroke
                    visible: showEthernetRecommendationCheckbox.checked
                }
            }
            contentItem: Text {
                text: showEthernetRecommendationCheckbox.text
                font.family: "Poppins"
                font.pixelSize: 10 * virtualstudio.fontScale * virtualstudio.uiScale
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                leftPadding: showEthernetRecommendationCheckbox.indicator.width + showEthernetRecommendationCheckbox.spacing
                color: textColour
            }
        }
    }

    Item {
        id: headphoneRecommendationItem
        width: parent.width; height: parent.height
        visible: recommendationScreen == "headphones"

        AppIcon {
            id: headphoneRecommendationLogo
            y: 60
            anchors.horizontalCenter: parent.horizontalCenter
            width: 118
            height: 128
            icon.source: "headphones.svg"
        }

        Text {
            id: headphoneRecommendationHeader
            text: "Use Wired Headphones"
            font { family: "Poppins"; weight: Font.Bold; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: headphoneRecommendationLogo.bottom
            anchors.topMargin: 32 * virtualstudio.uiScale
        }

        Text {
            id: headphoneRecommendationSubheader1
            text: "JackTrip requires the use of wired headphones."
            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            width: 400
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: headphoneRecommendationHeader.bottom
            anchors.topMargin: 32 * virtualstudio.uiScale
        }

        Text {
            id: headphoneRecommendationSubheader2
            text: "Using speakers can cause loud feedback loops."
            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            width: 400
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: headphoneRecommendationSubheader1.bottom
            anchors.topMargin: 24 * virtualstudio.uiScale
        }

        Text {
            id: headphoneRecommendationSubheader3
            text: "Wireless headphones add way too much latency."
            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            width: 400
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: headphoneRecommendationSubheader2.bottom
            anchors.topMargin: 24 * virtualstudio.uiScale
        }

        Button {
            id: okButtonHeadphones
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: okButtonHeadphones.down ? saveButtonPressedColour : saveButtonBackgroundColour
                border.width: 1
                border.color: okButtonHeadphones.down || okButtonHeadphones.hovered ? saveButtonPressedStroke : saveButtonStroke
                layer.enabled: okButtonHeadphones.hovered && !okButtonHeadphones.down
            }
            onClicked: {
                if (permissions.micPermission === "granted") {
                    virtualstudio.windowState = "browse";
                } else {
                    virtualstudio.windowState = "permissions";
                }
            }
            anchors.right: parent.right
            anchors.rightMargin: 16 * virtualstudio.uiScale
            anchors.bottomMargin: 16 * virtualstudio.uiScale
            anchors.bottom: parent.bottom
            width: 150 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
            Text {
                text: "Continue"
                font.family: "Poppins"
                font.pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale
                font.weight: Font.Bold
                color: saveButtonText
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        CheckBox {
            id: showHeadphonesRecommendationCheckbox
            checked: currShowRecommendations
            text: qsTr("Show recommendations again next time")
            anchors.right: okButtonHeadphones.left
            anchors.rightMargin: 16 * virtualstudio.uiScale
            anchors.verticalCenter: okButtonHeadphones.verticalCenter
            onClicked: { currShowRecommendations = showHeadphonesRecommendationCheckbox.checkState == Qt.Checked }
            indicator: Rectangle {
                implicitWidth: 16 * virtualstudio.uiScale
                implicitHeight: 16 * virtualstudio.uiScale
                x: showHeadphonesRecommendationCheckbox.leftPadding
                y: parent.height / 2 - height / 2
                radius: 3 * virtualstudio.uiScale
                border.color: showHeadphonesRecommendationCheckbox.down || showHeadphonesRecommendationCheckbox.hovered ? checkboxPressedStroke : checkboxStroke

                Rectangle {
                    width: 10 * virtualstudio.uiScale
                    height: 10 * virtualstudio.uiScale
                    x: 3 * virtualstudio.uiScale
                    y: 3 * virtualstudio.uiScale
                    radius: 2 * virtualstudio.uiScale
                    color: showHeadphonesRecommendationCheckbox.down || showHeadphonesRecommendationCheckbox.hovered ? checkboxPressedStroke : checkboxStroke
                    visible: showHeadphonesRecommendationCheckbox.checked
                }
            }
            contentItem: Text {
                text: showHeadphonesRecommendationCheckbox.text
                font.family: "Poppins"
                font.pixelSize: 10 * virtualstudio.fontScale * virtualstudio.uiScale
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                leftPadding: showHeadphonesRecommendationCheckbox.indicator.width + showHeadphonesRecommendationCheckbox.spacing
                color: textColour
            }
        }
    }
}
