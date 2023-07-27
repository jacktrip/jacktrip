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
    property string warningText: "#DB0A0A"
    property string saveButtonText: "#DB0A0A"
    property string checkboxStroke: "#0062cc"
    property string checkboxPressedStroke: "#007AFF"
    property string disabledButtonText: "#D3D4D4"
    property string linkText: virtualstudio.darkMode ? "#8B8D8D" : "#272525"

    property bool currShowWarnings: virtualstudio.showWarnings
    property string warningScreen: virtualstudio.showWarnings ? "ethernet" : ( permissions.micPermission == "unknown" ? "microphone" : "acknowledged")

    Item {
        id: ethernetWarningItem
        width: parent.width; height: parent.height
        visible: warningScreen == "ethernet"

        AppIcon {
            id: ethernetWarningLogo
            y: 60
            anchors.horizontalCenter: parent.horizontalCenter
            width: 179
            height: 128
            icon.source: "ethernet.svg"
        }

        Text {
            id: ethernetWarningHeader
            text: "Connect via Wired Ethernet"
            font { family: "Poppins"; weight: Font.Bold; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: ethernetWarningLogo.bottom
            anchors.topMargin: 32 * virtualstudio.uiScale
        }

        Text {
            id: ethernetWarningSubheader1
            text: "JackTrip works best when you connect directly to your router via wired ethernet."
            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            width: 400
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: ethernetWarningHeader.bottom
            anchors.topMargin: 32 * virtualstudio.uiScale
        }

        Text {
            id: ethernetWarningSubheader2
            text: "WiFi works OK for some people, but you will experience higher latency and audio glitches."
            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            width: 400
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: ethernetWarningSubheader1.bottom
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
            onClicked: { warningScreen = "headphones" }
            anchors.right: parent.right
            anchors.rightMargin: 16 * virtualstudio.uiScale
            anchors.bottomMargin: 16 * virtualstudio.uiScale
            anchors.bottom: parent.bottom
            width: 150 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
            Text {
                text: "OK"
                font.family: "Poppins"
                font.pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale
                font.weight: Font.Bold
                color: saveButtonText
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        CheckBox {
            id: showEthernetWarningCheckbox
            checked: currShowWarnings
            text: qsTr("Show warnings again next time")
            anchors.right: okButtonEthernet.left
            anchors.rightMargin: 16 * virtualstudio.uiScale
            anchors.verticalCenter: okButtonEthernet.verticalCenter
            onClicked: { currShowWarnings = showEthernetWarningCheckbox.checkState == Qt.Checked }
            indicator: Rectangle {
                implicitWidth: 16 * virtualstudio.uiScale
                implicitHeight: 16 * virtualstudio.uiScale
                x: showEthernetWarningCheckbox.leftPadding
                y: parent.height / 2 - height / 2
                radius: 3 * virtualstudio.uiScale
                border.color: showEthernetWarningCheckbox.down || showEthernetWarningCheckbox.hovered  ? checkboxPressedStroke : checkboxStroke

                Rectangle {
                    width: 10 * virtualstudio.uiScale
                    height: 10 * virtualstudio.uiScale
                    x: 3 * virtualstudio.uiScale
                    y: 3 * virtualstudio.uiScale
                    radius: 2 * virtualstudio.uiScale
                    color: showEthernetWarningCheckbox.down ||  showEthernetWarningCheckbox.hovered ? checkboxPressedStroke : checkboxStroke
                    visible: showEthernetWarningCheckbox.checked
                }
            }
            contentItem: Text {
                text: showEthernetWarningCheckbox.text
                font.family: "Poppins"
                font.pixelSize: 10 * virtualstudio.fontScale * virtualstudio.uiScale
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                leftPadding: showEthernetWarningCheckbox.indicator.width + showEthernetWarningCheckbox.spacing
                color: textColour
            }
        }
    }

    Item {
        id: headphoneWarningItem
        width: parent.width; height: parent.height
        visible: warningScreen == "headphones"

        AppIcon {
            id: headphoneWarningLogo
            y: 60
            anchors.horizontalCenter: parent.horizontalCenter
            width: 118
            height: 128
            icon.source: "headphones.svg"
        }

        Text {
            id: headphoneWarningHeader
            text: "Use Wired Headphones"
            font { family: "Poppins"; weight: Font.Bold; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: headphoneWarningLogo.bottom
            anchors.topMargin: 32 * virtualstudio.uiScale
        }

        Text {
            id: headphoneWarningSubheader1
            text: "JackTrip requires the use of wired headphones."
            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            width: 400
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: headphoneWarningHeader.bottom
            anchors.topMargin: 32 * virtualstudio.uiScale
        }

        Text {
            id: headphoneWarningSubheader2
            text: "Using speakers can cause loud feedback loops."
            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            width: 400
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: headphoneWarningSubheader1.bottom
            anchors.topMargin: 24 * virtualstudio.uiScale
        }

        Text {
            id: headphoneWarningSubheader3
            text: "Wireless headphones add way too much latency."
            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            width: 400
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: headphoneWarningSubheader2.bottom
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
                text: "OK"
                font.family: "Poppins"
                font.pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale
                font.weight: Font.Bold
                color: saveButtonText
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        CheckBox {
            id: showHeadphonesWarningCheckbox
            checked: currShowWarnings
            text: qsTr("Show warnings again next time")
            anchors.right: okButtonHeadphones.left
            anchors.rightMargin: 16 * virtualstudio.uiScale
            anchors.verticalCenter: okButtonHeadphones.verticalCenter
            onClicked: { currShowWarnings = showHeadphonesWarningCheckbox.checkState == Qt.Checked }
            indicator: Rectangle {
                implicitWidth: 16 * virtualstudio.uiScale
                implicitHeight: 16 * virtualstudio.uiScale
                x: showHeadphonesWarningCheckbox.leftPadding
                y: parent.height / 2 - height / 2
                radius: 3 * virtualstudio.uiScale
                border.color: showHeadphonesWarningCheckbox.down || showHeadphonesWarningCheckbox.hovered ? checkboxPressedStroke : checkboxStroke

                Rectangle {
                    width: 10 * virtualstudio.uiScale
                    height: 10 * virtualstudio.uiScale
                    x: 3 * virtualstudio.uiScale
                    y: 3 * virtualstudio.uiScale
                    radius: 2 * virtualstudio.uiScale
                    color: showHeadphonesWarningCheckbox.down || showHeadphonesWarningCheckbox.hovered ? checkboxPressedStroke : checkboxStroke
                    visible: showHeadphonesWarningCheckbox.checked
                }
            }
            contentItem: Text {
                text: showHeadphonesWarningCheckbox.text
                font.family: "Poppins"
                font.pixelSize: 10 * virtualstudio.fontScale * virtualstudio.uiScale
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                leftPadding: showHeadphonesWarningCheckbox.indicator.width + showHeadphonesWarningCheckbox.spacing
                color: textColour
            }
        }
    }
}
