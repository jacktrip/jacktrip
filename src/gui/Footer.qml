import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.12

Item {
    id: footer
    width: parent.width
    height: 24
    anchors.bottom: parent.bottom
    clip: true

    function getConnectionStateColor() {
        if (virtualstudio.connectionState == "Connected") {
            return meterGreen
        }
        if (virtualstudio.connectionState.includes("Disconnected") || virtualstudio.connectionState.includes("Error")) {
            return meterRed
        }
        if (virtualstudio.connectionState == "Connecting..." || virtualstudio.connectionState == "Reconnecting...") {
            return meterYellow
        }
        return "grey"
    }

    function getNetworkStatsText() {
        let minRtt = virtualstudio.networkStats.minRtt;
        let maxRtt = virtualstudio.networkStats.maxRtt;
        let avgRtt = virtualstudio.networkStats.avgRtt;

        let texts = ["Unstable", "Please plug into Ethernet & turn off WIFI.", meterRed];
        if (virtualstudio.networkOutage) {
            return texts;
        }

        texts = ["Measuring...", "", "grey"];
        if (!minRtt || !maxRtt) {
            return texts;
        }

        texts[1] = "<b>" + minRtt + " ms - " + maxRtt + " ms</b>, avg " + avgRtt + " ms round-trip time";
        let quality = "Poor";
        let color = meterRed;
        if (avgRtt <= 25) {
            if (maxRtt <= 30) {
                quality = "Excellent";
                color = meterGreen;
            } else {
                quality = "Good";
                color = meterGreen;
            }
        } else if (avgRtt <= 30) {
            quality = "Good";
            color = meterGreen;
        } else if (avgRtt <= 35) {
            quality = "Fair";
            color = meterYellow;
        }

        texts[0] = quality
        texts[2] = color;
        return texts;
    }

    RowLayout {
        id: layout
        anchors.fill: parent
        spacing: 4

        Rectangle {
            color: backgroundColour
            Layout.minimumWidth: 96
            Layout.preferredWidth: 128
            Layout.maximumWidth: 160
            Layout.fillHeight: true

            Item {
                id: connectionStatus
                anchors.verticalCenter: parent.verticalCenter

                Rectangle {
                    id: connectionStatusDot
                    anchors.left: parent.left
                    anchors.leftMargin: 8 * virtualstudio.uiScale
                    anchors.verticalCenter: parent.verticalCenter
                    width: 12
                    height: connectionStatusDot.width
                    radius: connectionStatusDot.height / 2
                    color: getConnectionStateColor()
                }

                Text {
                    id: connectionStatusText
                    anchors.left: connectionStatusDot.right
                    anchors.leftMargin: 4 * virtualstudio.uiScale
                    anchors.verticalCenter: parent.verticalCenter
                    text: virtualstudio.connectionState
                    font { family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
                    color: textColour
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        Rectangle {
            color: backgroundColour
            Layout.minimumWidth: 256
            Layout.preferredWidth: 512
            Layout.maximumWidth: 640
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignRight
            visible: studioStatus === "Ready"

            Text {
                id: connectionQualityTime
                anchors.right: parent.right
                anchors.rightMargin: 8 * virtualstudio.uiScale
                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment: Text.AlignRight
                text: getNetworkStatsText()[1]
                font { family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
                color: textColour
            }

            Text {
                id: connectionQualityName
                anchors.right: connectionQualityTime.left
                anchors.rightMargin: 8 * virtualstudio.uiScale
                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment: Text.AlignRight
                text: getNetworkStatsText()[0]
                font { family: "Poppins"; weight: Font.Bold; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
                color: getNetworkStatsText()[2]
            }

            Text {
                id: connectionQualityText
                anchors.right: connectionQualityName.left
                anchors.rightMargin: 2 * virtualstudio.uiScale
                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment: Text.AlignRight
                text: "Connection Quality:"
                font { family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
                color: textColour
            }

            Image {
                id: connectionQualityIcon
                anchors.right: connectionQualityText.left
                anchors.rightMargin: 2 * virtualstudio.uiScale
                anchors.verticalCenter: parent.verticalCenter
                source: "speed.svg"
                width: 16 * virtualstudio.uiScale; height: 16 * virtualstudio.uiScale
                sourceSize: Qt.size(connectionQualityIcon.width, connectionQualityIcon.height)
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            Colorize {
                anchors.fill: connectionQualityIcon
                source: connectionQualityIcon
                hue: 0
                saturation: 0
                lightness: imageLightnessValue
            }
        }
    }

    Rectangle {
        id: backgroundBorder
        width: parent.width
        height: 1
        y: parent.height - footer.height
        color: buttonStroke
    }
}
