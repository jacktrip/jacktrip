import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.12

Item {
    required property var level
    required property var enabled
    property int bins: 15
    property int innerMargin: 2 * virtualstudio.uiScale
    property int boxHeight: 10 * virtualstudio.uiScale
    property int boxWidth: (width / bins) - innerMargin
    property int boxRadius: 4 * virtualstudio.uiScale
    property string meterColor: enabled ? (virtualstudio.darkMode ? "#5B5858" : "#D3D4D4") : "#EAECEC"
    property string meterGreen: "#61C554"
    property string meterYellow: "#F5BF4F"
    property string meterRed: "#F21B1B"

    function getBoxColor (idx) {
        // Case where the meter should not be filled
        if (!enabled || level <= (idx / bins)) {
            return meterColor;
        }
        // Case where the meter should be filled
        let fillColor = meterGreen;
        if (idx > 8 && idx <= 11) {
            fillColor = meterYellow;
        } else if (idx > 11) {
            fillColor = meterRed;
        }
        return fillColor;
    }

    Rectangle {
        id: box0
        x: 0;
        y: 0;
        width: boxWidth
        height: boxHeight
        color: getBoxColor(0)
        radius: boxRadius
    }

    Rectangle {
        id: box1
        x: boxWidth + innerMargin;
        y: 0;
        width: boxWidth
        height: boxHeight
        color: getBoxColor(1)
        radius: boxRadius
    }

    Rectangle {
        id: box2
        x: (boxWidth) * 2 + innerMargin * 2;
        y: 0;
        width: boxWidth
        height: boxHeight
        color: getBoxColor(2)
        radius: boxRadius
    }

    Rectangle {
        id: box3
        x: (boxWidth) * 3 + innerMargin * 3;
        y: 0;
        width: boxWidth
        height: boxHeight
        color: getBoxColor(3)
        radius: boxRadius
    }

    Rectangle {
        id: box4
        x: (boxWidth) * 4 + innerMargin * 4;
        y: 0;
        width: boxWidth
        height: boxHeight
        color: getBoxColor(4)
        radius: boxRadius
    }

    Rectangle {
        id: box5
        x: (boxWidth) * 5 + innerMargin * 5;
        y: 0;
        width: boxWidth
        height: boxHeight
        color: getBoxColor(5)
        radius: boxRadius
    }

    Rectangle {
        id: box6
        x: (boxWidth) * 6 + innerMargin * 6;
        y: 0;
        width: boxWidth
        height: boxHeight
        color: getBoxColor(6)
        radius: boxRadius
    }

    Rectangle {
        id: box7
        x: (boxWidth) * 7 + innerMargin * 7;
        y: 0;
        width: boxWidth
        height: boxHeight
        color: getBoxColor(7)
        radius: boxRadius
    }

    Rectangle {
        id: box8
        x: (boxWidth) * 8 + innerMargin * 8;
        y: 0;
        width: boxWidth
        height: boxHeight
        color: getBoxColor(8)
        radius: boxRadius
    }

    Rectangle {
        id: box9
        x: (boxWidth) * 9 + innerMargin * 9;
        y: 0;
        width: boxWidth
        height: boxHeight
        color: getBoxColor(9)
        radius: boxRadius
    }

    Rectangle {
        id: box10
        x: (boxWidth) * 10 + innerMargin * 10;
        y: 0;
        width: boxWidth
        height: boxHeight
        color: getBoxColor(10)
        radius: boxRadius
    }

    Rectangle {
        id: box11
        x: (boxWidth) * 11 + innerMargin * 11;
        y: 0;
        width: boxWidth
        height: boxHeight
        color: getBoxColor(11)
        radius: boxRadius
    }

    Rectangle {
        id: box12
        x: (boxWidth) * 12 + innerMargin * 12;
        y: 0;
        width: boxWidth
        height: boxHeight
        color: getBoxColor(12)
        radius: boxRadius
    }

    Rectangle {
        id: box13
        x: (boxWidth) * 13 + innerMargin * 13;
        y: 0;
        width: boxWidth
        height: boxHeight
        color: getBoxColor(13)
        radius: boxRadius
    }

    Rectangle {
        id: box14
        x: (boxWidth) * 14 + innerMargin * 14;
        y: 0;
        width: boxWidth
        height: boxHeight
        color: getBoxColor(14)
        radius: boxRadius
    }
}