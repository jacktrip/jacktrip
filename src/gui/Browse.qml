import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    width: 696; height: 577
    
    property bool refreshing: false
    
    property int buttonHeight: 25
    property int buttonWidth: 103
    property int fontMedium: 11

    Rectangle {
        z: 1
        width: parent.width; height: parent.height
        color: "#40000000"
        visible: refreshing
        MouseArea {
            anchors.fill: parent
            propagateComposedEvents: false
            hoverEnabled: true
            preventStealing: true
        }
    }
    
    Component {
        id: sectionHeading
        Rectangle {
            color: "transparent"
            height: 72; x: 16; width: ListView.view.width - (2 * x)
            // required property string section
            Text {
                //anchors.bottom: parent.bottom
                y: 12
                text: parent.section
                font { family: "Poppins"; pointSize: 28 * virtualstudio.fontScale; weight: Font.Bold }
            }
            Image {
                source: "logo.svg"
                width: 32; height: 59
                anchors.right: parent.right
                visible: parent.section == virtualstudio.logoSection ? true : false
            }
        }
    }
    
    Component {
        id: footer
        Rectangle {
            height: 16
            x: 16
            width: parent.width - (2 * x)
        }
    }

    ListView {
        id: studioListView
        x:0; y: 0; width: parent.width - (2 * x); height: parent.height - 36
        spacing: 16
        header: footer
        footer: footer
        model: serverModel
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        delegate: Studio {
            serverLocation: location
            flagImage: flag
            studioName: name
            publicStudio: isPublic
            manageable: isManageable
            available: canConnect
            connected: false
        }
        
        section {property: "type"; criteria: ViewSection.FullString; delegate: sectionHeading }

        // Disable momentum scroll
        MouseArea {
            z: -1
            anchors.fill: parent
            onWheel: {
                // trackpad
                studioListView.contentY -= wheel.pixelDelta.y;
                // mouse wheel
                studioListView.contentY -= wheel.angleDelta.y;
                studioListView.returnToBounds();
            }
        }

        Component.onCompleted: {
            // Customize scroll properties on different platforms
            if (Qt.platform.os == "linux" || Qt.platform.os == "osx" ||
                Qt.platform.os == "unix" || Qt.platform.os == "windows") {
                var scrollBar = Qt.createQmlObject('import QtQuick.Controls 2.15; ScrollBar{}',
                                                   studioListView,
                                                   "dynamicSnippet1");
                scrollBar.policy = ScrollBar.AlwaysOn;
                ScrollBar.vertical = scrollBar;
            }
        }
    }
    
    Rectangle {
        x: 0; y: parent.height - 36; width: parent.width; height: 36
        border.color: "#33979797"
        
        Button {
            id: refreshButton
            background: Rectangle {
                radius: 6
                color: refreshButton.down ? "#4E979797" : (refreshButton.hovered ? "#34979797" : "#1A979797")
                border { width: 0.3; color: "#34979797" }
            }
            onClicked: { refreshing = true; virtualstudio.refreshStudios() }
            anchors.verticalCenter: parent.verticalCenter
            x: 16; width: buttonWidth; height: buttonHeight
            Text {
                text: "Refresh List"
                font { family: "Poppins"; pointSize: fontMedium * virtualstudio.fontScale }
                anchors {horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
            }
        }
        
        Button {
            id: aboutButton
            background: Rectangle {
                radius: 6
                color: aboutButton.down ? "#4E979797" : (aboutButton.hovered ? "#34979797" : "#1A979797")
                border { width: 0.3; color: "#34979797" }
            }
            onClicked: { virtualstudio.showAbout() }
            anchors.verticalCenter: parent.verticalCenter
            x: parent.width - 230; width: buttonWidth; height: buttonHeight
            Text {
                text: "About"
                font { family: "Poppins"; pointSize: fontMedium * virtualstudio.fontScale }
                anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
            }
        }
        
        Button {
            id: settingsButton
            background: Rectangle {
                radius: 6
                color: settingsButton.down ? "#4E979797" : (settingsButton.hovered ? "#34979797" : "#1A979797")
                border { width: 0.3; color: "#34979797" }
            }
            onClicked: window.state = "settings"
            anchors.verticalCenter: parent.verticalCenter
            x: 577; width: 103; height: 25
            Text {
                text: "Settings"
                font { family: "Poppins"; pointSize: fontMedium * virtualstudio.fontScale }
                anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
            }
        }
    }
}
