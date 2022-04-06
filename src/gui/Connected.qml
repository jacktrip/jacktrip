import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    width: 696; height: 577
    
    property bool connecting: false
    
    property int leftMargin: 16
    property int fontBig: 28
    property int fontMedium: 18

    Image {
        x: parent.width - 49; y: 16; width: 32; height: 59
        source: "logo.svg"
    }
    
    Text {
        id: heading
        text: virtualstudio.connectionState
        x: leftMargin; y: 34
        font { family: "Poppins"; weight: Font.Bold; pointSize: fontBig * virtualstudio.fontScale }
    }
    
    Studio {
        x: parent.leftMargin; y: 96
        connected: true
        serverLocation: serverModel[virtualstudio.currentStudio].location
        flagImage: serverModel[virtualstudio.currentStudio].flag
        studioName: serverModel[virtualstudio.currentStudio].name
        publicStudio: serverModel[virtualstudio.currentStudio].isPublic
        manageable: serverModel[virtualstudio.currentStudio].isManageable
        available: serverModel[virtualstudio.currentStudio].canConnect
    }
    
    Image {
        id: mic
        source: "mic.svg"
        x: 80; y: 250; width: 18; height: 28
    }
    
    Image {
        id: headphones
        source: "headphones.svg"
        anchors.horizontalCenter: mic.horizontalCenter
        y: 329; width: 24; height: 26
    }
    
    Text {
        x: 120
        text: virtualstudio.audioBackend == "JACK" ? 
            virtualstudio.audioBackend : inputComboModel[virtualstudio.inputDevice]
        font {family: "Poppins"; pointSize: fontMedium * virtualstudio.fontScale }
        anchors.verticalCenter: mic.verticalCenter
    }
    
    Text {
        x: 120
        text: virtualstudio.audioBackend == "JACK" ? 
            virtualstudio.audioBackend : outputComboModel[virtualstudio.outputDevice]
        font {family: "Poppins"; pointSize: fontMedium * virtualstudio.fontScale }
        anchors.verticalCenter: headphones.verticalCenter
    }
    
    //43 822
}
