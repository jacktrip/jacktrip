import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: footer
    width: parent.width
    height: 24
    anchors.bottom: parent.bottom
    color: backgroundColour
    clip: true

    property string connectionStateColor: getConnectionStateColor()
    property variant networkStatsText: getNetworkStatsText()

    function getConnectionStateColor() {
        if (virtualstudio.connectionState == "Connected") {
            return meterGreen
        }
        if (virtualstudio.connectionState.includes("Disconnected") || virtualstudio.connectionState.includes("Error")) {
            return meterRed
        }
        if (studioStatus === "Starting" || virtualstudio.connectionState == "Connecting..." || virtualstudio.connectionState == "Reconnecting...") {
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

        texts[1] = "<b>" + minRtt + " ms - " + maxRtt + " ms</b>, avg " + avgRtt + " ms";
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

    MouseArea {
        anchors.fill: parent
        propagateComposedEvents: false
    }

    RowLayout {
        id: layout
        anchors.fill: parent
        spacing: 4

        Rectangle {
            color: backgroundColour
            Layout.minimumWidth: 256
            Layout.preferredWidth: 512
            Layout.maximumWidth: 640
            Layout.fillHeight: true
            Layout.fillWidth: true
            visible: studioStatus === "Ready"

            AppIcon {
                id: connectionQualityIcon
                anchors.left: parent.left
                anchors.leftMargin: 8 * virtualstudio.uiScale
                anchors.verticalCenter: parent.verticalCenter
                width: 20 * virtualstudio.uiScale
                height: 20 * virtualstudio.uiScale
                icon.source: "speed.svg"
            }

            Text {
                id: connectionQualityText
                anchors.left: connectionQualityIcon.right
                anchors.leftMargin: 4 * virtualstudio.uiScale
                anchors.verticalCenter: parent.verticalCenter
                text: "Connection:"
                font { family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
                color: textColour
            }

            Text {
                id: connectionQualityName
                anchors.left: connectionQualityText.right
                anchors.leftMargin: 2 * virtualstudio.uiScale
                anchors.verticalCenter: parent.verticalCenter
                text: networkStatsText[0]
                font { family: "Poppins"; weight: Font.Bold; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
                color: networkStatsText[2]
            }

            Text {
                id: connectionQualityTime
                anchors.left: connectionQualityName.right
                anchors.leftMargin: 8 * virtualstudio.uiScale
                anchors.verticalCenter: parent.verticalCenter
                text: networkStatsText[1]
                font { family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
                color: textColour
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        Rectangle {
            color: backgroundColour
            Layout.minimumWidth: 96
            Layout.preferredWidth: 128
            Layout.maximumWidth: 160
            Layout.fillHeight: true

            Rectangle {
                id: connectionStatusDot
                anchors.right: connectionStatusText.left
                anchors.rightMargin: 4 * virtualstudio.uiScale
                anchors.verticalCenter: parent.verticalCenter
                width: 12
                height: connectionStatusDot.width
                radius: connectionStatusDot.height / 2
                color: connectionStateColor
            }

            Text {
                id: connectionStatusText
                anchors.right: parent.right
                anchors.rightMargin: 8 * virtualstudio.uiScale
                anchors.verticalCenter: parent.verticalCenter
                text: studioStatus === "Starting" ? "Starting..." : virtualstudio.connectionState
                font { family: "Poppins"; pixelSize: fontTiny * virtualstudio.fontScale * virtualstudio.uiScale }
                color: textColour
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

    Connections {
        target: virtualstudio

        function onConnectionStateChanged() {
            connectionStatusDot.color = getConnectionStateColor()
        }
        function onNetworkStatsChanged() {
            networkStatsText = getNetworkStatsText();
        }
        function onUpdatedNetworkOutage() {
            networkStatsText = getNetworkStatsText();
        }
    }
}
