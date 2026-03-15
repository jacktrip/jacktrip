import QtQuick
import QtQuick.Controls
import QtWebEngine

Item {
    width: parent.width; height: parent.height
    clip: true

    Rectangle {
        width: parent.width; height: parent.height
        color: backgroundColour
    }

    property int buttonHeight: 25
    property int buttonWidth: 103
    property int extraSettingsButtonWidth: 16
    property int emptyListMessageWidth: 450
    property int fontBig: 28
    property int fontMedium: 11

    property int scrollY: 0

    property string backgroundColour: virtualstudio.darkMode ? "#272525" : "#FAFBFB"
    property string textColour: virtualstudio.darkMode ? "#FAFBFB" : "#0F0D0D"

    property string browseDiscoverUrl: `https://${virtualstudio.apiHost === "test.jacktrip.com" ? "next-test.jacktrip.com" : "www.jacktrip.com"}/discover`
    property string browseStudiosUrl: `https://${virtualstudio.apiHost === "test.jacktrip.com" ? "next-test.jacktrip.com" : "www.jacktrip.com"}/studios`

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
            id: aboutButton
            text: "About"
            icon { source: "question-mark-circle.svg"; color: resolvedTextColor }
            onClicked: { virtualstudio.showAbout() }
            display: AbstractButton.TextBesideIcon
            fontSize: fontMedium
            leftPadding: 0
            rightPadding: 4
            spacing: 0
            anchors.verticalCenter: parent.verticalCenter
            x: parent.width - ((230 + extraSettingsButtonWidth) * virtualstudio.uiScale)
            width: buttonWidth * virtualstudio.uiScale; height: buttonHeight * virtualstudio.uiScale
        }

        StyledButton {
            id: settingsButton
            text: "Settings"
            icon { source: "cog.svg"; color: resolvedTextColor }
            onClicked: { virtualstudio.windowState = "settings"; audio.startAudio(); }
            display: AbstractButton.TextBesideIcon
            fontSize: fontMedium
            leftPadding: 0
            rightPadding: 4
            spacing: 0
            anchors.verticalCenter: parent.verticalCenter
            x: parent.width - ((119 + extraSettingsButtonWidth) * virtualstudio.uiScale)
            width: (buttonWidth + extraSettingsButtonWidth) * virtualstudio.uiScale; height: buttonHeight * virtualstudio.uiScale
        }
    }

    AboutWindow {
    }

    FeedbackSurvey {
    }
}
