import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.12

Item {
    width: parent.width; height: parent.height
    clip: true
    
    property bool connecting: false
    
    property int leftHeaderMargin: 16
    property int fontBig: 28
    property int fontMedium: 14
    property int fontSmall: 10
    property int fontTiny: 8

    property int bodyMargin: 60
    
    property string textColour: virtualstudio.darkMode ? "#FAFBFB" : "#0F0D0D"
    property string vuMeterColor: virtualstudio.darkMode ? "gray" : "#E0E0E0"
    property real imageLightnessValue: virtualstudio.darkMode ? 1.0 : 0.0

    property string vuMeterGreen: "#61C554"
    property string vuMeterYellow: "#F5BF4F"
    property string vuMeterRed: "#F21B1B"

    function getNetworkStatsText (networkStats) {
        let packetsSent = networkStats.packetsSent;
        let packetsReceived = networkStats.packetsReceived;
        let minRtt = networkStats.minRtt;
        let maxRtt = networkStats.maxRtt;
        let avgRtt = networkStats.avgRtt;

        let texts = ["Measuring stats ...", "", ""];

        if (!minRtt || !maxRtt) {
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

    Image {
        x: parent.width - (49 * virtualstudio.uiScale); y: 16 * virtualstudio.uiScale
        width: 32 * virtualstudio.uiScale; height: 59 * virtualstudio.uiScale
        source: "logo.svg"
    }
    
    Text {
        id: heading
        text: virtualstudio.connectionState
        x: leftHeaderMargin * virtualstudio.uiScale; y: 34 * virtualstudio.uiScale
        font { family: "Poppins"; weight: Font.Bold; pixelSize: fontBig * virtualstudio.fontScale * virtualstudio.uiScale }
        color: textColour
    }
    
    Studio {
        x: leftHeaderMargin * virtualstudio.uiScale; y: 96 * virtualstudio.uiScale
        width: parent.width - (2 * x)
        connected: true
        serverLocation: virtualstudio.currentStudio >= 0 && virtualstudio.regions[serverModel[virtualstudio.currentStudio].location] ? "in " + virtualstudio.regions[serverModel[virtualstudio.currentStudio].location].label : ""
        flagImage: virtualstudio.currentStudio >= 0 ? ( serverModel[virtualstudio.currentStudio].bannerURL ? serverModel[virtualstudio.currentStudio].bannerURL : serverModel[virtualstudio.currentStudio].flag ) : "flags/DE.svg"
        studioName: virtualstudio.currentStudio >= 0 ? serverModel[virtualstudio.currentStudio].name : "Test Studio"
        publicStudio: virtualstudio.currentStudio >= 0 ? serverModel[virtualstudio.currentStudio].isPublic : false
        manageable: virtualstudio.currentStudio >= 0 ? serverModel[virtualstudio.currentStudio].isManageable : false
        available: virtualstudio.currentStudio >= 0 ? serverModel[virtualstudio.currentStudio].canConnect : false
    }

    Item {
        id: inputDevice
        x: bodyMargin * virtualstudio.uiScale; y: 250 * virtualstudio.uiScale
        width: parent.width / 2 - x
        height: 100 * virtualstudio.uiScale
        clip: true

        Image {
            id: mic
            source: "mic.svg"
            x: 0; y: 0
            width: 18 * virtualstudio.uiScale; height: 28 * virtualstudio.uiScale
        }

        Colorize {
            anchors.fill: mic
            source: mic
            hue: 0
            saturation: 0
            lightness: imageLightnessValue
        }

        Text {
            id: inputDeviceHeader
            x: 64 * virtualstudio.uiScale
            width: parent.width - 64 * virtualstudio.uiScale
            text: "Input Device"
            font {family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            anchors.verticalCenter: mic.verticalCenter
            color: textColour
            elide: Text.ElideRight
        }

        Text {
            id: inputDeviceName
            width: parent.width - 100 * virtualstudio.uiScale
            anchors.top: inputDeviceHeader.bottom
            anchors.left: inputDeviceHeader.left
            text: virtualstudio.audioBackend == "JACK" ?
                virtualstudio.audioBackend : inputComboModel[virtualstudio.inputDevice]
            font {family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            elide: Text.ElideRight
        }
    }

    Item {
        id: inputDeviceVuMeters
        x: parent.width / 2; y: 250 * virtualstudio.uiScale
        width: parent.width / 2 - bodyMargin * virtualstudio.uiScale
        height: 100 * virtualstudio.uiScale

        ListView {
            x: 0; y: 0
            width: parent.width
            height: parent.height
            model: inputVuMeterModel

            delegate: Item {
                x: 0
                width: parent.width
                height: 24 * virtualstudio.uiScale
                required property double modelData

                Rectangle {
                    id: meterBox
                    x: 0; y:0; z:2
                    width: parent.width
                    height: 20 * virtualstudio.uiScale
                    color: "transparent"
                    border.color: "black"
                    border.width: 1
                    opacity: 1
                }

                Rectangle {
                    id: meterFill
                    x: 0; y: 0; z:0
                    width: parent.width
                    height: 20 * virtualstudio.uiScale
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        
                        GradientStop { position: 0.0; color: vuMeterGreen }
                        GradientStop { position: 0.4; color: vuMeterGreen }
                        GradientStop { position: 0.8; color: vuMeterYellow }
                        GradientStop { position: 1.0; color: vuMeterRed }
                    }

                }

                Rectangle {
                    id: meterBase
                    x: 0; z: 1
                    anchors.right: meterFill.right
                    width: parent.width - parent.width * (parent.modelData + 80) / 80
                    height: 20 * virtualstudio.uiScale
                    color: vuMeterColor
                }
            }
        }
    }

    Item {
        id: outputDeviceVuMeters
        x: parent.width / 2; y: 330 * virtualstudio.uiScale
        width: parent.width / 2 - bodyMargin * virtualstudio.uiScale
        height: 100 * virtualstudio.uiScale
        
        ListView {
            x: 0; y: 0
            width: parent.width
            height: parent.height
            model: outputVuMeterModel
            
            delegate: Item {
                x: 0
                width: parent.width
                height: 24 * virtualstudio.uiScale
                required property double modelData

                Rectangle {
                    id: meterBox
                    x: 0; y:0; z:2
                    width: parent.width
                    height: 20 * virtualstudio.uiScale
                    color: "transparent"
                    border.color: "black"
                    border.width: 1
                    opacity: 1
                }

                Rectangle {
                    id: meterFill
                    x: 0; y: 0; z:0
                    width: parent.width
                    height: 20 * virtualstudio.uiScale
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        
                        GradientStop { position: 0.0; color: vuMeterGreen }
                        GradientStop { position: 0.4; color: vuMeterGreen }
                        GradientStop { position: 0.8; color: vuMeterYellow }
                        GradientStop { position: 1.0; color: vuMeterRed }
                    }
                }

                Rectangle {
                    id: meterBase
                    x: 0; z: 1
                    anchors.right: meterFill.right
                    width: parent.width - parent.width * (parent.modelData + 80) / 80
                    height: 20 * virtualstudio.uiScale
                    color: vuMeterColor
                }
            }
        }
    }

    Item {
        id: outputDevice
        x: bodyMargin * virtualstudio.uiScale; y: 330 * virtualstudio.uiScale
        width: parent.width / 2 - x
        height: 100 * virtualstudio.uiScale
        clip: true

        Image {
            id: headphones
            source: "headphones.svg"
            x: 0; y: 0
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
            id: outputDeviceHeader
            x: 64 * virtualstudio.uiScale
            width: parent.width - 64 * virtualstudio.uiScale
            text: "Output Device"
            font {family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            anchors.verticalCenter: headphones.verticalCenter
            color: textColour
            elide: Text.ElideRight
        }

        Text {
            id: outputDeviceName
            width: parent.width - 100 * virtualstudio.uiScale
            anchors.top: outputDeviceHeader.bottom
            anchors.left: outputDeviceHeader.left
            text: virtualstudio.audioBackend == "JACK" ?
                virtualstudio.audioBackend : outputComboModel[virtualstudio.outputDevice]
            font {family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            elide: Text.ElideRight
        }
    }

    Item {
        id: networkStats
        x: bodyMargin * virtualstudio.uiScale; y: 410 * virtualstudio.uiScale
        width: parent.width - 2 * x
        height: 128

        Image {
            id: network
            source: "network.svg"
            x: 0; y: 0
            width: 28 * virtualstudio.uiScale; height: 28 * virtualstudio.uiScale
        }

        Colorize {
            anchors.fill: network
            source: network
            hue: 0
            saturation: 0
            lightness: imageLightnessValue
        }

        Text {
            id: networkStatsHeader
            text: "Network"
            font {family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            x: 64 * virtualstudio.uiScale
            anchors.verticalCenter: network.verticalCenter
            color: textColour
        }

        Text {
            id: netstat0
            x: parent.width / 2
            text: getNetworkStatsText(virtualstudio.networkStats)[0]
            font {family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
            topPadding: 8 * virtualstudio.uiScale
            anchors.verticalCenter: network.verticalCenter
            color: textColour
        }

        Text {
            id: netstat1
            x: parent.width / 2
            text: getNetworkStatsText(virtualstudio.networkStats)[1]
            font {family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
            topPadding: 8 * virtualstudio.uiScale
            anchors.top: netstat0.bottom
            color: textColour
        }

        Text {
            id: netstat2
            x: parent.width / 2
            text: getNetworkStatsText(virtualstudio.networkStats)[2]
            font {family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
            topPadding: 8 * virtualstudio.uiScale
            anchors.top: netstat1.bottom
            color: textColour
        }
    }
}
