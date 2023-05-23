import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.12
import VS 1.0

Item {
    width: parent.width; height: parent.height
    clip: true

    state: auth.authenticationStage
    states: [
        State {
            name: "unauthenticated"
        },
        State {
            name: "polling"
        },
        State {
            name: "success"
        },
        State {
            name: "failed"
        }
    ]

    Rectangle {
        width: parent.width; height: parent.height
        color: backgroundColour
    }

    property bool codeCopied: false
    property bool hasFailedAtLeastOnce: false

    property string backgroundColour: virtualstudio.darkMode ? "#272525" : "#FAFBFB"
    property string textColour: virtualstudio.darkMode ? "#FAFBFB" : "#0F0D0D"
    property string buttonColour: virtualstudio.darkMode ? "#FAFBFB" : "#F0F1F1"
    property string buttonHoverColour: virtualstudio.darkMode ? "#E9E9E9" : "#E4E5E5"
    property string buttonPressedColour: virtualstudio.darkMode ? "#FAFBFB" : "#E4E5E5"
    property string buttonStroke: virtualstudio.darkMode ? "#80827D7D" : "#34979797"
    property string buttonHoverStroke: virtualstudio.darkMode ? "#6F6C6C" : "#B0B5B5"
    property string buttonPressedStroke: virtualstudio.darkMode ? "#6F6C6C" : "#B0B5B5"
    property string buttonTextColour: virtualstudio.darkMode ? "#272525" : "#DB0A0A"
    property string buttonTextHover: virtualstudio.darkMode ? "#242222" : "#D00A0A"
    property string buttonTextPressed: virtualstudio.darkMode ? "#323030" : "#D00A0A"
    property string shadowColour: virtualstudio.darkMode ? "40000000" : "#80A1A1A1"
    property string linkTextColour: virtualstudio.darkMode ? "#8B8D8D" : "#272525"
    property string toolTipTextColour: codeCopied ? "#FAFBFB" : textColour
    property string toolTipBackgroundColour: codeCopied ? "#57B147" : (virtualstudio.darkMode ? "#323232" : "#F3F3F3")
    property string tooltipStroke: virtualstudio.darkMode ? "#80827D7D" : "#34979797"
    property string disabledButtonText: "#D3D4D4"
    property string errorTextColour: "#DB0A0A"

    Clipboard {
        id: clipboard
    }

    Image {
        id: loginLogo
        source: "logo.svg"
        x: parent.width / 2 - (150 * virtualstudio.uiScale); y: 35 * virtualstudio.uiScale
        width: 42 * virtualstudio.uiScale; height: 76 * virtualstudio.uiScale
        sourceSize: Qt.size(loginLogo.width,loginLogo.height)
        fillMode: Image.PreserveAspectFit
        smooth: true
        visible: true
    }

    Image {
        source: virtualstudio.darkMode ? "jacktrip white.png" : "jacktrip.png"
        anchors.bottom: loginLogo.bottom
        x: parent.width / 2 - (88 * virtualstudio.uiScale)
        width: 238 * virtualstudio.uiScale; height: 56 * virtualstudio.uiScale
        visible: true
    }

    Text {
        text: "Virtual Studio"
        font.family: "Poppins"
        font.pixelSize: 28 * virtualstudio.fontScale * virtualstudio.uiScale
        anchors.horizontalCenter: parent.horizontalCenter
        y: 113 * virtualstudio.uiScale
        color: textColour
        visible: true
    }

    Text {
        id: authFailedText
        text: "There was an error trying to sign in. Please try again."
        font.family: "Poppins"
        font.pixelSize: 10 * virtualstudio.fontScale * virtualstudio.uiScale
        anchors.horizontalCenter: parent.horizontalCenter
        y: backButton.visible ? 547 * virtualstudio.uiScale : 500 * virtualstudio.uiScale
        visible: (loginScreen.state === "failed" || hasFailedAtLeastOnce) && loginScreen.state !== "success"
        color: errorTextColour
    }

    Image {
        id: successIcon
        source: "check.svg"
        y: 291 * virtualstudio.uiScale
        anchors.horizontalCenter: parent.horizontalCenter
        visible: loginScreen.state === "success"
        sourceSize: Qt.size(96 * virtualstudio.uiScale, 96 * virtualstudio.uiScale)
        fillMode: Image.PreserveAspectFit
        smooth: true
    }

    Colorize {
        anchors.fill: successIcon
        source: successIcon
        hue: .44
        saturation: .55
        lightness: .49
        visible: loginScreen.state === "success"
    }

    Text {
        id: deviceVerificationExplanation
        text: `Please sign in and confirm the following code using your web browser. Return here when you are done.`
        font.family: "Poppins"
        font.pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale
        anchors.horizontalCenter: parent.horizontalCenter
        y: 195 * virtualstudio.uiScale
        width: 500 * virtualstudio.uiScale;
        visible: true
        color: textColour
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter
        textFormat: Text.RichText
        onLinkActivated: link => {
            if (!Boolean(auth.verificationCode)) {
                return;
            }
            virtualstudio.openLink(link)
        }
    }

    Text {
        id: deviceVerificationCode
        text: auth.verificationCode || "Loading...";
        font.family: "Poppins"
        font.pixelSize: 20 * virtualstudio.fontScale * virtualstudio.uiScale
        font.letterSpacing: Boolean(auth.verificationCode) ? 8 : 1
        anchors.horizontalCenter: parent.horizontalCenter
        y: 267 * virtualstudio.uiScale
        width: 360 * virtualstudio.uiScale;
        visible: !auth.isAuthenticated
        color: Boolean(auth.verificationCode) ? textColour : disabledButtonText
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter

        Timer {
            id: copiedResetTimer
            interval: 2000; running: false; repeat: false
            onTriggered: codeCopied = false;
        }

        MouseArea {
            id: deviceVerificationCodeMouseArea
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            enabled: Boolean(auth.verificationCode)
            hoverEnabled: true
            onClicked: () => {
                codeCopied = true;
                clipboard.setText(auth.verificationCode);
                copiedResetTimer.restart()
            }
        }

        ToolTip {
            parent: deviceVerificationCode
            visible: loginScreen.state === "polling" && deviceVerificationCodeMouseArea.containsMouse
            delay: 100
            contentItem: Rectangle {
                color: toolTipBackgroundColour
                radius: 3
                anchors.fill: parent
                layer.enabled: true
                border.width: 1
                border.color: tooltipStroke

                Text {
                    anchors.centerIn: parent
                    font { family: "Poppins"; pixelSize: 8 * virtualstudio.fontScale * virtualstudio.uiScale}
                    text: codeCopied ? qsTr("📋 Copied code to clipboard") : qsTr("📋 Copy code to Clipboard")
                    color: toolTipTextColour
                }
            }
            background: Rectangle {
                color: "transparent"
            }
        }
    }

    Button {
        id: loginButton
        background: Rectangle {
            radius: 6 * virtualstudio.uiScale
            color: loginButton.down ? buttonPressedColour : (loginButton.hovered ? buttonHoverColour : buttonColour)
            border.width: 1
            border.color: loginButton.down ? buttonPressedStroke : (loginButton.hovered ? buttonHoverStroke : buttonStroke)
            layer.enabled: !loginButton.down
        }
        onClicked: {
            if (auth.verificationCode && auth.verificationUrl) {
                virtualstudio.showFirstRun = false;
                virtualstudio.openLink(auth.verificationUrl);
            }
        }
        anchors.horizontalCenter: parent.horizontalCenter
        y: 340 * virtualstudio.uiScale
        width: 263 * virtualstudio.uiScale; height: 64 * virtualstudio.uiScale
        Text {
            text: "Sign In"
            font.family: "Poppins"
            font.pixelSize: 18 * virtualstudio.fontScale * virtualstudio.uiScale
            font.weight: Font.Bold
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            color: loginButton.down ? buttonTextPressed : (loginButton.hovered ? buttonTextHover : buttonTextColour)
        }
        visible: !auth.isAuthenticated
    }

    Item {
        id: loginScreenFooter
        anchors.horizontalCenter: parent.horizontalCenter
        y: 500 * virtualstudio.uiScale
        height: 100 * virtualstudio.uiScale

        Button {
            id: backButton
            visible: !virtualstudio.vsFtux
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: backButton.down ? buttonPressedColour : (backButton.hovered ? buttonHoverColour : buttonColour)
                border.color: backButton.down ? buttonPressedStroke : (backButton.hovered ? buttonHoverStroke : buttonStroke)
                border.width: 1
                layer.enabled: !backButton.down
            }
            onClicked: () => { if (!auth.isAuthenticated) { virtualstudio.windowState = "start"; } }
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 8 * virtualstudio.uiScale
            width: 144 * virtualstudio.uiScale; height: 32 * virtualstudio.uiScale
            Text {
                text: "Back"
                font.family: "Poppins"
                font.pixelSize: 9 * virtualstudio.fontScale * virtualstudio.uiScale
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                color: backButton.down ? buttonTextPressed : (backButton.hovered ? buttonTextHover : textColour)
            }
        }

        Button {
            id: classicModeButton
            visible: virtualstudio.showFirstRun && virtualstudio.vsFtux
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: classicModeButton.down ? buttonPressedColour : (classicModeButton.hovered ? buttonHoverColour : buttonColour)
                border.color: classicModeButton.down ? buttonPressedStroke : (classicModeButton.hovered ? buttonHoverStroke : buttonStroke)
                border.width: 1
                layer.enabled: !classicModeButton.down
            }
            onClicked: { virtualstudio.windowState = "login"; virtualstudio.toStandard(); }
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 8 * virtualstudio.uiScale
            width: 160 * virtualstudio.uiScale; height: 32 * virtualstudio.uiScale
            Text {
                text: "Use Classic Mode"
                font.family: "Poppins"
                font.pixelSize: 9 * virtualstudio.fontScale * virtualstudio.uiScale
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                color: classicModeButton.down ? buttonTextPressed : (classicModeButton.hovered ? buttonTextHover : textColour)
            }
        }

        Button {
            id: resetCodeButton
            visible: true
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: resetCodeButton.down ? buttonPressedColour : (resetCodeButton.hovered ? buttonHoverColour : buttonColour)
                border.color: resetCodeButton.down ? buttonPressedStroke : (resetCodeButton.hovered ? buttonHoverStroke : buttonStroke)
                border.width: 1
                layer.enabled: !resetCodeButton.down
            }
            onClicked: () => { 
                if (auth.verificationCode && auth.verificationUrl) {
                    auth.resetCode();
                }
            }
            
            anchors.left: (backButton.visible || classicModeButton.visible) ? parent.horizontalCenter : undefined
            anchors.leftMargin: (backButton.visible || classicModeButton.visible) ? 8 * virtualstudio.uiScale : undefined
            anchors.horizontalCenter: (!backButton.visible && !classicModeButton.visible) ? parent.horizontalCenter : undefined
            anchors.verticalCenter: parent.verticalCenter
            width: 144 * virtualstudio.uiScale; height: 32 * virtualstudio.uiScale
            Text {
                text: "Reset Code"
                font.family: "Poppins"
                font.pixelSize: 9 * virtualstudio.fontScale * virtualstudio.uiScale
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                color: resetCodeButton.down ? buttonTextPressed : (resetCodeButton.hovered ? buttonTextHover : textColour)
            }
        }
    }

    Connections {
        target: auth
        function onUpdatedAuthenticationStage (stage) {
            loginScreen.state = stage;
            if (stage === "failed") {
                hasFailedAtLeastOnce = true;
            }
            if (stage === "success") {
                hasFailedAtLeastOnce = false; // reset
            }
        }
    }
}
