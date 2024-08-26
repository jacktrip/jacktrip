import QtQuick
import QtQuick.Controls

Loader {
    anchors.fill: parent
    source: "WebEngine.qml"

    // TODO: Add support for QtWebView
    // source: useWebEngine ? "WebEngine.qml" : "WebView.qml"
}
