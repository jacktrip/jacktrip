import QtQuick
import QtQuick.Controls

StyledButton {
    id: refreshButton
    text: "Refresh Devices"
    fontSize: 8
    display: AbstractButton.TextBesideIcon

    property var onDeviceRefresh: function () { audio.refreshDevices(); };

    width: 144 * virtualstudio.uiScale
    height: 30 * virtualstudio.uiScale
    icon {
        source: "refresh.svg"
        color: resolvedTextColor
    }
    onClicked: { onDeviceRefresh(); }
}
