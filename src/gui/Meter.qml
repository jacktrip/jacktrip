import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.12

Item {
    required property var model

    property int min: -80
    property int max: 0
    property int bins: 15

    property int innerMargin: 2 * virtualstudio.uiScale
    property int clipWidth: 10 * virtualstudio.uiScale
    required property bool clipped

    property string vuMeterColor: virtualstudio.darkMode ? "#5B5858" : "#D3D4D4"

    property string vuMeterGreen: "#61C554"
    property string vuMeterYellow: "#F5BF4F"
    property string vuMeterRed: "#F21B1B"

    function getBoxColor (idx, level) {

        // Case where the meter should be filled
        if (level > ((max - min) / bins) * idx + min) {
            let fillColor = vuMeterGreen;
            if (idx > 8 && idx <= 11) {
                fillColor = vuMeterYellow;
            } else if (idx > 11) {
                fillColor = vuMeterRed;
            }
            return fillColor;

        // Case where the meter should not be filled
        } else {
            return vuMeterColor
        }
    }

    ListView {
        id: meters
        x: 0; y: 0
        width: parent.width - clipWidth
        height: parent.height
        model: parent.model

        delegate: Item {
            x: 0;
            width: parent.width
            height: 14 * virtualstudio.uiScale
            required property double modelData

            property int boxHeight: 10 * virtualstudio.uiScale
            property int boxWidth: (width / bins) - innerMargin
            property int boxRadius: 4 * virtualstudio.uiScale

            Rectangle {
                id: box0
                x: 0;
                y: 0;
                width: boxWidth
                height: boxHeight
                color: getBoxColor(0, parent.modelData)
                radius: boxRadius
            }

            Rectangle {
                id: box1
                x: boxWidth + innerMargin;
                y: 0;
                width: boxWidth
                height: boxHeight
                color: getBoxColor(1, parent.modelData)
                radius: boxRadius
            }

            Rectangle {
                id: box2
                x: (boxWidth) * 2 + innerMargin * 2;
                y: 0;
                width: boxWidth
                height: boxHeight
                color: getBoxColor(2, parent.modelData)
                radius: boxRadius
            }

            Rectangle {
                id: box3
                x: (boxWidth) * 3 + innerMargin * 3;
                y: 0;
                width: boxWidth
                height: boxHeight
                color: getBoxColor(3, parent.modelData)
                radius: boxRadius
            }

            Rectangle {
                id: box4
                x: (boxWidth) * 4 + innerMargin * 4;
                y: 0;
                width: boxWidth
                height: boxHeight
                color: getBoxColor(4, parent.modelData)
                radius: boxRadius
            }

            Rectangle {
                id: box5
                x: (boxWidth) * 5 + innerMargin * 5;
                y: 0;
                width: boxWidth
                height: boxHeight
                color: getBoxColor(5, parent.modelData)
                radius: boxRadius
            }

            Rectangle {
                id: box6
                x: (boxWidth) * 6 + innerMargin * 6;
                y: 0;
                width: boxWidth
                height: boxHeight
                color: getBoxColor(6, parent.modelData)
                radius: boxRadius
            }

            Rectangle {
                id: box7
                x: (boxWidth) * 7 + innerMargin * 7;
                y: 0;
                width: boxWidth
                height: boxHeight
                color: getBoxColor(7, parent.modelData)
                radius: boxRadius
            }

            Rectangle {
                id: box8
                x: (boxWidth) * 8 + innerMargin * 8;
                y: 0;
                width: boxWidth
                height: boxHeight
                color: getBoxColor(8, parent.modelData)
                radius: boxRadius
            }

            Rectangle {
                id: box9
                x: (boxWidth) * 9 + innerMargin * 9;
                y: 0;
                width: boxWidth
                height: boxHeight
                color: getBoxColor(9, parent.modelData)
                radius: boxRadius
            }

            Rectangle {
                id: box10
                x: (boxWidth) * 10 + innerMargin * 10;
                y: 0;
                width: boxWidth
                height: boxHeight
                color: getBoxColor(10, parent.modelData)
                radius: boxRadius
            }

            Rectangle {
                id: box11
                x: (boxWidth) * 11 + innerMargin * 11;
                y: 0;
                width: boxWidth
                height: boxHeight
                color: getBoxColor(11, parent.modelData)
                radius: boxRadius
            }

            Rectangle {
                id: box12
                x: (boxWidth) * 12 + innerMargin * 12;
                y: 0;
                width: boxWidth
                height: boxHeight
                color: getBoxColor(12, parent.modelData)
                radius: boxRadius
            }

            Rectangle {
                id: box13
                x: (boxWidth) * 13 + innerMargin * 13;
                y: 0;
                width: boxWidth
                height: boxHeight
                color: getBoxColor(13, parent.modelData)
                radius: boxRadius
            }

            Rectangle {
                id: box14
                x: (boxWidth) * 14 + innerMargin * 14;
                y: 0;
                width: boxWidth
                height: boxHeight
                color: getBoxColor(14, parent.modelData)
                radius: boxRadius
            }
        }
    }

    Rectangle {
        id: clipIndicator
        x: meters.x + meters.width; y: 0
        width: Math.min(clipWidth, ((parent.width - clipWidth) / bins) - innerMargin)
        height: 24 * virtualstudio.uiScale
        radius: 4 * virtualstudio.uiScale
        color: clipped ? vuMeterRed : vuMeterColor
    }
}