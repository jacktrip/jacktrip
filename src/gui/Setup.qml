import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.12

Item {
    width: parent.width; height: parent.height
    clip: true

    property int fontBig: 28
    property int fontMedium: 13
    property int fontSmall: 11
    property int fontExtraSmall: 8

    property int buttonWidth: 103
    property int buttonHeight: 25

    property int leftMargin: 48
    property int rightMargin: 16

    property string backgroundColour: virtualstudio.darkMode ? "#272525" : "#FAFBFB"
    property real imageLightnessValue: virtualstudio.darkMode ? 1.0 : 0.0
    property string textColour: virtualstudio.darkMode ? "#FAFBFB" : "#0F0D0D"
    property string buttonColour: virtualstudio.darkMode ? "#494646" : "#EAECEC"
    property string buttonHoverColour: virtualstudio.darkMode ? "#5B5858" : "#D3D4D4"
    property string buttonPressedColour: virtualstudio.darkMode ? "#524F4F" : "#DEE0E0"
    property string buttonStroke: virtualstudio.darkMode ? "#80827D7D" : "#34979797"
    property string buttonHoverStroke: virtualstudio.darkMode ? "#7B7777" : "#BABCBC"
    property string buttonPressedStroke: virtualstudio.darkMode ? "#827D7D" : "#BABCBC"
    property string sliderColour: virtualstudio.darkMode ? "#BABCBC" :  "#EAECEC"
    property string sliderPressedColour: virtualstudio.darkMode ? "#ACAFAF" : "#DEE0E0"
    property string saveButtonShadow: "#80A1A1A1"
    property string saveButtonBackgroundColour: "#F2F3F3"
    property string saveButtonPressedColour: "#E7E8E8"
    property string saveButtonStroke: "#EAEBEB"
    property string saveButtonPressedStroke: "#B0B5B5"
    property string warningText: "#DB0A0A"
    property string saveButtonText: "#DB0A0A"
    property string checkboxStroke: "#0062cc"
    property string checkboxPressedStroke: "#007AFF"

    property bool currShowWarnings: virtualstudio.showWarnings
    property string warningScreen: virtualstudio.showWarnings ? "ethernet" : "acknowledged"

    Item {
        id: ethernetWarningItem
        width: parent.width; height: parent.height
        visible: warningScreen == "ethernet"

        Image {
            id: ethernetWarningLogo
            source: "ethernet.png"
            width: 179
            height: 128
            y: 60
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Colorize {
            anchors.fill: ethernetWarningLogo
            source: ethernetWarningLogo
            hue: 0
            saturation: 0
            lightness: imageLightnessValue
        }

        Text {
            id: ethernetWarningHeader
            text: "Connect via Wired Ethernet"
            font { family: "Poppins"; weight: Font.Bold; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: ethernetWarningLogo.bottom
            anchors.topMargin: 32 * virtualstudio.uiScale
        }

        Text {
            id: ethernetWarningSubheader1
            text: "JackTrip works best when you connect directly to your router via wired ethernet."
            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            width: 400
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: ethernetWarningHeader.bottom
            anchors.topMargin: 32 * virtualstudio.uiScale
        }

        Text {
            id: ethernetWarningSubheader2
            text: "WiFi works OK for some people, but you will experience higher latency and audio glitches."
            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            width: 400
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: ethernetWarningSubheader1.bottom
            anchors.topMargin: 24 * virtualstudio.uiScale
        }

        Button {
            id: okButtonEthernet
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: okButtonEthernet.down ? saveButtonPressedColour : saveButtonBackgroundColour
                border.width: 1
                border.color: okButtonEthernet.down ? saveButtonPressedStroke : saveButtonStroke
                layer.enabled: okButtonEthernet.hovered && !okButtonEthernet.down
                layer.effect: DropShadow {
                    horizontalOffset: 1 * virtualstudio.uiScale
                    verticalOffset: 1 * virtualstudio.uiScale
                    radius: 8.0 * virtualstudio.uiScale
                    samples: 17
                    color: saveButtonShadow
                }
            }
            onClicked: { warningScreen = "headphones" }
            anchors.right: parent.right
            anchors.rightMargin: 16 * virtualstudio.uiScale
            anchors.bottomMargin: 16 * virtualstudio.uiScale
            anchors.bottom: parent.bottom
            width: 150 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
            Text {
                text: "OK"
                font.family: "Poppins"
                font.pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale
                font.weight: Font.Bold
                color: saveButtonText
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        CheckBox {
            id: showEthernetWarningCheckbox
            checked: currShowWarnings
            text: qsTr("Show warnings again next time")
            anchors.right: okButtonEthernet.left
            anchors.rightMargin: 16 * virtualstudio.uiScale
            anchors.verticalCenter: okButtonEthernet.verticalCenter
            onClicked: { currShowWarnings = showEthernetWarningCheckbox.checkState == Qt.Checked }
            indicator: Rectangle {
                implicitWidth: 16 * virtualstudio.uiScale
                implicitHeight: 16 * virtualstudio.uiScale
                x: showEthernetWarningCheckbox.leftPadding
                y: parent.height / 2 - height / 2
                radius: 3 * virtualstudio.uiScale
                border.color: showEthernetWarningCheckbox.down ? checkboxPressedStroke : checkboxStroke

                Rectangle {
                    width: 10 * virtualstudio.uiScale
                    height: 10 * virtualstudio.uiScale
                    x: 3 * virtualstudio.uiScale
                    y: 3 * virtualstudio.uiScale
                    radius: 2 * virtualstudio.uiScale
                    color: showEthernetWarningCheckbox.down ? checkboxPressedStroke : checkboxStroke
                    visible: showEthernetWarningCheckbox.checked
                }
            }
            contentItem: Text {
                text: showEthernetWarningCheckbox.text
                font.family: "Poppins"
                font.pixelSize: 10 * virtualstudio.fontScale * virtualstudio.uiScale
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                leftPadding: showEthernetWarningCheckbox.indicator.width + showEthernetWarningCheckbox.spacing
                color: textColour
            }
        }
    }

    Item {
        id: headphoneWarningItem
        width: parent.width; height: parent.height
        visible: warningScreen == "headphones"

        Image {
            id: headphoneWarningLogo
            source: "headphones.svg"
            sourceSize: Qt.size( img.sourceSize.width*5, img.sourceSize.height*5 )
            Image {
                id: img
                source: parent.source
                width: 0
                height: 0
            }
            width: 118
            height: 128
            y: 60
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Colorize {
            anchors.fill: headphoneWarningLogo
            source: headphoneWarningLogo
            hue: 0
            saturation: 0
            lightness: imageLightnessValue
        }

        Text {
            id: headphoneWarningHeader
            text: "Use Wired Headphones"
            font { family: "Poppins"; weight: Font.Bold; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: headphoneWarningLogo.bottom
            anchors.topMargin: 32 * virtualstudio.uiScale
        }

        Text {
            id: headphoneWarningSubheader1
            text: "JackTrip requires the use of wired headphones."
            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            width: 400
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: headphoneWarningHeader.bottom
            anchors.topMargin: 32 * virtualstudio.uiScale
        }

        Text {
            id: headphoneWarningSubheader2
            text: "Using speakers can cause loud feedback loops."
            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            width: 400
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: headphoneWarningSubheader1.bottom
            anchors.topMargin: 24 * virtualstudio.uiScale
        }

        Text {
            id: headphoneWarningSubheader3
            text: "Wireless headphones add way too much latency."
            font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
            width: 400
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: headphoneWarningSubheader2.bottom
            anchors.topMargin: 24 * virtualstudio.uiScale
        }

        Button {
            id: okButtonHeadphones
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: okButtonHeadphones.down ? saveButtonPressedColour : saveButtonBackgroundColour
                border.width: 1
                border.color: okButtonHeadphones.down ? saveButtonPressedStroke : saveButtonStroke
                layer.enabled: okButtonHeadphones.hovered && !okButtonHeadphones.down
                layer.effect: DropShadow {
                    horizontalOffset: 1 * virtualstudio.uiScale
                    verticalOffset: 1 * virtualstudio.uiScale
                    radius: 8.0 * virtualstudio.uiScale
                    samples: 17
                    color: saveButtonShadow
                }
            }
            onClicked: { virtualstudio.showWarnings = currShowWarnings; warningScreen = "acknowledged" }
            anchors.right: parent.right
            anchors.rightMargin: 16 * virtualstudio.uiScale
            anchors.bottomMargin: 16 * virtualstudio.uiScale
            anchors.bottom: parent.bottom
            width: 150 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
            Text {
                text: "OK"
                font.family: "Poppins"
                font.pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale
                font.weight: Font.Bold
                color: saveButtonText
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        CheckBox {
            id: showHeadphonesWarningCheckbox
            checked: currShowWarnings
            text: qsTr("Show warnings again next time")
            anchors.right: okButtonHeadphones.left
            anchors.rightMargin: 16 * virtualstudio.uiScale
            anchors.verticalCenter: okButtonHeadphones.verticalCenter
            onClicked: { currShowWarnings = showHeadphonesWarningCheckbox.checkState == Qt.Checked }
            indicator: Rectangle {
                implicitWidth: 16 * virtualstudio.uiScale
                implicitHeight: 16 * virtualstudio.uiScale
                x: showHeadphonesWarningCheckbox.leftPadding
                y: parent.height / 2 - height / 2
                radius: 3 * virtualstudio.uiScale
                border.color: showHeadphonesWarningCheckbox.down ? checkboxPressedStroke : checkboxStroke

                Rectangle {
                    width: 10 * virtualstudio.uiScale
                    height: 10 * virtualstudio.uiScale
                    x: 3 * virtualstudio.uiScale
                    y: 3 * virtualstudio.uiScale
                    radius: 2 * virtualstudio.uiScale
                    color: showHeadphonesWarningCheckbox.down ? checkboxPressedStroke : checkboxStroke
                    visible: showHeadphonesWarningCheckbox.checked
                }
            }
            contentItem: Text {
                text: showHeadphonesWarningCheckbox.text
                font.family: "Poppins"
                font.pixelSize: 10 * virtualstudio.fontScale * virtualstudio.uiScale
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                leftPadding: showHeadphonesWarningCheckbox.indicator.width + showHeadphonesWarningCheckbox.spacing
                color: textColour
            }
        }
    }

    // Item {
    //     id: headphoneWarningItem
    //     width: parent.width; height: parent.height
    //     visible: warningScreen == "headphones"

    //     Image {
    //         id: headphoneWarningLogo
    //         source: "headphones.svg"
    //         sourceSize: Qt.size( img.sourceSize.width*5, img.sourceSize.height*5 )
    //         Image {
    //             id: img
    //             source: parent.source
    //             width: 0
    //             height: 0
    //         }
    //         width: 118
    //         height: 128
    //         y: 60
    //         anchors.horizontalCenter: parent.horizontalCenter
    //     }

    //     Colorize {
    //         anchors.fill: headphoneWarningLogo
    //         source: headphoneWarningLogo
    //         hue: 0
    //         saturation: 0
    //         lightness: imageLightnessValue
    //     }

    //     Text {
    //         id: headphoneWarningHeader
    //         text: "Use Wired Headphones"
    //         font { family: "Poppins"; weight: Font.Bold; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
    //         color: textColour
    //         anchors.horizontalCenter: parent.horizontalCenter
    //         anchors.top: headphoneWarningLogo.bottom
    //         anchors.topMargin: 32 * virtualstudio.uiScale
    //     }

    //     Text {
    //         id: headphoneWarningSubheader1
    //         text: "JackTrip requires the use of wired headphones."
    //         font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
    //         color: textColour
    //         width: 400
    //         wrapMode: Text.Wrap
    //         horizontalAlignment: Text.AlignHCenter
    //         anchors.horizontalCenter: parent.horizontalCenter
    //         anchors.top: headphoneWarningHeader.bottom
    //         anchors.topMargin: 32 * virtualstudio.uiScale
    //     }

    //     Text {
    //         id: headphoneWarningSubheader2
    //         text: "Using speakers can cause loud feedback loops."
    //         font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
    //         color: textColour
    //         width: 400
    //         wrapMode: Text.Wrap
    //         horizontalAlignment: Text.AlignHCenter
    //         anchors.horizontalCenter: parent.horizontalCenter
    //         anchors.top: headphoneWarningSubheader1.bottom
    //         anchors.topMargin: 24 * virtualstudio.uiScale
    //     }

    //     Text {
    //         id: headphoneWarningSubheader3
    //         text: "Wireless headphones add way too much latency."
    //         font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
    //         color: textColour
    //         width: 400
    //         wrapMode: Text.Wrap
    //         horizontalAlignment: Text.AlignHCenter
    //         anchors.horizontalCenter: parent.horizontalCenter
    //         anchors.top: headphoneWarningSubheader2.bottom
    //         anchors.topMargin: 24 * virtualstudio.uiScale
    //     }

    //     Button {
    //         id: okButtonHeadphones
    //         background: Rectangle {
    //             radius: 6 * virtualstudio.uiScale
    //             color: okButtonHeadphones.down ? saveButtonPressedColour : saveButtonBackgroundColour
    //             border.width: 1
    //             border.color: okButtonHeadphones.down ? saveButtonPressedStroke : saveButtonStroke
    //             layer.enabled: okButtonHeadphones.hovered && !okButtonHeadphones.down
    //             layer.effect: DropShadow {
    //                 horizontalOffset: 1 * virtualstudio.uiScale
    //                 verticalOffset: 1 * virtualstudio.uiScale
    //                 radius: 8.0 * virtualstudio.uiScale
    //                 samples: 17
    //                 color: saveButtonShadow
    //             }
    //         }
    //         onClicked: { virtualstudio.showWarnings = currShowWarnings; warningScreen = "acknowledged" }
    //         anchors.right: parent.right
    //         anchors.rightMargin: 16 * virtualstudio.uiScale
    //         anchors.bottomMargin: 16 * virtualstudio.uiScale
    //         anchors.bottom: parent.bottom
    //         width: 150 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
    //         Text {
    //             text: "OK"
    //             font.family: "Poppins"
    //             font.pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale
    //             font.weight: Font.Bold
    //             color: saveButtonText
    //             anchors.horizontalCenter: parent.horizontalCenter
    //             anchors.verticalCenter: parent.verticalCenter
    //         }
    //     }

    //     CheckBox {
    //         id: showHeadphonesWarningCheckbox
    //         checked: currShowWarnings
    //         text: qsTr("Show warnings again next time")
    //         anchors.right: okButtonHeadphones.left
    //         anchors.rightMargin: 16 * virtualstudio.uiScale
    //         anchors.verticalCenter: okButtonHeadphones.verticalCenter
    //         onClicked: { currShowWarnings = showHeadphonesWarningCheckbox.checkState == Qt.Checked }
    //         indicator: Rectangle {
    //             implicitWidth: 16 * virtualstudio.uiScale
    //             implicitHeight: 16 * virtualstudio.uiScale
    //             x: showHeadphonesWarningCheckbox.leftPadding
    //             y: parent.height / 2 - height / 2
    //             radius: 3 * virtualstudio.uiScale
    //             border.color: showHeadphonesWarningCheckbox.down ? checkboxPressedStroke : checkboxStroke

    //             Rectangle {
    //                 width: 10 * virtualstudio.uiScale
    //                 height: 10 * virtualstudio.uiScale
    //                 x: 3 * virtualstudio.uiScale
    //                 y: 3 * virtualstudio.uiScale
    //                 radius: 2 * virtualstudio.uiScale
    //                 color: showHeadphonesWarningCheckbox.down ? checkboxPressedStroke : checkboxStroke
    //                 visible: showHeadphonesWarningCheckbox.checked
    //             }
    //         }
    //         contentItem: Text {
    //             text: showHeadphonesWarningCheckbox.text
    //             font.family: "Poppins"
    //             font.pixelSize: 10 * virtualstudio.fontScale * virtualstudio.uiScale
    //             anchors.horizontalCenter: parent.horizontalCenter
    //             anchors.verticalCenter: parent.verticalCenter
    //             leftPadding: showHeadphonesWarningCheckbox.indicator.width + showHeadphonesWarningCheckbox.spacing
    //             color: textColour
    //         }
    //     }
    // }

    Item {
        id: noMicItem
        width: parent.width; height: parent.height
        visible: warningScreen == "acknowledged" && permissions.micPermission == 'denied'

        Text {
            id: noMicPageTitle
            x: 16 * virtualstudio.uiScale; y: 32 * virtualstudio.uiScale
            text: permissions.micPermission
            font { family: "Poppins"; weight: Font.Bold; pixelSize: fontBig * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
        }
    }

    Item {
        id: setupItem
        width: parent.width; height: parent.height
        visible: warningScreen == "acknowledged" && permissions.micPermission == 'granted'

        Text {
            id: pageTitle
            x: 16 * virtualstudio.uiScale; y: 32 * virtualstudio.uiScale
            text: "Choose your audio devices"
            font { family: "Poppins"; weight: Font.Bold; pixelSize: fontBig * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
        }

        ComboBox {
            id: backendCombo
            model: backendComboModel
            currentIndex: virtualstudio.audioBackend == "JACK" ? 0 : 1
            onActivated: { virtualstudio.audioBackend = currentText }
            anchors.right: parent.right
            anchors.rightMargin: rightMargin * virtualstudio.uiScale
            y: pageTitle.y + 96 * virtualstudio.uiScale
            width: parent.width - (234 * virtualstudio.uiScale); height: 36 * virtualstudio.uiScale
            visible: virtualstudio.selectableBackend
        }

        Text {
            id: backendLabel
            anchors.verticalCenter: backendCombo.verticalCenter
            x: leftMargin * virtualstudio.uiScale
            text: "Audio Backend"
            font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            visible: virtualstudio.selectableBackend
            color: textColour
        }

        Text {
            id: jackLabel
            x: leftMargin * virtualstudio.uiScale; y: 150 * virtualstudio.uiScale
            width: parent.width - x - (16 * virtualstudio.uiScale)
            text: "Using JACK for audio input and output. Use QjackCtl to adjust your sample rate, buffer, and device settings."
            font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            wrapMode: Text.WordWrap
            visible: virtualstudio.audioBackend == "JACK" && !virtualstudio.selectableBackend
            color: textColour
        }

        ComboBox {
            id: outputCombo
            model: outputComboModel
            currentIndex: virtualstudio.outputDevice
            onActivated: { virtualstudio.outputDevice = currentIndex }
            x: backendCombo.x; y: backendCombo.y + virtualstudio.uiScale * (virtualstudio.selectableBackend ? 48 : 0)
            width: backendCombo.width; height: backendCombo.height
            visible: virtualstudio.audioBackend != "JACK"
        }

        Text {
            id: outputLabel
            anchors.verticalCenter: virtualstudio.audioBackend != "JACK" ? outputCombo.verticalCenter : outputSlider.verticalCenter
            x: leftMargin * virtualstudio.uiScale
            text: virtualstudio.audioBackend != "JACK" ? "Output Device" : "Output Volume"
            font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
        }

        Slider {
            id: outputSlider
            from: 0.0
            value: audioInterface ? audioInterface.outputVolume : 0.5
            onMoved: { audioInterface.outputVolume = value }
            to: 1.0
            padding: 0
            y: virtualstudio.audioBackend != "JACK" ? outputCombo.y + 48 * virtualstudio.uiScale : backendCombo.y + virtualstudio.uiScale * (virtualstudio.selectableBackend ? 60 : 0)
            anchors.left: outputCombo.left
            anchors.right: parent.right
            anchors.rightMargin: rightMargin * virtualstudio.uiScale
            handle: Rectangle {
                x: outputSlider.leftPadding + outputSlider.visualPosition * (outputSlider.availableWidth - width)
                y: outputSlider.topPadding + outputSlider.availableHeight / 2 - height / 2
                implicitWidth: 26 * virtualstudio.uiScale
                implicitHeight: 26 * virtualstudio.uiScale
                radius: 13 * virtualstudio.uiScale
                color: outputSlider.pressed ? sliderPressedColour : sliderColour
                border.color: buttonStroke
            }
        }

        Button {
            id: testOutputAudioButton
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: testOutputAudioButton.down ? buttonPressedColour : (testOutputAudioButton.hovered ? buttonHoverColour : buttonColour)
                border.width: 1
                border.color: testOutputAudioButton.down ? buttonPressedStroke : (testOutputAudioButton.hovered ? buttonHoverStroke : buttonStroke)
            }
            onClicked: { virtualstudio.playOutputAudio() }
            anchors.right: parent.right
            anchors.rightMargin: rightMargin * virtualstudio.uiScale
            y: outputSlider.y + (36 * virtualstudio.uiScale)
            width: 216 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
            visible: virtualstudio.audioBackend != "JACK"
            Text {
                text: "Test Output Audio"
                font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
                color: textColour
            }
        }

        ComboBox {
            id: inputCombo
            model: inputComboModel
            currentIndex: virtualstudio.inputDevice
            onActivated: { virtualstudio.inputDevice = currentIndex }
            anchors.right: parent.right
            anchors.rightMargin: rightMargin * virtualstudio.uiScale
            y: testOutputAudioButton.y + (48 * virtualstudio.uiScale)
            width: parent.width - (234 * virtualstudio.uiScale); height: 36 * virtualstudio.uiScale
            visible: virtualstudio.audioBackend != "JACK"
        }

        Text {
            id: inputLabel
            anchors.top: virtualstudio.audioBackend != "JACK" ? inputCombo.top : inputDeviceMeters.top
            anchors.topMargin: virtualstudio.audioBackend != "JACK" ? (inputCombo.height - inputLabel.height)/2 : 0
            x: leftMargin * virtualstudio.uiScale
            text: virtualstudio.audioBackend != "JACK" ? "Input Device" : "Input Volume"
            font { family: "Poppins"; pixelSize: fontMedium * virtualstudio.fontScale * virtualstudio.uiScale }
            color: textColour
        }

        Meter {
            id: inputDeviceMeters
            anchors.left: backendCombo.left
            anchors.right: parent.right
            anchors.rightMargin: rightMargin * virtualstudio.uiScale
            y: virtualstudio.audioBackend != "JACK" ? inputCombo.y + 72 * virtualstudio.uiScale : outputSlider.y + (72 * virtualstudio.uiScale)
            height: 100 * virtualstudio.uiScale
            model: inputMeterModel
            clipped: inputClipped
        }

        Slider {
            id: inputSlider
            from: 0.0
            value: audioInterface ? audioInterface.inputVolume : 0.5
            onMoved: { audioInterface.inputVolume = value }
            to: 1.0
            padding: 0
            y: inputDeviceMeters.y + 48 * virtualstudio.uiScale
            anchors.left: inputDeviceMeters.left
            anchors.right: parent.right
            anchors.rightMargin: rightMargin * virtualstudio.uiScale
            handle: Rectangle {
                x: inputSlider.leftPadding + inputSlider.visualPosition * (inputSlider.availableWidth - width)
                y: inputSlider.topPadding + inputSlider.availableHeight / 2 - height / 2
                implicitWidth: 26 * virtualstudio.uiScale
                implicitHeight: 26 * virtualstudio.uiScale
                radius: 13 * virtualstudio.uiScale
                color: inputSlider.pressed ? sliderPressedColour : sliderColour
                border.color: buttonStroke
            }
        }

        Button {
            id: refreshButton
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: refreshButton.down ? buttonPressedColour : (refreshButton.hovered ? buttonHoverColour : buttonColour)
                border.width: 1
                border.color: refreshButton.down ? buttonPressedStroke : (refreshButton.hovered ? buttonHoverStroke : buttonStroke)
            }
            onClicked: { virtualstudio.refreshDevices() }
            anchors.right: parent.right
            anchors.rightMargin: rightMargin * virtualstudio.uiScale
            anchors.topMargin: 18 * virtualstudio.uiScale
            anchors.top: inputSlider.bottom
            width: 216 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
            visible: virtualstudio.audioBackend != "JACK"
            Text {
                text: "Refresh Devices"
                font { family: "Poppins"; pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale }
                anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
                color: textColour
            }
        }

        Text {
            anchors.left: outputLabel.left
            anchors.right: outputCombo.right
            anchors.leftMargin: 16 * virtualstudio.uiScale
            anchors.rightMargin: 16 * virtualstudio.uiScale
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 60 * virtualstudio.uiScale
            text: "JackTrip on Windows requires use of an audio device with ASIO drivers. If you do not see your device, you may need to install drivers from your manufacturer."
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            color: warningText
            font { family: "Poppins"; pixelSize: fontExtraSmall * virtualstudio.fontScale * virtualstudio.uiScale }
            visible: Qt.platform.os == "windows" && virtualstudio.audioBackend != "JACK"
        }

        Button {
            id: saveButton
            background: Rectangle {
                radius: 6 * virtualstudio.uiScale
                color: saveButton.down ? saveButtonPressedColour : saveButtonBackgroundColour
                border.width: 1
                border.color: saveButton.down ? saveButtonPressedStroke : saveButtonStroke
                layer.enabled: saveButton.hovered && !saveButton.down
                layer.effect: DropShadow {
                    horizontalOffset: 1 * virtualstudio.uiScale
                    verticalOffset: 1 * virtualstudio.uiScale
                    radius: 8.0 * virtualstudio.uiScale
                    samples: 17
                    color: saveButtonShadow
                }
            }
            onClicked: { window.state = "browse"; virtualstudio.applySettings() }
            anchors.right: parent.right
            anchors.rightMargin: rightMargin * virtualstudio.uiScale
            anchors.bottomMargin: rightMargin * virtualstudio.uiScale
            anchors.bottom: parent.bottom
            width: 150 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
            Text {
                text: "Save Settings"
                font.family: "Poppins"
                font.pixelSize: fontSmall * virtualstudio.fontScale * virtualstudio.uiScale
                font.weight: Font.Bold
                color: saveButtonText
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        CheckBox {
            id: showAgainCheckbox
            checked: virtualstudio.showDeviceSetup
            text: qsTr("Ask again next time")
            anchors.right: saveButton.left
            anchors.rightMargin: 16 * virtualstudio.uiScale
            anchors.verticalCenter: saveButton.verticalCenter
            onClicked: { virtualstudio.showDeviceSetup = showAgainCheckbox.checkState == Qt.Checked }
            indicator: Rectangle {
                implicitWidth: 16 * virtualstudio.uiScale
                implicitHeight: 16 * virtualstudio.uiScale
                x: showAgainCheckbox.leftPadding
                y: parent.height / 2 - height / 2
                radius: 3 * virtualstudio.uiScale
                border.color: showAgainCheckbox.down ? checkboxPressedStroke : checkboxStroke

                Rectangle {
                    width: 10 * virtualstudio.uiScale
                    height: 10 * virtualstudio.uiScale
                    x: 3 * virtualstudio.uiScale
                    y: 3 * virtualstudio.uiScale
                    radius: 2 * virtualstudio.uiScale
                    color: showAgainCheckbox.down ? checkboxPressedStroke : checkboxStroke
                    visible: showAgainCheckbox.checked
                }
            }

            contentItem: Text {
                text: showAgainCheckbox.text
                font.family: "Poppins"
                font.pixelSize: 10 * virtualstudio.fontScale * virtualstudio.uiScale
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                leftPadding: showAgainCheckbox.indicator.width + showAgainCheckbox.spacing
                color: textColour
            }
        }
    }
}
