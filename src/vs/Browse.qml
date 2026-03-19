import QtQuick
import QtQuick.Controls
import QtWebEngine
import Qt5Compat.GraphicalEffects

Item {
    width: parent.width; height: parent.height
    clip: true

    Rectangle {
        width: parent.width; height: parent.height
        color: backgroundColour
    }

    property int buttonHeight: 25
    property int iconButtonSize: 32
    property int buttonWidth: 103
    property int extraSettingsButtonWidth: 16
    property int emptyListMessageWidth: 450
    property int fontBig: 28
    property int fontMedium: 11

    property int scrollY: 0

    property string backgroundColour: virtualstudio.darkMode ? "#272525" : "#FAFBFB"
    property string textColour: virtualstudio.darkMode ? "#FAFBFB" : "#0F0D0D"
    property string toolTipBackgroundColour: virtualstudio.darkMode ? "#323232" : "#F3F3F3"
    property int fontTiny: 8

    //property string browseBaseUrl: "http://localhost:3000"
    property string browseBaseUrl: virtualstudio.apiHost === "test.jacktrip.com" ? "https://next-test.jacktrip.com" : "https://www.jacktrip.com"
    property string browseDiscoverUrl: `${browseBaseUrl}/discover`
    property string browseStudiosUrl: `${browseBaseUrl}/studios`

    Loader {
        id: webLoader
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.bottom: footer.top
        sourceComponent: virtualstudio.windowState === "browse" && auth.isAuthenticated ? browseWeb : browseNull
    }

    Component {
        id: browseNull
        Rectangle {
            anchors.fill: parent
            color: backgroundColour
        }
    }

    Component {
        id: browseWeb
        WebEngineView {
            id: webEngineView
            anchors.fill: parent
            settings.fullScreenSupportEnabled: true
            settings.javascriptCanAccessClipboard: true
            settings.javascriptCanPaste: true
            settings.screenCaptureEnabled: true
            settings.playbackRequiresUserGesture: false
            url: browseStudiosUrl

            onContextMenuRequested: function(request) {
                // this disables the default context menu: https://doc.qt.io/qt-6.2/qml-qtwebengine-contextmenurequest.html#accepted-prop
                request.accepted = true;
            }

            onNewWindowRequested: function(request) {
                Qt.openUrlExternally(request.requestedUrl);
            }

            onFeaturePermissionRequested: function(securityOrigin, feature) {
                webEngineView.grantFeaturePermission(securityOrigin, feature, true);
            }

            onRenderProcessTerminated: function(terminationStatus, exitCode) {
                var status = "";
                switch (terminationStatus) {
                case WebEngineView.NormalTerminationStatus:
                    status = "(normal exit)";
                    break;
                case WebEngineView.AbnormalTerminationStatus:
                    status = "(abnormal exit)";
                    break;
                case WebEngineView.CrashedTerminationStatus:
                    status = "(crashed)";
                    break;
                case WebEngineView.KilledTerminationStatus:
                    status = "(killed)";
                    break;
                }
                console.log("Render process exited with code " + exitCode + " " + status);
            }
        }
    }

    Rectangle {
        id: footer
        x: 0; y: parent.height - 36 * virtualstudio.uiScale; width: parent.width; height: 36 * virtualstudio.uiScale
        border.color: "#33979797"
        color: backgroundColour

        StyledButton {
            id: studiosButton
            text: "My Studios"
            icon { source: "squares-2x2.svg"; color: resolvedTextColor }
            onClicked: { if (webLoader.item) webLoader.item.url = browseStudiosUrl }
            enabled: !(webLoader.item && webLoader.item.url && webLoader.item.url.toString().startsWith(browseStudiosUrl))
            display: AbstractButton.TextBesideIcon
            fontSize: fontMedium
            leftPadding: 0
            rightPadding: 4
            spacing: 0
            anchors.verticalCenter: parent.verticalCenter
            x: 16 * virtualstudio.uiScale
            width: (buttonWidth + extraSettingsButtonWidth) * virtualstudio.uiScale; height: buttonHeight * virtualstudio.uiScale
        }

        StyledButton {
            id: discoverButton
            text: "Discover"
            icon { source: "public.svg"; color: resolvedTextColor }
            onClicked: { if (webLoader.item) webLoader.item.url = browseDiscoverUrl }
            enabled: !(webLoader.item && webLoader.item.url && webLoader.item.url.toString().startsWith(browseDiscoverUrl))
            display: AbstractButton.TextBesideIcon
            fontSize: fontMedium
            leftPadding: 0
            rightPadding: 4
            spacing: 0
            anchors.verticalCenter: parent.verticalCenter
            x: (16 + buttonWidth + extraSettingsButtonWidth + 8) * virtualstudio.uiScale
            width: (buttonWidth + extraSettingsButtonWidth) * virtualstudio.uiScale; height: buttonHeight * virtualstudio.uiScale
        }

        StyledButton {
            id: goBackButton
            icon { source: "arrow-left.svg"; color: resolvedTextColor; width: iconButtonSize * virtualstudio.uiScale; height: iconButtonSize * virtualstudio.uiScale }
            onClicked: { if (webLoader.item) webLoader.item.goBack() }
            enabled: !!(webLoader.item && webLoader.item.canGoBack)
            display: AbstractButton.IconOnly
            showBorder: false
            anchors.verticalCenter: parent.verticalCenter
            x: parent.width - (16 + iconButtonSize * 6 + 8 * 5) * virtualstudio.uiScale
            width: iconButtonSize * virtualstudio.uiScale; height: buttonHeight * virtualstudio.uiScale

            ToolTip {
                visible: goBackButton.hovered
                delay: 500
                contentItem: Text {
                    text: qsTr("Go Back")
                    font { family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
                    color: textColour
                }
                background: Rectangle {
                    color: toolTipBackgroundColour
                    radius: 4
                    layer.enabled: true
                    layer.effect: Glow {
                        radius: 8
                        color: "#66000000"
                        transparentBorder: true
                    }
                }
            }
        }

        StyledButton {
            id: goForwardButton
            icon { source: "arrow-right.svg"; color: resolvedTextColor; width: iconButtonSize * virtualstudio.uiScale; height: iconButtonSize * virtualstudio.uiScale }
            onClicked: { if (webLoader.item) webLoader.item.goForward() }
            enabled: !!(webLoader.item && webLoader.item.canGoForward)
            display: AbstractButton.IconOnly
            showBorder: false
            anchors.verticalCenter: parent.verticalCenter
            x: parent.width - (16 + iconButtonSize * 5 + 8 * 4) * virtualstudio.uiScale
            width: iconButtonSize * virtualstudio.uiScale; height: buttonHeight * virtualstudio.uiScale

            ToolTip {
                visible: goForwardButton.hovered
                delay: 500
                contentItem: Text {
                    text: qsTr("Go Forward")
                    font { family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
                    color: textColour
                }
                background: Rectangle {
                    color: toolTipBackgroundColour
                    radius: 4
                    layer.enabled: true
                    layer.effect: Glow {
                        radius: 8
                        color: "#66000000"
                        transparentBorder: true
                    }
                }
            }
        }

        StyledButton {
            id: refreshButton
            icon { source: "refresh.svg"; color: resolvedTextColor; width: iconButtonSize * virtualstudio.uiScale; height: iconButtonSize * virtualstudio.uiScale }
            onClicked: { if (webLoader.item) webLoader.item.reload() }
            display: AbstractButton.IconOnly
            showBorder: false
            anchors.verticalCenter: parent.verticalCenter
            x: parent.width - (16 + iconButtonSize * 4 + 8 * 3) * virtualstudio.uiScale
            width: iconButtonSize * virtualstudio.uiScale; height: buttonHeight * virtualstudio.uiScale

            ToolTip {
                visible: refreshButton.hovered
                delay: 500
                contentItem: Text {
                    text: qsTr("Refresh")
                    font { family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
                    color: textColour
                }
                background: Rectangle {
                    color: toolTipBackgroundColour
                    radius: 4
                    layer.enabled: true
                    layer.effect: Glow {
                        radius: 8
                        color: "#66000000"
                        transparentBorder: true
                    }
                }
            }
        }

        StyledButton {
            id: openInBrowserButton
            icon { source: "arrow-top-right-on-square.svg"; color: resolvedTextColor; width: iconButtonSize * virtualstudio.uiScale; height: iconButtonSize * virtualstudio.uiScale }
            onClicked: { if (webLoader.item) Qt.openUrlExternally(webLoader.item.url) }
            display: AbstractButton.IconOnly
            showBorder: false
            anchors.verticalCenter: parent.verticalCenter
            x: parent.width - (16 + iconButtonSize * 3 + 8 * 2) * virtualstudio.uiScale
            width: iconButtonSize * virtualstudio.uiScale; height: buttonHeight * virtualstudio.uiScale

            ToolTip {
                visible: openInBrowserButton.hovered
                delay: 500
                contentItem: Text {
                    text: qsTr("Open in Browser")
                    font { family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
                    color: textColour
                }
                background: Rectangle {
                    color: toolTipBackgroundColour
                    radius: 4
                    layer.enabled: true
                    layer.effect: Glow {
                        radius: 8
                        color: "#66000000"
                        transparentBorder: true
                    }
                }
            }
        }

        StyledButton {
            id: aboutButton
            icon { source: "question-mark-circle.svg"; color: resolvedTextColor; width: iconButtonSize * virtualstudio.uiScale; height: iconButtonSize * virtualstudio.uiScale }
            onClicked: { virtualstudio.showAbout() }
            display: AbstractButton.IconOnly
            showBorder: false
            anchors.verticalCenter: parent.verticalCenter
            x: parent.width - (16 + iconButtonSize * 2 + 8) * virtualstudio.uiScale
            width: iconButtonSize * virtualstudio.uiScale; height: buttonHeight * virtualstudio.uiScale

            ToolTip {
                visible: aboutButton.hovered
                delay: 500
                contentItem: Text {
                    text: qsTr("About")
                    font { family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
                    color: textColour
                }
                background: Rectangle {
                    color: toolTipBackgroundColour
                    radius: 4
                    layer.enabled: true
                    layer.effect: Glow {
                        radius: 8
                        color: "#66000000"
                        transparentBorder: true
                    }
                }
            }
        }

        StyledButton {
            id: settingsButton
            icon { source: "cog.svg"; color: resolvedTextColor; width: iconButtonSize * virtualstudio.uiScale; height: iconButtonSize * virtualstudio.uiScale }
            onClicked: { virtualstudio.windowState = "settings"; audio.startAudio(); }
            display: AbstractButton.IconOnly
            showBorder: false
            anchors.verticalCenter: parent.verticalCenter
            x: parent.width - (16 + iconButtonSize) * virtualstudio.uiScale
            width: iconButtonSize * virtualstudio.uiScale; height: buttonHeight * virtualstudio.uiScale

            ToolTip {
                visible: settingsButton.hovered
                delay: 500
                contentItem: Text {
                    text: qsTr("Settings")
                    font { family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
                    color: textColour
                }
                background: Rectangle {
                    color: toolTipBackgroundColour
                    radius: 4
                    layer.enabled: true
                    layer.effect: Glow {
                        radius: 8
                        color: "#66000000"
                        transparentBorder: true
                    }
                }
            }
        }
    }

    AboutWindow {
    }

    FeedbackSurvey {
    }
}
