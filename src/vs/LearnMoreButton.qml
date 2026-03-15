import QtQuick
import QtQuick.Controls

StyledButton {
    property string url
    property string buttonText: "Learn more"

    text: buttonText
    width: 150 * virtualstudio.uiScale
    height: 30 * virtualstudio.uiScale
    onClicked: virtualstudio.openLink(url)
}
