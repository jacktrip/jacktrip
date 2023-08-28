import QtQuick
import QtQuick.Controls
import QtWebEngine

Item {
    width: parent.width; height: parent.height
    clip: true

    function contentScriptFactory (port) {
        return `
            // add script tag for qwebchannel
            document.head.addEventListener("initqwebchannel", () => {

                var script = document.createElement("script");
                script.onload = function () {
                    var url = "ws://localhost:${port}";
                    var socket = new WebSocket(url);
                    
                    socket.onclose = function() {
                        console.error("[QT] web channel closed");
                    };
                    socket.onerror = function(event) {
                        console.error("[QT] web channel error: " + event.type);
                    };
                    socket.onopen = function() {
                        new QWebChannel(socket, function(channel) {
                            console.log("[QT] Socket opened");

                            // make core object accessible globally
                            window.virtualstudio = channel.objects.virtualstudio;
                            window.auth = channel.objects.auth;
                            window.clipboard = channel.objects.clipboard;

                            const event = new CustomEvent("qwebchannelinitialized");
                            document.head.dispatchEvent(event);
                            console.log("[QT] Dispatched qwebchannelinitialized event");
                            console.log("[QT] Connected to WebChannel, ready to send/receive messages!");
                        });
                    }
                }
                script.setAttribute("src", "qrc:///qtwebchannel/qwebchannel.js");
                script.setAttribute("type", "text/javascript");
                document.head.appendChild(script);
                console.log("[QT] Added qwebchannel initialization script to DOM.");
            });
            console.log("[QT] Added initqwebchannel event listener");
        `
    }

    Rectangle {
        id: web
        anchors.fill: parent
        color: backgroundColour

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

            onNavigationRequested: function(request) {
                webEngineView.userScripts.collection = [
                    {
                        name: "script",
                        sourceCode: contentScriptFactory(virtualstudio.webChannelPort),
                        injectionPoint: WebEngineScript.DocumentReady,
                        worldId: WebEngineScript.MainWorld
                    }
                ]
            }
        }
    }
}
