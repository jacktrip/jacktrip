import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    width: parent.width; height: parent.height
    clip: true
    
    property bool refreshing: false
    
    property int buttonHeight: 25
    property int buttonWidth: 103
    property int fontMedium: 11
    
    property int scrollY: 0

    function refresh() {
        scrollY = studioListView.contentY;
        var currentIndex = studioListView.indexAt(16 * virtualstudio.uiScale, studioListView.contentY);
        if (currentIndex == -1) {
            currentIndex = studioListView.indexAt(16 * virtualstudio.uiScale, studioListView.contentY + (16 * virtualstudio.uiScale));
        }
        virtualstudio.refreshStudios(currentIndex)
    }
    
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
            height: 72 * virtualstudio.uiScale; x: 16 * virtualstudio.uiScale; width: ListView.view.width - (2 * x)
            // required property string section: section (for 5.15)
            Text {
                //anchors.bottom: parent.bottom
                y: 12 * virtualstudio.uiScale
                // text: parent.section (for 5.15)
                text: section
                font { family: "Poppins"; pixelSize: 28 * virtualstudio.fontScale * virtualstudio.uiScale; weight: Font.Bold }
            }
            Image {
                source: "logo.svg"
                width: 32 * virtualstudio.uiScale; height: 59 * virtualstudio.uiScale
                anchors.right: parent.right
                // visible: parent.section == virtualstudio.logoSection ? true : false (for 5.15)
                visible: section == virtualstudio.logoSection ? true : false
            }
        }
    }
    
    Component {
        id: footer
        Rectangle {
            height: 16 * virtualstudio.uiScale
            x: 16 * virtualstudio.uiScale
            width: parent.width - (2 * x)
            color: "#FAFBFB"
        }
    }

    ListView {
        id: studioListView
        x:0; y: 0; width: parent.width - (2 * x); height: parent.height - 36 * virtualstudio.uiScale
        spacing: 16 * virtualstudio.uiScale
        header: footer
        footer: footer
        model: serverModel
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        delegate: Studio {
            x: 16 * virtualstudio.uiScale
            width: studioListView.width - (2 * x)
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
                var scrollBar = Qt.createQmlObject('import QtQuick.Controls 2.12; ScrollBar{}',
                                                   studioListView,
                                                   "dynamicSnippet1");
                scrollBar.policy = ScrollBar.AlwaysOn;
                ScrollBar.vertical = scrollBar;
            }
        }
    }
    
    Rectangle {
        x: 0; y: parent.height - 36 * virtualstudio.uiScale; width: parent.width; height: 36 * virtualstudio.uiScale
        border.color: "#33979797"
        
        Button {
            id: refreshButton
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: refreshButton.down ? "#DEE0E0" : (refreshButton.hovered ? "#D3D4D4" : "#EAECEC")
                border.width: 1
                border.color: refreshButton.down || refreshButton.hovered ? "#BABCBC" : "#34979797"
            }
            onClicked: { refreshing = true; refresh() }
            anchors.verticalCenter: parent.verticalCenter
            x: 16 * virtualstudio.uiScale
            width: buttonWidth * virtualstudio.uiScale; height: buttonHeight * virtualstudio.uiScale
            Text {
                text: "Refresh List"
                font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
                anchors {horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
            }
        }
        
        Button {
            id: aboutButton
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: aboutButton.down ? "#DEE0E0" : (aboutButton.hovered ? "#D3D4D4" : "#EAECEC")
                border.width: 1
                border.color: aboutButton.down || aboutButton.hovered ? "#BABCBC" : "#34979797"
            }
            onClicked: { virtualstudio.showAbout() }
            anchors.verticalCenter: parent.verticalCenter
            x: parent.width - (230 * virtualstudio.uiScale)
            width: buttonWidth * virtualstudio.uiScale; height: buttonHeight * virtualstudio.uiScale
            Text {
                text: "About"
                font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
                anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
            }
        }
        
        Button {
            id: settingsButton
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: settingsButton.down ? "#DEE0E0" : (settingsButton.hovered ? "#D3D4D4" : "#EAECEC")
                border.width: 1
                border.color: settingsButton.down || settingsButton.hovered ? "#BABCBC" : "#34979797"
            }
            onClicked: window.state = "settings"
            anchors.verticalCenter: parent.verticalCenter
            x: parent.width - (119 * virtualstudio.uiScale)
            width: buttonWidth * virtualstudio.uiScale; height: buttonHeight * virtualstudio.uiScale
            Text {
                text: "Settings"
                font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale}
                anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
            }
        }
    }
    
    Connections {
        target: virtualstudio
        // Need to do this to avoid layout issues with our section header.
        function onNewScale() { studioListView.positionViewAtEnd(); studioListView.positionViewAtBeginning() }
        function onRefreshFinished(index) {
            refreshing = false;
            if (index == -1) {
                studioListView.contentY = scrollY
            } else {
                studioListView.positionViewAtIndex(index, ListView.Beginning);
            }
        }
        function onPeriodicRefresh() { refresh() }
    }
}
