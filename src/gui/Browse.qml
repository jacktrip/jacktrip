import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    width: 696; height: 577

    Component {
        id: sectionHeading
        Rectangle {
            color: "#FAFBFB"
            height: 72; width: ListView.view.width
            required property string section
            Text {
                //anchors.bottom: parent.bottom
                y: 12
                text: parent.section
                font.family: "Poppins"
                font.pointSize: 28
                font.weight: Font.Bold
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
        }
    }

    ListView {
        x:16; y: 0; width: 664; height: parent.height - 36
        spacing: 16
        header: footer
        footer: footer
        model: serverModel
        clip: true
        delegate: Studio {
            serverLocation: location
            flagImage: flag
            studioName: name
            publicStudio: isPublic
            manageable: isManageable
            connected: false
        }
        
        section.property: "type"
        section.criteria: ViewSection.FullString
        section.delegate: sectionHeading
    }
    
    Rectangle {
        x: 0; y: parent.height - 36; width: parent.width; height: 36
        border.color: "#33979797"
        
        Button {
            id: aboutButton
            background: Rectangle {
                radius: 6
                color: aboutButton.down ? "#4E979797" : (aboutButton.hovered ? "#34979797" : "#1A979797")
                border.width: 0.3
                border.color: "#34979797"
            }
            onClicked: { virtualstudio.showAbout() }
            anchors.verticalCenter: parent.verticalCenter
            x: 466; width: 103; height: 25
            Text {
                text: "About"
                font.family: "Poppins"
                font.pointSize: 11
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }
        }
        
        Button {
            id: settingsButton
            background: Rectangle {
                radius: 6
                color: settingsButton.down ? "#4E979797" : (settingsButton.hovered ? "#34979797" : "#1A979797")
                border.width: 0.3
                border.color: "#34979797"
            }
            onClicked: window.state = "settings"
            anchors.verticalCenter: parent.verticalCenter
            x: 577; width: 103; height: 25
            Text {
                text: "Settings"
                font.family: "Poppins"
                font.pointSize: 11
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }
}
