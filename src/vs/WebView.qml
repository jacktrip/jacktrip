import QtQuick
import QtQuick.Controls
import QtWebView

Item {
    width: parent.width; height: parent.height
    clip: true

    Item {
        id: web
        anchors.fill: parent

        property string studioId: virtualstudio.currentStudio.id

        WebView {
            id: webEngineView
            anchors.fill: parent
            url: `https://${virtualstudio.apiHost}/studios/${web.studioId}/live`
        }
    }
}
