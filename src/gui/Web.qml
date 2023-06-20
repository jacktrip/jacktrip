import QtQuick 2.12
import QtQuick.Controls 2.12
import QtWebView 1.15

Item {
    width: parent.width; height: parent.height
    clip: true

    function contentScriptFactory (port) {
        console.log("PORT: ", port);
        return `
            // add script tag for qwebchannel
            var script = document.createElement("script");
            script.onload = function () {
                var url = "ws://localhost:${port}";
                var socket = new WebSocket(url);

                document.body.innerHTML = "Script running!";
                
                socket.onclose = function() {
                    console.error("web channel closed");
                };
                socket.onerror = function(error) {
                    console.error("web channel error: " + error);
                };
                socket.onopen = function() {
                    new QWebChannel(socket, function(channel) {
                        window.virtualstudio = channel.objects.virtualstudio;
                        window.auth = channel.objects.auth;
                        window.clipboard = channel.objects.clipboard;
                        console.log("[QT6] Connected to WebChannel, ready to send/receive messages!");

                        var event = new CustomEvent("qwebchannelinitialized");
                        document.dispatchEvent(event);
                    });
                }
            }

            script.onerror = function(e) {
                var msg = JSON.stringify(e, ["message", "arguments", "type", "name", "isTrusted"]);
                document.body.innerHTML = msg;
            }

            script.setAttribute("type", "text/javascript");
            script.setAttribute("crossorigin", "anonymous");
            script.setAttribute("src", "https://${virtualstudio.apiHost}/qt-utils/qt5/qwebchannel.js"); // using qrc protocol leads to CORS error
            document.head.appendChild(script);
        `
    }

    Item {
        id: web
        anchors.fill: parent

        property string accessToken: auth.isAuthenticated && Boolean(auth.accessToken) ? auth.accessToken : ""
        property string studioId: virtualstudio.currentStudio >= 0 ? serverModel[virtualstudio.currentStudio].id : ""

        WebView {
            id: webEngineView
            anchors.fill: parent
            // settings.javascriptCanAccessClipboard: true
            // settings.javascriptCanPaste: true
            // settings.screenCaptureEnabled: true
            // profile.httpUserAgent: `JackTrip/${virtualstudio.versionString}`
            url: `https://${virtualstudio.apiHost}/studios/${studioId}/live?accessToken=${accessToken}`

            // useful for debugging
            // onJavaScriptConsoleMessage: function(level, message, lineNumber, sourceID) {
            //     console.log(level, message, lineNumber, sourceID);
            // }

            // useful for debugging
            // onLoadingChanged: function(loadRequest) {
            //     console.log("onLoadingChanged", loadRequest.errorCode, loadRequest.errorDomain, loadRequest.errorString, loadRequest.status, loadRequest.url);
            // }

            onLoadingChanged: function(loadRequest) {
                if (loadRequest.status === WebView.LoadSucceededStatus) {
                    const script = contentScriptFactory(virtualstudio.port);
                    webEngineView.runJavaScript(script);
                }
            }


            /*
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
            */
        }
    }
}
