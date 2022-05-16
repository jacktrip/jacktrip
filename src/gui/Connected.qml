import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.12

Item {
    width: parent.width; height: parent.height
    clip: true
    
    property bool connecting: false
    property bool devicesChanged: false
    
    property int leftMargin: 16
    property int fontBig: 28
    property int fontMedium: 18

    Image {
        x: parent.width - (49 * virtualstudio.uiScale); y: 16 * virtualstudio.uiScale
        width: 32 * virtualstudio.uiScale; height: 59 * virtualstudio.uiScale
        source: "logo.svg"
    }
    
    Text {
        id: heading
        text: virtualstudio.connectionState
        x: leftMargin * virtualstudio.uiScale; y: 34 * virtualstudio.uiScale
        font { family: "Poppins"; weight: Font.Bold; pixelSize: fontBig * virtualstudio.fontScale * virtualstudio.uiScale }
    }
    
    Studio {
        x: parent.leftMargin * virtualstudio.uiScale; y: 96 * virtualstudio.uiScale
        width: parent.width - (2 * x)
        connected: true
        serverLocation: virtualstudio.currentStudio >= 0 ? serverModel[virtualstudio.currentStudio].location : "Germany - Berlin"
        flagImage: virtualstudio.currentStudio >= 0 ? serverModel[virtualstudio.currentStudio].flag : "flags/DE.svg"
        studioName: virtualstudio.currentStudio >= 0 ? serverModel[virtualstudio.currentStudio].name : "Test Studio"
        publicStudio: virtualstudio.currentStudio >= 0 ? serverModel[virtualstudio.currentStudio].isPublic : false
        manageable: virtualstudio.currentStudio >= 0 ? serverModel[virtualstudio.currentStudio].isManageable : false
        available: virtualstudio.currentStudio >= 0 ? serverModel[virtualstudio.currentStudio].canConnect : false
    }
    
    Image {
        id: mic
        source: "mic.svg"
        x: 80 * virtualstudio.uiScale; y: 250 * virtualstudio.uiScale
        width: 18 * virtualstudio.uiScale; height: 28 * virtualstudio.uiScale
    }
    
    Image {
        id: headphones
        source: "headphones.svg"
        anchors.horizontalCenter: mic.horizontalCenter
        y: 329 * virtualstudio.uiScale
        width: 24 * virtualstudio.uiScale; height: 26 * virtualstudio.uiScale
    }
    
    Text {
        x: 120 * virtualstudio.uiScale
        text: virtualstudio.audioBackend == "JACK" ? 
            virtualstudio.audioBackend : inputComboModel[virtualstudio.inputDevice]
        font {family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
        anchors.verticalCenter: mic.verticalCenter
        visible: virtualstudio.audioBackend == "JACK"
    }
    
    Text {
        x: 120 * virtualstudio.uiScale
        text: virtualstudio.audioBackend == "JACK" ? 
            virtualstudio.audioBackend : outputComboModel[virtualstudio.outputDevice]
        font {family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
        anchors.verticalCenter: headphones.verticalCenter
        visible: virtualstudio.audioBackend == "JACK"
    }

    ComboBox {
        id: inputCombo
        model: inputComboModel
        currentIndex: virtualstudio.inputDevice
        onActivated: { virtualstudio.inputDevice = currentIndex; devicesChanged = true }
        x: 120 * virtualstudio.uiScale
        anchors.verticalCenter: mic.verticalCenter
        width: parent.width - 2 * x - (16 * virtualstudio.uiScale); height: 36 * virtualstudio.uiScale
        visible: virtualstudio.audioBackend != "JACK"
    }
    
    ComboBox {
        id: outputCombo
        model: outputComboModel
        currentIndex: virtualstudio.outputDevice
        onActivated: { virtualstudio.outputDevice = currentIndex; devicesChanged = true }
        x: 120 * virtualstudio.uiScale
        anchors.verticalCenter: headphones.verticalCenter
        width: parent.width - 2 * x - (16 * virtualstudio.uiScale); height: 36 * virtualstudio.uiScale
        visible: virtualstudio.audioBackend != "JACK"
    }

    Button {
        id: saveButton
        background: Rectangle {
            radius: 6 * virtualstudio.uiScale
            color: saveButton.down ? "#E7E8E8" : "#F2F3F3"
            border.width: 1
            border.color: saveButton.down ? "#B0B5B5" : "#EAEBEB"
            layer.enabled: saveButton.hovered && !saveButton.down
            layer.effect: DropShadow {
                horizontalOffset: 1 * virtualstudio.uiScale
                verticalOffset: 1 * virtualstudio.uiScale
                radius: 8.0 * virtualstudio.uiScale
                samples: 17
                color: "#80A1A1A1"
            }
        }
        onClicked: { devicesChanged = false }
        anchors.right: outputCombo.right
        anchors.topMargin: 16
        anchors.top: outputCombo.bottom
        width: 150 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
        Text {
            text: "Save Changes"
            font.family: "Poppins"
            font.pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale
            font.weight: Font.Bold
            color: "#DB0A0A"
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
        }
        visible: devicesChanged == true
    }

    Text {
        anchors.left: outputCombo.left
        anchors.right: saveButton.left
        anchors.rightMargin: 16
        anchors.verticalCenter: saveButton.verticalCenter
        text: "Saving will reconnect you to this Studio."
        color: "#DB0A0A"
        font {family: "Poppins"; pixelSize: 10 * virtualstudio.fontScale * virtualstudio.uiScale }
        visible: devicesChanged == true
        wrapMode: Text.WordWrap
    }
    
    //43 822
}
