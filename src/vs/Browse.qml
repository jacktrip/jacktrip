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
    property string buttonColour: virtualstudio.darkMode ? "#494646" : "#EAECEC"
    property string buttonHoverColour: virtualstudio.darkMode ? "#5B5858" : "#D3D4D4"
    property string buttonPressedColour: virtualstudio.darkMode ? "#524F4F" : "#DEE0E0"
    property string buttonStroke: virtualstudio.darkMode ? "#80827D7D" : "#40979797"
    property string buttonHoverStroke: virtualstudio.darkMode ? "#7B7777" : "#BABCBC"
    property string buttonPressedStroke: virtualstudio.darkMode ? "#827D7D" : "#BABCBC"

    function refresh() {
        virtualstudio.refreshStudios(currentIndex, true)
    }

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
            url: `https://${virtualstudio.apiHost === "test.jacktrip.com" ? "next-test.jacktrip.com" : "www.jacktrip.com"}`

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

        Button {
            id: refreshButton
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: refreshButton.down ? buttonPressedColour : (refreshButton.hovered ? buttonHoverColour : buttonColour)
                border.width: 1
                border.color: refreshButton.down ? buttonPressedStroke : (refreshButton.hovered ? buttonHoverStroke : buttonStroke)
            }
            onClicked: { refresh() }
            anchors.verticalCenter: parent.verticalCenter
            x: 16 * virtualstudio.uiScale
            width: buttonWidth * virtualstudio.uiScale; height: buttonHeight * virtualstudio.uiScale
            Text {
                text: "Refresh List"
                font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
                anchors {horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
                color: textColour
            }
        }

        Button {
            id: aboutButton
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: aboutButton.down ? buttonPressedColour : (aboutButton.hovered ? buttonHoverColour : buttonColour)
                border.width: 1
                border.color: aboutButton.down ? buttonPressedStroke : (aboutButton.hovered ? buttonHoverStroke : buttonStroke)
            }
            onClicked: { virtualstudio.showAbout() }
            anchors.verticalCenter: parent.verticalCenter
            x: parent.width - ((230 + extraSettingsButtonWidth) * virtualstudio.uiScale)
            width: buttonWidth * virtualstudio.uiScale; height: buttonHeight * virtualstudio.uiScale
            Text {
                text: "About"
                font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
                anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
                color: textColour
            }
        }

        Button {
            id: settingsButton
            text: "Settings"
            palette.buttonText: textColour
            icon {
                source: "cog.svg";
                color: textColour;
            }
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: settingsButton.down ? buttonPressedColour : (settingsButton.hovered ? buttonHoverColour : buttonColour)
                border.width: 1
                border.color: settingsButton.down ? buttonPressedStroke : (settingsButton.hovered ? buttonHoverStroke : buttonStroke)
            }
            onClicked: { virtualstudio.windowState = "settings"; audio.startAudio(); }
            display: AbstractButton.TextBesideIcon
            font {
                family: "Poppins";
                pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale;
            }
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

    Connections {
        target: virtualstudio
        // Need to do this to avoid layout issues with our section header.
        function onNewScale() {
            studioListView.positionViewAtEnd();
            studioListView.positionViewAtBeginning();
            scrollY = studioListView.contentY;
        }
        function onRefreshFinished(index) {
            if (index == -1) {
                studioListView.contentY = scrollY
            } else {
                studioListView.positionViewAtIndex(index, ListView.Beginning);
            }
        }
    }
}
