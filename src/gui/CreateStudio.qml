import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtWebEngine
import org.jacktrip.jacktrip 1.0

Item {
    width: parent.width; height: parent.height
    clip: true

    property int fontMedium: 12
    property string browserButtonColour: virtualstudio.darkMode ? "#494646" : "#EAECEC"
    property string browserButtonHoverColour: virtualstudio.darkMode ? "#5B5858" : "#D3D4D4"
    property string browserButtonPressedColour: virtualstudio.darkMode ? "#524F4F" : "#DEE0E0"
    property string buttonStroke: virtualstudio.darkMode ? "#80827D7D" : "#34979797"

    Loader {
        id: webLoader
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.bottom: footer.top
        property string accessToken: auth.isAuthenticated && Boolean(auth.accessToken) ? auth.accessToken : ""
        sourceComponent: virtualstudio.windowState === "create_studio" && accessToken ? createStudioWeb : createStudioNull
    }

    Component {
        id: createStudioNull
        Rectangle {
            anchors.fill: parent
            color: backgroundColour
        }
    }

    Component {
        id: createStudioWeb
        WebEngineView {
            id: webEngineView
            anchors.fill: parent
            settings.javascriptCanAccessClipboard: true
            settings.javascriptCanPaste: true
            settings.screenCaptureEnabled: true
            profile.httpUserAgent: `JackTrip/${virtualstudio.versionString}`
            url: `https://${virtualstudio.apiHost}/qt/create?accessToken=${accessToken}`

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
        anchors.bottom: parent.bottom
        width: parent.width
        height: 48
        color: backgroundColour

        RowLayout {
            id: layout
            anchors.fill: parent

            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true

                Button {
                    id: backButton
                    anchors.centerIn: parent
                    width: 180 * virtualstudio.uiScale
                    height: 36 * virtualstudio.uiScale
                    background: Rectangle {
                        radius: 8 * virtualstudio.uiScale
                        color: backButton.down ? browserButtonPressedColour : (backButton.hovered ? browserButtonHoverColour : browserButtonColour)
                    }
                    onClicked: virtualstudio.windowState = "browse"

                    Text {
                        text: "Back to Studios"
                        font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale}
                        anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
                        color: textColour
                    }
                }
            }
        }

        Rectangle {
            id: backgroundBorder
            width: parent.width
            height: 1
            y: parent.height - footer.height
            color: buttonStroke
        }
    }
}
