import QtQuick
import QtQuick.Controls
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
            name: "refreshing"
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
    property int numFailures: 0;

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

    property bool showCodeFlow: (loginScreen.state === "unauthenticated" && !auth.attemptingRefreshToken) || (loginScreen.state === "polling" || loginScreen.state === "failed" || (loginScreen.state === "success" && auth.authenticationMethod === "code flow"))
    property bool showLoading: (loginScreen.state === "unauthenticated" ** auth.attemptingRefreshToken) || loginScreen.state === "refreshing" || (loginScreen.state === "success" && auth.authenticationMethod === "refresh token")

    Clipboard {
        id: clipboard
    }

    Item {
        id: loginScreenHeader
        anchors.horizontalCenter: parent.horizontalCenter
        y: showCodeFlow ? 48 * virtualstudio.uiScale : 144 * virtualstudio.uiScale

        Image {
            id: loginLogo
            source: "logo.svg"
            x: parent.width / 2 - (150 * virtualstudio.uiScale);
            width: 42 * virtualstudio.uiScale; height: 76 * virtualstudio.uiScale
            sourceSize: Qt.size(loginLogo.width,loginLogo.height)
            fillMode: Image.PreserveAspectFit
            smooth: true
        }

        Image {
            source: virtualstudio.darkMode ? "jacktrip white.png" : "jacktrip.png"
            anchors.bottom: loginLogo.bottom
            x: parent.width / 2 - (88 * virtualstudio.uiScale)
            width: 238 * virtualstudio.uiScale; height: 56 * virtualstudio.uiScale
        }

        Text {
            text: "Virtual Studio"
            font.family: "Poppins"
            font.pixelSize: 24 * virtualstudio.fontScale * virtualstudio.uiScale
            anchors.horizontalCenter: parent.horizontalCenter
            y: 80 * virtualstudio.uiScale
            color: textColour
        }
    }

    Item {
        id: codeFlow
        anchors.horizontalCenter: parent.horizontalCenter
        y: 68 * virtualstudio.uiScale
        height: parent.height - codeFlow.y
        visible: showCodeFlow
        width: parent.width

        Text {
            id: deviceVerificationExplanation
            text: `Please sign in and confirm the following code using your web browser. Return here when you are done.`
            font.family: "Poppins"
            font.pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale
            anchors.horizontalCenter: parent.horizontalCenter
            y: 128 * virtualstudio.uiScale
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

        AppIcon {
            id: successIcon
            y: 224 * virtualstudio.uiScale
            anchors.horizontalCenter: parent.horizontalCenter
            width: 96 * virtualstudio.uiScale
            height: 96 * virtualstudio.uiScale
            icon.source: "check.svg"
            color: "green"
            visible: loginScreen.state === "success"
        }

        Text {
            id: deviceVerificationCode
            text: auth.verificationCode || ((numFailures >= 5) ? "Error" : "Loading...");
            font.family: "Poppins"
            font.pixelSize: 20 * virtualstudio.fontScale * virtualstudio.uiScale
            font.letterSpacing: Boolean(auth.verificationCode) ? 8 : 1
            anchors.horizontalCenter: parent.horizontalCenter
            y: 196 * virtualstudio.uiScale
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
                    virtualstudio.openLink(auth.verificationUrl);
                }
            }
            anchors.horizontalCenter: parent.horizontalCenter
            y: 260 * virtualstudio.uiScale
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

        Text {
            id: authFailedText
            text: "There was an error trying to sign in. Please try again."
            font.family: "Poppins"
            font.pixelSize: 10 * virtualstudio.fontScale * virtualstudio.uiScale
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: loginScreenFooter.top
            anchors.bottomMargin: 16 * virtualstudio.uiScale
            visible: (loginScreen.state === "failed" || numFailures > 0) && loginScreen.state !== "success"
            color: errorTextColour
        }

        Item {
            id: loginScreenFooter
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 24 * virtualstudio.uiScale
            width: parent.width
            height: 48 * virtualstudio.uiScale

            property bool showBackButton: !virtualstudio.vsFtux
            property bool showClassicModeButton: virtualstudio.vsFtux

            Item {
                id: backButton
                visible: parent.showBackButton
                anchors.verticalCenter: parent.verticalCenter
                x: (parent.x + parent.width / 2) - backButton.width - 8 * virtualstudio.uiScale
                width: 144 * virtualstudio.uiScale; height: 32 * virtualstudio.uiScale
                Text {
                    text: "Back"
                    font.family: "Poppins"
                    font.underline: true
                    font.pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    color: textColour
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: () => { if (!auth.isAuthenticated) { virtualstudio.windowState = "start"; } }
                    cursorShape: Qt.PointingHandCursor
                }
            }

            Item {
                id: classicModeButton
                visible: parent.showClassicModeButton
                anchors.verticalCenter: parent.verticalCenter
                x: (parent.x + parent.width / 2) - classicModeButton.width - 8 * virtualstudio.uiScale
                width: 160 * virtualstudio.uiScale; height: 32 * virtualstudio.uiScale
                Text {
                    text: "Use Classic Mode"
                    font.underline: true
                    font.family: "Poppins"
                    font.pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    color: textColour
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: () => { virtualstudio.windowState = "login"; virtualstudio.toStandard(); }
                    cursorShape: Qt.PointingHandCursor
                }
            }

            Item {
                id: resetCodeButton
                visible: true
                x: (parent.showBackButton || parent.showClassicModeButton) ? (parent.x + parent.width / 2) + 8 * virtualstudio.uiScale : (parent.x + parent.width / 2) - resetCodeButton.width / 2
                anchors.verticalCenter: parent.verticalCenter
                width: 144 * virtualstudio.uiScale; height: 32 * virtualstudio.uiScale
                Text {
                    text: "Reset Code"
                    font.family: "Poppins"
                    font.underline: true
                    font.pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    color: textColour
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: () => {
                        if (auth.verificationCode && auth.verificationUrl) {
                            auth.resetCode();
                        }
                    }
                    cursorShape: Qt.PointingHandCursor
                }
            }
        }
    }

    Item {
        id: refreshToken
        anchors.horizontalCenter: parent.horizontalCenter
        y: 108 * virtualstudio.uiScale
        visible: showLoading

        Text {
            id: loadingViaRefreshToken
            text: "Logging In...";
            font.family: "Poppins"
            font.pixelSize: 20 * virtualstudio.fontScale * virtualstudio.uiScale
            anchors.horizontalCenter: parent.horizontalCenter
            y: 208 * virtualstudio.uiScale
            width: 360 * virtualstudio.uiScale;
            color: textColour
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
        }
    }

    Connections {
        target: auth
        function onUpdatedAuthenticationStage (stage) {
            loginScreen.state = stage;
            if (stage === "failed") {
                numFailures = numFailures + 1;
                if (numFailures < 5 && !virtualstudio.hasRefreshToken) {
                    virtualstudio.login();
                }
            }
            if (stage === "success") {
                numFailures = 0;
            }
        }
    }
}
