import QtQuick
import QtQuick.Controls

Item {
    id: appIcon

    property alias icon: btn.icon
    property string color: ""
    property string defaultColor: virtualstudio.darkMode ? "#CCCCCC" : "#333333"
    signal clicked()

    Button {
        id: btn
        anchors.fill: parent
        anchors.centerIn: parent
        topInset: 0
        leftInset: 0
        rightInset: 0
        bottomInset: 0
        padding: 0

        background: Rectangle { color: "transparent" }
        icon.color: color ? color : defaultColor
        icon.width: parent.width
        icon.height: parent.height
        display: AbstractButton.IconOnly
        onClicked: appIcon.clicked()
    }
}
