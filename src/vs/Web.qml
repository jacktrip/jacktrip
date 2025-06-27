import QtQuick
import QtQuick.Controls

Loader {
    anchors.fill: parent
    // use alternative for Linux that does not include onDesktopMediaRequested
    source: Qt.version >= "6.7.0" ? "OldWebEngine.qml" : "WebEngine.qml"

    // TODO: Add support for QtWebView
    // source: useWebEngine ? "WebEngine.qml" : "WebView.qml"
}
