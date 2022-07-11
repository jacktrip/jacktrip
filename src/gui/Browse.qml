import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.12

Item {
    width: parent.width; height: parent.height
    clip: true

    Rectangle {
        width: parent.width; height: parent.height
        color: backgroundColour
    }
    
    property bool refreshing: false
    
    property int buttonHeight: 25
    property int buttonWidth: 103
    property int fontMedium: 11
    
    property int scrollY: 0
    
    property string backgroundColour: virtualstudio.darkMode ? "#272525" : "#FAFBFB"
    property string textColour: virtualstudio.darkMode ? "#FAFBFB" : "#0F0D0D"
    property string buttonColour: virtualstudio.darkMode ? "#494646" : "#EAECEC"
    property string buttonHoverColour: virtualstudio.darkMode ? "#5B5858" : "#D3D4D4"
    property string buttonPressedColour: virtualstudio.darkMode ? "#524F4F" : "#DEE0E0"
    property string buttonStroke: virtualstudio.darkMode ? "#80827D7D" : "#40979797"
    property string buttonHoverStroke: virtualstudio.darkMode ? "#7B7777" : "#BABCBC"
    property string buttonPressedStroke: virtualstudio.darkMode ? "#827D7D" : "#BABCBC"
    
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
                id: sectionText
                //anchors.bottom: parent.bottom
                y: 12 * virtualstudio.uiScale
                // text: parent.section (for 5.15)
                text: section
                font { family: "Poppins"; pixelSize: 28 * virtualstudio.fontScale * virtualstudio.uiScale; weight: Font.Bold }
                color: textColour
            }
            Button {
                id: createButton
                background: Rectangle {
                    radius: 6 * virtualstudio.uiScale
                    color: createButton.down ? "#E7E8E8" : "#F2F3F3"
                    border.width: 1
                    border.color: createButton.down ? "#B0B5B5" : "#EAEBEB"
                    layer.enabled: createButton.hovered && !createButton.down
                    layer.effect: DropShadow {
                        horizontalOffset: 1 * virtualstudio.uiScale
                        verticalOffset: 1 * virtualstudio.uiScale
                        radius: 8.0 * virtualstudio.uiScale
                        samples: 17
                        color: "#80A1A1A1"
                    }
                }
                onClicked: { virtualstudio.createStudio(); }
                anchors.right: filterButton.left
                anchors.rightMargin: 16
                anchors.verticalCenter: sectionText.verticalCenter
                width: 150 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
                Text {
                    text: "Create a Studio"
                    font.family: "Poppins"
                    font.pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale
                    font.weight: Font.Bold
                    color: "#DB0A0A"
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                }
                visible: section == virtualstudio.logoSection ? true : false
            }
            Button {
                id: filterButton
                background: Rectangle {
                    radius: 6 * virtualstudio.uiScale
                    color: filterButton.down ? "#E7E8E8" : "#F2F3F3"
                    border.width: 1
                    border.color: filterButton.down ? "#B0B5B5" : "#EAEBEB"
                    layer.enabled: filterButton.hovered && !filterButton.down
                    layer.effect: DropShadow {
                        horizontalOffset: 1 * virtualstudio.uiScale
                        verticalOffset: 1 * virtualstudio.uiScale
                        radius: 8.0 * virtualstudio.uiScale
                        samples: 17
                        color: "#80A1A1A1"
                    }
                }
                onClicked: { filterMenu.open(); }
                anchors.right: parent.right
                anchors.verticalCenter: sectionText.verticalCenter
                width: 150 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
                Text {
                    text: "Filter Studios"
                    font.family: "Poppins"
                    font.pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                }
                visible: section == virtualstudio.logoSection ? true : false

                Popup {
                    id: filterMenu
                    y: Math.round(parent.height + 8)
                    rightMargin: 16 * virtualstudio.uiScale
                    width: 210 * virtualstudio.uiScale; height: 64 * virtualstudio.uiScale
                    modal: false
                    focus: false
                    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
                    background: Rectangle {
                        radius: 6 * virtualstudio.uiScale
                        color: "#F6F8F8"
                        border.width: 1
                        border.color: "#34979797"
                        layer.enabled: true
                        layer.effect: DropShadow {
                            horizontalOffset: 1 * virtualstudio.uiScale
                            verticalOffset: 1 * virtualstudio.uiScale
                            radius: 8.0 * virtualstudio.uiScale
                            samples: 17
                            color: "#80A1A1A1"
                        }
                    }
                    contentItem: Column {
                        anchors.fill: parent
                        CheckBox {
                            id: inactiveCheckbox
                            text: qsTr("Show my inactive Studios")
                            checkState: virtualstudio.showInactive ? Qt.Checked : Qt.Unchecked
                            onClicked: { virtualstudio.showInactive = inactiveCheckbox.checkState == Qt.Checked;
                                refreshing = true;
                                refresh();
                            }
                            indicator: Rectangle {
                                implicitWidth: 16 * virtualstudio.uiScale
                                implicitHeight: 16 * virtualstudio.uiScale
                                x: inactiveCheckbox.leftPadding
                                y: parent.height / 2 - height / 2
                                radius: 3 * virtualstudio.uiScale
                                border.color: inactiveCheckbox.down ? "#007AFF" : "#0062cc"

                                Rectangle {
                                    width: 10 * virtualstudio.uiScale
                                    height: 10 * virtualstudio.uiScale
                                    x: 3 * virtualstudio.uiScale
                                    y: 3 * virtualstudio.uiScale
                                    radius: 2 * virtualstudio.uiScale
                                    color: inactiveCheckbox.down ? "#007AFF" : "#0062cc"
                                    visible: inactiveCheckbox.checked
                                }
                            }
                            contentItem: Text {
                                text: inactiveCheckbox.text
                                font.family: "Poppins"
                                font.pixelSize: 10 * virtualstudio.fontScale * virtualstudio.uiScale
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.verticalCenter: parent.verticalCenter
                                leftPadding: inactiveCheckbox.indicator.width + inactiveCheckbox.spacing
                            }
                        }
                        CheckBox {
                            id: selfHostedCheckbox
                            text: qsTr("Show self-hosted Studios")
                            checkState: virtualstudio.showSelfHosted ? Qt.Checked : Qt.Unchecked
                            onClicked: { virtualstudio.showSelfHosted = selfHostedCheckbox.checkState == Qt.Checked;
                                refreshing = true;
                                refresh();
                            }
                            indicator: Rectangle {
                                implicitWidth: 16 * virtualstudio.uiScale
                                implicitHeight: 16 * virtualstudio.uiScale
                                x: selfHostedCheckbox.leftPadding
                                y: parent.height / 2 - height / 2
                                radius: 3 * virtualstudio.uiScale
                                border.color: selfHostedCheckbox.down ? "#007AFF" : "#0062CC"

                                Rectangle {
                                    width: 10 * virtualstudio.uiScale
                                    height: 10 * virtualstudio.uiScale
                                    x: 3 * virtualstudio.uiScale
                                    y: 3 * virtualstudio.uiScale
                                    radius: 2 * virtualstudio.uiScale
                                    color: selfHostedCheckbox.down ? "#007AFF" : "#0062CC"
                                    visible: selfHostedCheckbox.checked
                                }
                            }
                            contentItem: Text {
                                text: selfHostedCheckbox.text
                                font.family: "Poppins"
                                font.pixelSize: 10 * virtualstudio.fontScale * virtualstudio.uiScale
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.verticalCenter: parent.verticalCenter
                                leftPadding: selfHostedCheckbox.indicator.width + selfHostedCheckbox.spacing
                            }
                        }
                    }
                }
            }
        }
    }
    
    Component {
        id: footer
        Rectangle {
            height: 16 * virtualstudio.uiScale
            x: 16 * virtualstudio.uiScale
            width: parent.width - (2 * x)
            color: backgroundColour
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
            serverLocation: virtualstudio.regions[location] ? "in " + virtualstudio.regions[location].label : ""
            flagImage: bannerURL ? bannerURL : flag
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
        color: backgroundColour
        
        Button {
            id: refreshButton
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: refreshButton.down ? buttonPressedColour : (refreshButton.hovered ? buttonHoverColour : buttonColour)
                border.width: 1
                border.color: refreshButton.down ? buttonPressedStroke : (refreshButton.hovered ? buttonHoverStroke : buttonStroke)
            }
            onClicked: { refreshing = true; refresh() }
            anchors.verticalCenter: parent.verticalCenter
            x: 16 * virtualstudio.uiScale
            width: buttonWidth * virtualstudio.uiScale; height: buttonHeight * virtualstudio.uiScale
            Text {
                text: "Refresh List"
                font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
                anchors {horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
                color: textColour
            }
        }
        
        Button {
            id: aboutButton
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: aboutButton.down ? buttonPressedColour : (aboutButton.hovered ? buttonHoverColour : buttonColour)
                border.width: 1
                border.color: aboutButton.down ? buttonPressedStroke : (aboutButton.hovered ? buttonHoverStroke : buttonStroke)
            }
            onClicked: { virtualstudio.showAbout() }
            anchors.verticalCenter: parent.verticalCenter
            x: parent.width - (230 * virtualstudio.uiScale)
            width: buttonWidth * virtualstudio.uiScale; height: buttonHeight * virtualstudio.uiScale
            Text {
                text: "About"
                font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
                anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
                color: textColour
            }
        }
        
        Button {
            id: settingsButton
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: settingsButton.down ? buttonPressedColour : (settingsButton.hovered ? buttonHoverColour : buttonColour)
                border.width: 1
                border.color: settingsButton.down ? buttonPressedStroke : (settingsButton.hovered ? buttonHoverStroke : buttonStroke)
            }
            onClicked: window.state = "settings"
            anchors.verticalCenter: parent.verticalCenter
            x: parent.width - (119 * virtualstudio.uiScale)
            width: buttonWidth * virtualstudio.uiScale; height: buttonHeight * virtualstudio.uiScale
            Text {
                text: "Settings"
                font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale}
                anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
                color: textColour
            }
        }
    }
    
    Connections {
        target: virtualstudio
        // Need to do this to avoid layout issues with our section header.
        function onNewScale() { 
            studioListView.positionViewAtEnd();
            studioListView.positionViewAtBeginning();
            scrollY = studioListView.contentY;
        }
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
