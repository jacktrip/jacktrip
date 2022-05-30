import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    width: parent.width; height: parent.height
    clip: true
    
    property bool connecting: false
    
    property int leftMargin: 16
    property int fontBig: 28
    property int fontMedium: 18
    
    property string textColour: virtualstudio.darkMode ? "#FAFBFB" : "#0F0D0D"

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
        color: textColour
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
        color: textColour
    }
    
    Text {
        x: 120 * virtualstudio.uiScale
        text: virtualstudio.audioBackend == "JACK" ? 
            virtualstudio.audioBackend : outputComboModel[virtualstudio.outputDevice]
        font {family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
        anchors.verticalCenter: headphones.verticalCenter
        color: textColour
    }
    
    //43 822
}
