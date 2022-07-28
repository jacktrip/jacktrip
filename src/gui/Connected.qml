import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.12

Item {
    width: parent.width; height: parent.height
    clip: true
    
    property bool connecting: false
    
    property int leftMargin: 16
    property int fontBig: 28
    property int fontMedium: 18

    property int smallTextPadding: 8
    
    property string textColour: virtualstudio.darkMode ? "#FAFBFB" : "#0F0D0D"
    property real imageLightnessValue: virtualstudio.darkMode ? 1.0 : 0.0

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
        serverLocation: virtualstudio.currentStudio >= 0 && virtualstudio.regions[serverModel[virtualstudio.currentStudio].location] ? "in " + virtualstudio.regions[serverModel[virtualstudio.currentStudio].location].label : ""
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

    Colorize {
        anchors.fill: mic
        source: mic
        hue: 0
        saturation: 0
        lightness: imageLightnessValue
    }
    
    Image {
        id: headphones
        source: "headphones.svg"
        anchors.horizontalCenter: mic.horizontalCenter
        y: 329 * virtualstudio.uiScale
        width: 24 * virtualstudio.uiScale; height: 26 * virtualstudio.uiScale
    }

    Image {
        id: network
        source: "network.svg"
        anchors.horizontalCenter: mic.horizontalCenter
        y: 408 * virtualstudio.uiScale
        width: 28 * virtualstudio.uiScale; height: 28 * virtualstudio.uiScale
    }

    Colorize {
        anchors.fill: headphones
        source: headphones
        hue: 0
        saturation: 0
        lightness: imageLightnessValue
    }
    
    Text {
        id: inputDeviceHeader
        x: 120 * virtualstudio.uiScale
        text: virtualstudio.audioBackend == "JACK" ? 
            virtualstudio.audioBackend : inputComboModel[virtualstudio.inputDevice]
        font {family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
        anchors.verticalCenter: mic.verticalCenter
        color: textColour
    }
    
    Text {
        id: outputDeviceHeader
        x: 120 * virtualstudio.uiScale
        text: virtualstudio.audioBackend == "JACK" ? 
            virtualstudio.audioBackend : outputComboModel[virtualstudio.outputDevice]
        font {family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
        anchors.verticalCenter: headphones.verticalCenter
        color: textColour
    }

    Text {
        id: networkStatsHeader
        x: 120 * virtualstudio.uiScale
        text: "Network Quality"
        font {family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
        anchors.verticalCenter: network.verticalCenter
        color: textColour
    }

    function getNetworkStatsText (networkStats) {
        let packetsSent = networkStats.packetsSent;
        let packetsReceived = networkStats.packetsReceived;
        let minRtt = networkStats.minRtt;
        let maxRtt = networkStats.maxRtt;
        let avgRtt = networkStats.avgRtt;

        let texts = ["Measuring stats ...", "", ""];

        if (minRtt === 0 || maxRtt === 0) {
            return texts;
        }

        texts[0] = "<b>" + minRtt + " ms - " + maxRtt + " ms</b>, avg " + avgRtt + " ms round-trip time";
        texts[1] = (100 - (packetsReceived * 100 / packetsSent)) + "% packet loss";

        let quality = "poor";
        if (avgRtt <= 25) {

            if (maxRtt <= 30 && packetsSent === packetsReceived) {
                quality = "excellent";
            } else {
                quality = "good";
            }

        } else if (avgRtt <= 30) {

            if (packetsSent === packetsReceived) {
                quality = "good";
            } else {
                quality = "fair";
            }

        } else if (avgRtt <= 35) {

            if (packetsSent === packetsReceived) {
                quality = "fair";
            } else {
                quality = "low";
            }

        }

        texts[2] = "Your connection quality is <b>" + quality + "</b>."
        return texts;
    }

    Text {
        id: netstat0
        text: getNetworkStatsText(virtualstudio.networkStats)[0]
        font {family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
        topPadding: smallTextPadding
        anchors.left: inputDeviceHeader.left
        anchors.top: networkStatsHeader.bottom
        color: textColour
    }

    Text {
        id: netstat1
        text: getNetworkStatsText(virtualstudio.networkStats)[1]
        font {family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
        topPadding: smallTextPadding
        anchors.left: inputDeviceHeader.left
        anchors.top: netstat0.bottom
        color: textColour
    }

    Text {
        id: netstat2
        text: getNetworkStatsText(virtualstudio.networkStats)[2]
        font {family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
        topPadding: smallTextPadding
        anchors.left: inputDeviceHeader.left
        anchors.top: netstat1.bottom
        color: textColour
    }
    //43 822
}
