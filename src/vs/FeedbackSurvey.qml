import QtQuick
import QtQuick.Controls

Item {
    id: userFeedbackSurvey

    anchors.centerIn: parent
    width: 480 * virtualstudio.uiScale
    height: 232 * virtualstudio.uiScale

    property int leftHeaderMargin: 16
    property int fontBig: 28
    property int fontMedium: 12
    property int fontSmall: 10
    property int fontTiny: 8
    property int bodyMargin: 60
    property int rightMargin: 16
    property int bottomToolTipMargin: 8
    property int rightToolTipMargin: 4

    property string buttonColour: "#F2F3F3"
    property string buttonHoverColour: "#E7E8E8"
    property string buttonPressedColour: "#E7E8E8"
    property string buttonStroke: "#EAEBEB"
    property string buttonHoverStroke: "#B0B5B5"
    property string buttonPressedStroke: "#B0B5B5"

    property string textColour: virtualstudio.darkMode ? "#FAFBFB" : "#0F0D0D"
    property string textAreaTextColour: virtualstudio.darkMode ? "#A6A6A6" : "#757575"
    property string textAreaColour: virtualstudio.darkMode ? "#494646" : "#EAECEC"

    property string serverId: ""

    property int rating: 0
    property int hover: star1MouseArea.containsMouse ? 1 : star2MouseArea.containsMouse ? 2 : star3MouseArea.containsMouse ? 3 : star4MouseArea.containsMouse ? 4 : star5MouseArea.containsMouse ? 5 : 0
    property int currentView: (hover > 0 ? hover : rating)
    property bool submitted: false;

    property string message: ""

    Popup {
        id: userFeedbackModal
        padding: 1
        width: parent.width
        height: 300 * virtualstudio.uiScale
        anchors.centerIn: parent
        modal: true
        focus: true
        closePolicy: Popup.NoAutoClose

        background: Rectangle {
            anchors.fill: parent
            color: "transparent"
            radius: 6 * virtualstudio.uiScale
            border.width: 1
            border.color: buttonStroke
            clip: true
        }

        contentItem: Rectangle {
            width: parent.width
            height: parent.height
            color: backgroundColour
            radius: 6 * virtualstudio.uiScale

            Item {
                id: userFeedbackSurveyContent
                anchors.top: parent.top
                anchors.topMargin: 24 * virtualstudio.uiScale
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                visible: !submitted

                Text {
                    id: userFeedbackSurveyHeader
                    anchors.top: parent.top
                    anchors.topMargin: 16 * virtualstudio.uiScale
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width
                    text: "How did your session go?"
                    font {family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale; bold: true }
                    horizontalAlignment: Text.AlignHCenter
                    color: textColour
                    elide: Text.ElideRight
                    wrapMode: Text.WordWrap
                }

                Text {
                    id: ratingItemInstructions
                    anchors.top: userFeedbackSurveyHeader.bottom
                    anchors.topMargin: 12 * virtualstudio.uiScale
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width
                    text: "Rate your session on a scale of 1 to 5"
                    font {family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                    horizontalAlignment: Text.AlignHCenter
                    color: textColour
                    elide: Text.ElideRight
                    wrapMode: Text.WordWrap
                }

                Item {
                    id: ratingItem
                    anchors.top: ratingItemInstructions.bottom
                    anchors.topMargin: 4 * virtualstudio.uiScale
                    anchors.horizontalCenter: parent.horizontalCenter
                    height: 40 * virtualstudio.uiScale
                    width: 200 * virtualstudio.uiScale

                    Item {
                        id: star1Container
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: star2Container.left
                        width: parent.width / 5
                        height: parent.height

                        AppIcon {
                            id: star1Icon
                            anchors.centerIn: parent
                            width: currentView >= 1 ? parent.width : 20 * virtualstudio.uiScale
                            height: currentView >= 1 ? parent.height : 20 * virtualstudio.uiScale
                            icon.source: "star.svg"
                            color: currentView >= 1 ? "#faaf00" : "#606060"
                        }

                        MouseArea {
                            id: star1MouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: () => {
                                if (rating === 1) {
                                    rating = 0;
                                } else {
                                    rating = 1;
                                }
                            }
                        }
                    }

                    Item {
                        id: star2Container
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: star3Container.left
                        width: parent.width / 5
                        height: parent.height

                        AppIcon {
                            id: star2Icon
                            anchors.centerIn: parent
                            width: currentView >= 2 ? parent.width : 20 * virtualstudio.uiScale
                            height: currentView >= 2 ? parent.height : 20 * virtualstudio.uiScale
                            icon.source: "star.svg"
                            color: currentView >= 2 ? "#faaf00" : "#606060"
                        }

                        MouseArea {
                            id: star2MouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: () => {
                                if (rating === 2) {
                                    rating = 0;
                                } else {
                                    rating = 2;
                                }
                            }
                        }
                    }

                    Item {
                        id: star3Container
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width / 5
                        height: parent.height

                        AppIcon {
                            id: star3Icon
                            anchors.centerIn: parent
                            width: currentView >= 3 ? parent.width : 20 * virtualstudio.uiScale
                            height: currentView >= 3 ? parent.height : 20 * virtualstudio.uiScale
                            icon.source: "star.svg"
                            color: currentView >= 3 ? "#faaf00" : "#606060"
                        }

                        MouseArea {
                            id: star3MouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: () => {
                                if (rating === 3) {
                                    rating = 0;
                                } else {
                                    rating = 3;
                                }
                            }
                        }
                    }

                    Item {
                        id: star4Container
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: star3Container.right
                        width: parent.width / 5
                        height: parent.height

                        AppIcon {
                            id: star4Icon
                            anchors.centerIn: parent
                            width: currentView >= 4 ? parent.width : 20 * virtualstudio.uiScale
                            height: currentView >= 4 ? parent.height : 20 * virtualstudio.uiScale
                            icon.source: "star.svg"
                            color: currentView >= 4 ? "#faaf00" : "#606060"
                        }

                        MouseArea {
                            id: star4MouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: () => {
                                if (rating === 4) {
                                    rating = 0;
                                } else {
                                    rating = 4;
                                }
                            }
                        }
                    }

                    Item {
                        id: star5Container
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: star4Container.right
                        width: parent.width / 5
                        height: parent.height

                        AppIcon {
                            id: star5Icon
                            anchors.centerIn: parent
                            width: currentView >= 5 ? parent.width : 20 * virtualstudio.uiScale
                            height: currentView >= 5 ? parent.height : 20 * virtualstudio.uiScale
                            icon.source: "star.svg"
                            color: currentView >= 5 ? "#faaf00" : "#606060"
                        }

                        MouseArea {
                            id: star5MouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: () => {
                                if (rating === 5) {
                                    rating = 0;
                                } else {
                                    rating = 5;
                                }
                            }
                        }
                    }
                }

                ScrollView {
                    id: messageBoxScrollArea
                    anchors.left: parent.left
                    anchors.leftMargin: 32 * virtualstudio.uiScale
                    anchors.right: parent.right
                    anchors.rightMargin: 32 * virtualstudio.uiScale
                    anchors.top: ratingItem.bottom
                    anchors.topMargin: 12 * virtualstudio.uiScale
                    height: 64 * virtualstudio.uiScale

                    TextArea {
                        id: messageBox
                        placeholderText: qsTr("(Optional) Let us know how we can improve your experience.")
                        placeholderTextColor: textAreaTextColour
                        color: textColour
                        background: Rectangle {
                        color: textAreaColour
                        radius: 6 * virtualstudio.uiScale
                        border.width: 1
                        border.color: buttonStroke
                        }
                    }
                }

                Item {
                    id: buttonsArea
                    height: 32 * virtualstudio.uiScale
                    width: 324 * virtualstudio.uiScale
                    anchors.horizontalCenter: messageBoxScrollArea.horizontalCenter
                    anchors.top: messageBoxScrollArea.bottom
                    anchors.topMargin: 24 * virtualstudio.uiScale

                    Button {
                        id: userFeedbackButton
                        anchors.right: buttonsArea.right
                        anchors.horizontalCenter: buttonsArea.horizontalCenter
                        anchors.verticalCenter: parent.buttonsArea
                        width: 150 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
                        onClicked: () => {
                            if (rating === 0 && messageBox.text === "") {
                                userFeedbackModal.close();
                                serverId = "";
                                messageBox.clear();
                                return;
                            }
                            virtualstudio.collectFeedbackSurvey(serverId, rating, messageBox.text);
                            submitted = true;
                            rating = 0;
                            serverId = "";
                            messageBox.clear();
                            userFeedbackModal.height = 150 * virtualstudio.uiScale
                            submittedFeedbackTimer.start();
                        }

                        background: Rectangle {
                            radius: 6 * virtualstudio.uiScale
                            color: userFeedbackButton.down ? buttonPressedColour : (userFeedbackButton.hovered ? buttonHoverColour : buttonColour)
                            border.width: 1
                            border.color: userFeedbackButton.down ? buttonPressedStroke : (userFeedbackButton.hovered ? buttonHoverStroke : buttonStroke)
                        }

                        Text {
                            text: (rating === 0 && messageBox.text === "") ? "Dismiss" : "Submit"
                            font.family: "Poppins"
                            font.pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale
                            font.weight: Font.Bold
                            color: "#DB0A0A"
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    Timer {
                        id: submittedFeedbackTimer
                        interval: 5000; running: false; repeat: false
                        onTriggered: () => {
                            submitted = false;
                            userFeedbackModal.height = 300 * virtualstudio.uiScale
                            userFeedbackModal.close();
                        }
                    }
                }
            }

            Item {
                id: submittedFeedbackContent
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                visible: submitted

                Text {
                    id: submittedFeedbackHeader
                    anchors.top: submittedFeedbackContent.top
                    anchors.topMargin: 24 * virtualstudio.uiScale
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width
                    text: "Thank you!"
                    font {family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale; bold: true }
                    horizontalAlignment: Text.AlignHCenter
                    color: textColour
                    elide: Text.ElideRight
                    wrapMode: Text.WordWrap
                }

                Text {
                    id: submittedFeedbackText
                    anchors.top: submittedFeedbackHeader.bottom
                    anchors.topMargin: 16 * virtualstudio.uiScale
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width
                    text: "Your feedback has been recorded."
                    font {family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                    horizontalAlignment: Text.AlignHCenter
                    color: textColour
                    elide: Text.ElideRight
                    wrapMode: Text.WordWrap
                }

                Button {
                    id: closeButtonFeedback
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: submittedFeedbackText.bottom
                    anchors.topMargin: 16 * virtualstudio.uiScale
                    width: 150 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
                    onClicked: () => {
                        submitted = false;
                        userFeedbackModal.height = 300 * virtualstudio.uiScale
                        userFeedbackModal.close();
                    }

                    background: Rectangle {
                        radius: 6 * virtualstudio.uiScale
                        color: closeButtonFeedback.down ? buttonPressedColour : (closeButtonFeedback.hovered ? buttonHoverColour : buttonColour)
                        border.width: 1
                        border.color: closeButtonFeedback.down ? buttonPressedStroke : (closeButtonFeedback.hovered ? buttonHoverStroke : buttonStroke)
                    }

                    Text {
                        text: "Close"
                        font.family: "Poppins"
                        font.pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }
        }
    }

    Connections {
      target: virtualstudio

      function onOpenFeedbackSurveyModal(serverId) {
        userFeedbackSurvey.serverId = serverId;
        userFeedbackModal.open();
      }
    }
}
