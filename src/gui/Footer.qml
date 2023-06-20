import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.12

Item {
    width: parent.width
    height: 20
    anchors.bottom: parent.bottom
    clip: true

    property string strokeColor: virtualstudio.darkMode ? "#80827D7D" : "#34979797"

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

    function getNetworkStatsText () {
        let minRtt = virtualstudio.networkStats.minRtt;
        let maxRtt = virtualstudio.networkStats.maxRtt;
        let avgRtt = virtualstudio.networkStats.avgRtt;

        let texts = ["Unstable", "Please plug into Ethernet & turn off WIFI."];

        if (virtualstudio.networkOutage) {
            return texts;
        }

        texts = ["Measuring...", ""];

        if (!minRtt || !maxRtt) {
            return texts;
        }

        texts[1] = "<b>" + minRtt + " ms - " + maxRtt + " ms</b>, avg " + avgRtt + " ms round-trip time";

        let quality = "Poor";
        if (avgRtt <= 25) {
            if (maxRtt <= 30) {
                quality = "Excellent";
            } else {
                quality = "Good";
            }
        } else if (avgRtt <= 30) {
            quality = "Good";
        } else if (avgRtt <= 35) {
            quality = "Fair";
        }

        texts[0] = quality
        return texts;
    }

    RowLayout {
        id: layout
        anchors.fill: parent
        spacing: 6

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
                    font.family: "Poppins"
                    font.pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale
                    color: textColour
                }
            }
        }

        Item {
            Layout.fillWidth: true
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
                id: connectionQualityText
                anchors.right: parent.right
                anchors.rightMargin: 8 * virtualstudio.uiScale
                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment: Text.AlignRight
                text: getNetworkStatsText()[1] ? "Connection Quality: <b>" + getNetworkStatsText()[0] + "</b> | " + getNetworkStatsText()[1] : "Connection Quality: <b>" + getNetworkStatsText()[0] + "</b>"
                font {family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
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
        y: parent.height - 20
        color: strokeColor
    }
}
