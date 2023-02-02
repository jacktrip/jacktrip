import QtQuick 2.12
import QtQuick.Controls 2.12

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
    property int extraSettingsButtonWidth: 16
    property int emptyListMessageWidth: 450
    property int createMessageTopMargin: 16
    property int createButtonTopMargin: 24
    property int fontBig: 28
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
    property string createButtonStroke: virtualstudio.darkMode ? "#AB0F0F" : "#0F0D0D"
    
    function refresh() {
        scrollY = studioListView.contentY;
        var currentIndex = studioListView.indexAt(16 * virtualstudio.uiScale, studioListView.contentY);
        if (currentIndex == -1) {
            currentIndex = studioListView.indexAt(16 * virtualstudio.uiScale, studioListView.contentY + (16 * virtualstudio.uiScale));
        }
        virtualstudio.refreshStudios(currentIndex, true)
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
            admin: isAdmin
            available: canConnect
            connected: false
            studioId: id ? id : ""
            inviteKeyString: inviteKey ? inviteKey : ""
        }
        
        section {property: "type"; criteria: ViewSection.FullString; delegate: SectionHeading {} }

        // Show sectionHeading if there are no Studios in list
        SectionHeading {
            id: emptyListSectionHeading
            listIsEmpty: true
            visible: parent.count == 0 && !virtualstudio.showCreateStudio
        }

        Text {
            id: emptyListMessage
            visible: parent.count == 0 && !virtualstudio.showCreateStudio
            text: "No studios found that match your filter criteria."
            font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            width: emptyListMessageWidth
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: emptyListSectionHeading.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
        }

        Button {
            id: resetFiltersButton
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: resetFiltersButton.down ? buttonPressedColour : (resetFiltersButton.hovered ? buttonHoverColour : buttonColour)
                border.width: 1
                border.color: resetFiltersButton.down ? buttonPressedStroke : (resetFiltersButton.hovered ? buttonHoverStroke : buttonStroke)
            }
            visible: parent.count == 0 && !virtualstudio.showCreateStudio
            onClicked: {
                virtualstudio.showSelfHosted = false;
                virtualstudio.showInactive = true;
                refreshing = true;
                refresh();
            }
            anchors.top: emptyListMessage.bottom
            anchors.topMargin: createButtonTopMargin
            anchors.horizontalCenter: emptyListMessage.horizontalCenter
            width: 120 * virtualstudio.uiScale; height: 32 * virtualstudio.uiScale
            Text {
                text: "Reset Filters"
                font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
                anchors {horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
                color: textColour
            }
        }

        Rectangle {
            id: newUserEmptyList
            anchors.fill: parent
            color: "transparent"
            visible: parent.count == 0 && virtualstudio.showCreateStudio

            Rectangle {
                color: "transparent"
                width: emptyListMessageWidth
                height: createButton.height + createStudioMessage.height + welcomeMessage.height + createButtonTopMargin + createMessageTopMargin
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter

                Text {
                    id: welcomeMessage
                    text: "Welcome"
                    font { family: "Poppins"; pixelSize: fontBig * virtualstudio.fontScale * virtualstudio.uiScale; weight: Font.Bold }
                    color: textColour
                    width: emptyListMessageWidth
                    wrapMode: Text.Wrap
                    horizontalAlignment: Text.AlignHCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                }

                Text {
                    id: createStudioMessage
                    text: "Looks like you're not a member of any studios!\nHave the studio owner send you an invite link, or create your own studio to invite others."
                    font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
                    color: textColour
                    width: emptyListMessageWidth
                    wrapMode: Text.Wrap
                    horizontalAlignment: Text.AlignHCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: welcomeMessage.bottom
                    anchors.topMargin: createMessageTopMargin
                }

                Button {
                    id: createButton
                    background: Rectangle {
                        radius: 6 * virtualstudio.uiScale
                        color: createButton.down ? "#E7E8E8" : "#F2F3F3"
                        border.width: 1
                        border.color: createButton.down ? "#B0B5B5" : createButtonStroke
                        layer.enabled: createButton.hovered && !createButton.down
                    }
                    onClicked: { virtualstudio.createStudio(); }
                    anchors.top: createStudioMessage.bottom
                    anchors.topMargin: createButtonTopMargin
                    anchors.horizontalCenter: createStudioMessage.horizontalCenter
                    width: 210 * virtualstudio.uiScale; height: 45 * virtualstudio.uiScale
                    Text {
                        text: "Create a Studio"
                        font.family: "Poppins"
                        font.pixelSize: 18 * virtualstudio.fontScale * virtualstudio.uiScale
                        font.weight: Font.Bold
                        color: "#DB0A0A"
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }
        }

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
            x: parent.width - ((230 + extraSettingsButtonWidth) * virtualstudio.uiScale)
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
            text: "Settings"
            palette.buttonText: textColour
            icon {
                source: "cog.svg";
                color: textColour;
            }
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: settingsButton.down ? buttonPressedColour : (settingsButton.hovered ? buttonHoverColour : buttonColour)
                border.width: 1
                border.color: settingsButton.down ? buttonPressedStroke : (settingsButton.hovered ? buttonHoverStroke : buttonStroke)
            }
            Timer {
                id: restartAudioTimer
                interval: 805; running: false; repeat: false
                onTriggered: virtualstudio.audioActivated = true;
            }
            onClicked: { virtualstudio.windowState = "settings"; restartAudioTimer.restart(); }
            display: AbstractButton.TextBesideIcon
            font {
                family: "Poppins"; 
                pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale;
            }
            leftPadding: 0
            rightPadding: 4
            spacing: 0
            anchors.verticalCenter: parent.verticalCenter
            x: parent.width - ((119 + extraSettingsButtonWidth) * virtualstudio.uiScale)
            width: (buttonWidth + extraSettingsButtonWidth) * virtualstudio.uiScale; height: buttonHeight * virtualstudio.uiScale
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
