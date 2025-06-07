import QtQuick
import QtQuick.Controls

Item {
    anchors.centerIn: parent
    width: 480 * virtualstudio.uiScale

    property var mediaRequest: null

    // Theme-aware colors matching DeviceWarningModal
    property string textColour: virtualstudio.darkMode ? "#FAFBFB" : "#0F0D0D"
    property string buttonColour: virtualstudio.darkMode ? "#494646" : "#EAECEC"
    property string buttonPressedStroke: virtualstudio.darkMode ? "#827D7D" : "#BABCBC"
    property string buttonPressedColour: virtualstudio.darkMode ? "#524F4F" : "#DEE0E0"
    property string buttonStroke: virtualstudio.darkMode ? "#80827D7D" : "#34979797"
    property string buttonHoverStroke: virtualstudio.darkMode ? "#7B7777" : "#BABCBC"
    property string buttonHoverColour: virtualstudio.darkMode ? "#5B5858" : "#D3D4D4"

    Popup {
        id: screenSharePopup
        padding: 1
        width: parent.width
        height: 400 * virtualstudio.uiScale
        anchors.centerIn: parent
        modal: true
        focus: true

        background: Rectangle {
            anchors.fill: parent
            color: "transparent"
            radius: 6 * virtualstudio.uiScale
            border.width: 1
            border.color: buttonStroke
            clip: true
        }

        contentItem: Rectangle {
            id: screenShareContent
            width: parent.width
            height: parent.height
            color: backgroundColour
            radius: 6 * virtualstudio.uiScale

            Text {
                id: titleText
                text: "What would you like to share?"
                font.family: "Poppins"
                font.pixelSize: 12 * virtualstudio.fontScale * virtualstudio.uiScale
                font.bold: true
                color: textColour
                anchors.top: parent.top
                anchors.topMargin: 16 * virtualstudio.uiScale
                anchors.left: parent.left
                anchors.leftMargin: 24 * virtualstudio.uiScale
            }

            Button {
                id: cancelButton
                width: 160 * virtualstudio.uiScale
                height: 30 * virtualstudio.uiScale
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 16 * virtualstudio.uiScale

                onClicked: {
                    if (mediaRequest) {
                        mediaRequest.cancel();
                    }
                    screenSharePopup.close();
                }

                background: Rectangle {
                    radius: 6 * virtualstudio.uiScale
                    color: parent.down ? buttonPressedColour : (parent.hovered ? buttonHoverColour : buttonColour)
                    border.width: 1
                    border.color: parent.down ? buttonPressedStroke : (parent.hovered ? buttonHoverStroke : buttonStroke)
                }

                contentItem: Text {
                    text: "Cancel"
                    font.family: "Poppins"
                    font.pixelSize: 10 * virtualstudio.fontScale * virtualstudio.uiScale
                    color: textColour
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            ScrollView {
                id: scrollingSection
                x: 24 * virtualstudio.uiScale
                y: titleText.y + titleText.height + 16 * virtualstudio.uiScale
                width: screenShareContent.width - 48 * virtualstudio.uiScale
                height: screenShareContent.height - titleText.height - cancelButton.height - 64 * virtualstudio.uiScale
                clip: true
                
                Column {
                    width: scrollingSection.width - 20 * virtualstudio.uiScale
                    spacing: 8 * virtualstudio.uiScale
                    
                    // Screens section
                    Column {
                        width: parent.width
                        spacing: 4 * virtualstudio.uiScale
                        visible: mediaRequest && mediaRequest.screensModel.rowCount() > 0
                        
                        Text {
                            text: "Screens:"
                            font.family: "Poppins"
                            font.pixelSize: 10 * virtualstudio.fontScale * virtualstudio.uiScale
                            font.bold: true
                            color: textColour
                        }
                        
                        Repeater {
                            model: mediaRequest ? mediaRequest.screensModel : null
                            delegate: Rectangle {
                                width: parent.width
                                height: 36 * virtualstudio.uiScale
                                color: screensMouseArea.containsMouse ? buttonHoverColour : buttonColour
                                border.color: screensMouseArea.containsMouse ? buttonHoverStroke : buttonStroke
                                border.width: 1
                                radius: 6 * virtualstudio.uiScale
                            
                                Row {
                                    anchors.left: parent.left
                                    anchors.leftMargin: 12 * virtualstudio.uiScale
                                    anchors.verticalCenter: parent.verticalCenter
                                    spacing: 12 * virtualstudio.uiScale
                                    
                                    Rectangle {
                                        width: 20 * virtualstudio.uiScale
                                        height: 20 * virtualstudio.uiScale
                                        color: "#4CAF50"
                                        radius: 3 * virtualstudio.uiScale
                                        anchors.verticalCenter: parent.verticalCenter
                                        
                                        Text {
                                            anchors.centerIn: parent
                                            text: "🖥️"
                                            font.pixelSize: 10 * virtualstudio.fontScale * virtualstudio.uiScale
                                            color: "white"
                                        }
                                    }
                                    
                                    Text {
                                        text: display || ("Screen " + (index + 1))
                                        anchors.verticalCenter: parent.verticalCenter
                                        font.family: "Poppins"
                                        font.pixelSize: 10 * virtualstudio.fontScale * virtualstudio.uiScale
                                        color: textColour
                                        elide: Text.ElideRight
                                        width: parent.parent.width - (60 * virtualstudio.uiScale)
                                    }
                                }
                            
                                MouseArea {
                                    id: screensMouseArea
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onClicked: {
                                        if (mediaRequest) {
                                            var screenIndex = mediaRequest.screensModel.index(index, 0);
                                            mediaRequest.selectScreen(screenIndex);
                                            screenSharePopup.close();
                                        }
                                    }
                                }
                            }
                        }
                    }
                    
                    // Windows section
                    Column {
                        width: parent.width
                        spacing: 4 * virtualstudio.uiScale
                        visible: mediaRequest && mediaRequest.windowsModel.rowCount() > 0
                        
                        Text {
                            text: "Windows:"
                            font.family: "Poppins"
                            font.pixelSize: 10 * virtualstudio.fontScale * virtualstudio.uiScale
                            font.bold: true
                            color: textColour
                        }
                        
                        Repeater {
                            model: mediaRequest ? mediaRequest.windowsModel : null
                            delegate: Rectangle {
                                width: parent.width
                                height: 36 * virtualstudio.uiScale
                                color: windowsMouseArea.containsMouse ? buttonHoverColour : buttonColour
                                border.color: windowsMouseArea.containsMouse ? buttonHoverStroke : buttonStroke
                                border.width: 1
                                radius: 6 * virtualstudio.uiScale
                            
                                Row {
                                    anchors.left: parent.left
                                    anchors.leftMargin: 12 * virtualstudio.uiScale
                                    anchors.verticalCenter: parent.verticalCenter
                                    spacing: 12 * virtualstudio.uiScale
                                    
                                    Rectangle {
                                        width: 20 * virtualstudio.uiScale
                                        height: 20 * virtualstudio.uiScale
                                        color: "#2196F3"
                                        radius: 3 * virtualstudio.uiScale
                                        anchors.verticalCenter: parent.verticalCenter
                                        
                                        Text {
                                            anchors.centerIn: parent
                                            text: "🪟"
                                            font.pixelSize: 10 * virtualstudio.fontScale * virtualstudio.uiScale
                                            color: "white"
                                        }
                                    }
                                    
                                    Text {
                                        text: display || ("Window " + (index + 1))
                                        anchors.verticalCenter: parent.verticalCenter
                                        font.family: "Poppins"
                                        font.pixelSize: 10 * virtualstudio.fontScale * virtualstudio.uiScale
                                        color: textColour
                                        elide: Text.ElideRight
                                        width: parent.parent.width - (60 * virtualstudio.uiScale)
                                    }
                                }
                            
                                MouseArea {
                                    id: windowsMouseArea
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onClicked: {
                                        if (mediaRequest) {
                                            var windowIndex = mediaRequest.windowsModel.index(index, 0);
                                            mediaRequest.selectWindow(windowIndex);
                                            screenSharePopup.close();
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    function open() {
        screenSharePopup.open();
    }

    function close() {
        screenSharePopup.close();
    }
} 