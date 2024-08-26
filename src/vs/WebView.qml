import QtQuick
import QtQuick.Controls
import QtWebView

Item {
    width: parent.width; height: parent.height
    clip: true

    Item {
        id: web
        anchors.fill: parent

        property string accessToken: auth.isAuthenticated && Boolean(auth.accessToken) ? auth.accessToken : ""
        property string studioId: virtualstudio.currentStudio.id

        WebView {
            id: webEngineView
            anchors.fill: parent
            httpUserAgent: `JackTrip/${virtualstudio.versionString}`
            url: `https://${virtualstudio.apiHost}/studios/${studioId}/live?accessToken=${accessToken}`
        }
    }
}
