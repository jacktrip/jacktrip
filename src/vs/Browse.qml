import QtQuick
import QtQuick.Controls

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
        model: virtualstudio.serverModel
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        delegate: Studio {
            x: 16 * virtualstudio.uiScale
            width: studioListView.width - (2 * x)
            serverLocation: virtualstudio.regions[modelData.location] ? "in " + virtualstudio.regions[modelData.location].label : ""
            flagImage: modelData.bannerURL ? modelData.bannerURL : modelData.flag
            studioName: modelData.name
            publicStudio: modelData.isPublic
            admin: modelData.isAdmin
            available: modelData.canConnect
            connected: false
            studioId: modelData.id ? modelData.id : ""
            streamId: modelData.streamId ? modelData.streamId : ""
            inviteKeyString: modelData.inviteKey ? modelData.inviteKey : ""
            sampleRate: modelData.sampleRate
        }

        section { property: "modelData.type"; criteria: ViewSection.FullString; delegate: SectionHeading {} }

        // Show sectionHeading if there are no Studios in list
        SectionHeading {
            id: emptyListSectionHeading
            listIsEmpty: true
            visible: parent.count == 0
        }

        Text {
            id: emptyListMessage
            visible: parent.count == 0
            text: virtualstudio.isRefreshingStudios ? "Loading Studios..." : "No studios found that match your filter criteria."
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
            visible: parent.count == 0
            onClicked: {
                virtualstudio.showSelfHosted = true;
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

        // Disable momentum scroll
        MouseArea {
            z: -1
            anchors.fill: parent
            onWheel: function (wheel) {
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
                var scrollBar = Qt.createQmlObject('import QtQuick.Controls; ScrollBar{}',
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
            onClicked: { virtualstudio.windowState = "settings"; audio.startAudio(); }
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

    AboutWindow {
    }

    FeedbackSurvey {
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
