import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import QtWebEngine

Item {
    width: parent.width; height: parent.height
    clip: true

    function contentScriptFactory (port) {
        return `
            // add script tag for qwebchannel
            var script = document.createElement("script");
            script.onload = function () {
                var url = "ws://localhost:${port}";
                var socket = new WebSocket(url);
                
                socket.onclose = function() {
                    console.error("web channel closed");
                };
                socket.onerror = function(error) {
                    console.error("web channel error: " + error);
                };
                socket.onopen = function() {
                    new QWebChannel(socket, function(channel) {
                        // make core object accessible globally
                        window.virtualstudio = channel.objects.virtualstudio;
                        console.log("[QT6] Connected to WebChannel, ready to send/receive messages!");
                    });
                }

            }

            script.setAttribute("src", "qrc:///qtwebchannel/qwebchannel.js");
            script.setAttribute("type", "text/javascript");
            document.head.appendChild(script);

            console.log("[QT] Loaded content script.");
        `
    }

    Item {
        id: web
        anchors.fill: parent

        property string accessToken: auth.isAuthenticated && Boolean(auth.accessToken) ? auth.accessToken : ""
        property string studioId: virtualstudio.currentStudio >= 0 ? serverModel[virtualstudio.currentStudio].id : ""

        WebEngineView {
            id: webEngineView
            anchors.fill: parent
            settings.javascriptCanAccessClipboard: true
            settings.javascriptCanPaste: true
            settings.screenCaptureEnabled: true
            profile.httpUserAgent: `JackTrip/${virtualstudio.versionString}`
            url: `http://localhost:3000/studios/${studioId}/live?accessToken=${accessToken}`

            // useful for debugging
            // onJavaScriptConsoleMessage: function(level, message, lineNumber, sourceID) {
            //     console.log(level, message, lineNumber, sourceID);
            // }

            // useful for debugging
            // onLoadingChanged: function(loadRequest) {
            //     console.log("onLoadingChanged", loadRequest.errorCode, loadRequest.errorDomain, loadRequest.errorString, loadRequest.status, loadRequest.url);
            // }

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

            onNavigationRequested: function(request) {
                webEngineView.userScripts.collection = [
                    {
                        name: "script",
                        sourceCode: contentScriptFactory(virtualstudio.port),
                        injectionPoint: WebEngineScript.DocumentReady,
                        worldId: WebEngineScript.MainWorld
                    }
                ]
            }
        }
    }
}
