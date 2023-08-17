import QtQuick
import QtQuick.Controls
import QtWebEngine

Item {
    width: parent.width; height: parent.height
    clip: true

    Item {
        id: web
        anchors.fill: parent

        property string accessToken: auth.isAuthenticated && Boolean(auth.accessToken) ? auth.accessToken : ""
        property string studioId: virtualstudio.currentStudio.id

        WebEngineView {
            id: webEngineView
            anchors.fill: parent
            settings.javascriptCanAccessClipboard: true
            settings.javascriptCanPaste: true
            settings.screenCaptureEnabled: true
            profile.httpUserAgent: `JackTrip/${virtualstudio.versionString}`
            url: `https://${virtualstudio.apiHost}/studios/${studioId}/live?accessToken=${accessToken}`

            // useful for debugging
            // onJavaScriptConsoleMessage: function(level, message, lineNumber, sourceID) {
            //     console.log(level, message, lineNumber, sourceID);
            // }

            // useful for debugging
            // onLoadingChanged: function(loadRequest) {
            //     console.log("onLoadingChanged", loadRequest.errorCode, loadRequest.errorDomain, loadRequest.errorString, loadRequest.status, loadRequest.url);
            // }

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
}
