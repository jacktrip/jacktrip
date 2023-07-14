import QtQuick
import QtQuick.Controls

Rectangle {
    property string backgroundColour: virtualstudio.darkMode ? "#272525" : "#FAFBFB"
    property string textColour: virtualstudio.darkMode ? "#FAFBFB" : "#0F0D0D"

    width: 800
    height: 640
    color: backgroundColour
    state: virtualstudio.showFirstRun ? "start" : virtualstudio.windowState
    anchors.fill: parent

    id: window
    states: [
        State {
            name: "start"
            PropertyChanges { target: startScreen; x: 0 }
            PropertyChanges { target: loginScreen; x: window.width; }
            PropertyChanges { target: setupScreen; x: window.width }
            PropertyChanges { target: browseScreen; x: window.width }
            PropertyChanges { target: settingsScreen; x: window.width }
            PropertyChanges { target: connectedScreen; x: window.width }
            PropertyChanges { target: changeDevicesScreen; x: 2*window.width }
            PropertyChanges { target: failedScreen; x: window.width }
        },

        State {
            name: "login"
            PropertyChanges { target: startScreen; x: -startScreen.width }
            PropertyChanges { target: loginScreen; x: 0; }
            PropertyChanges { target: setupScreen; x: window.width }
            PropertyChanges { target: browseScreen; x: window.width }
            PropertyChanges { target: settingsScreen; x: window.width }
            PropertyChanges { target: connectedScreen; x: window.width }
            PropertyChanges { target: changeDevicesScreen; x: 2*window.width }
            PropertyChanges { target: failedScreen; x: window.width }
        },

        State {
            name: "setup"
            PropertyChanges { target: loginScreen; x: -loginScreen.width }
            PropertyChanges { target: startScreen; x: -startScreen.width }
            PropertyChanges { target: setupScreen; x: 0 }
            PropertyChanges { target: browseScreen; x: window.width }
            PropertyChanges { target: settingsScreen; x: window.width }
            PropertyChanges { target: connectedScreen; x: window.width }
            PropertyChanges { target: changeDevicesScreen; x: 2*window.width }
            PropertyChanges { target: failedScreen; x: window.width }
        },

        State {
            name: "browse"
            PropertyChanges { target: loginScreen; x: -loginScreen.width }
            PropertyChanges { target: startScreen; x: -startScreen.width }
            PropertyChanges { target: setupScreen; x: -setupScreen.width }
            PropertyChanges { target: browseScreen; x: 0 }
            PropertyChanges { target: settingsScreen; x: window.width }
            PropertyChanges { target: connectedScreen; x: window.width }
            PropertyChanges { target: changeDevicesScreen; x: 2*window.width }
            PropertyChanges { target: failedScreen; x: window.width }
        },

        State {
            name: "settings"
            PropertyChanges { target: loginScreen; x: -loginScreen.width }
            PropertyChanges { target: startScreen; x: -startScreen.width }
            PropertyChanges { target: setupScreen; x: -setupScreen.width }
            PropertyChanges { target: browseScreen; x: -browseScreen.width }
            PropertyChanges { target: settingsScreen; x: 0 }
            PropertyChanges { target: connectedScreen; x: window.width }
            PropertyChanges { target: changeDevicesScreen; x: 2*window.width }
            PropertyChanges { target: failedScreen; x: window.width }
        },

        State {
            name: "connected"
            PropertyChanges { target: loginScreen; x: -loginScreen.width }
            PropertyChanges { target: startScreen; x: -startScreen.width }
            PropertyChanges { target: setupScreen; x: -setupScreen.width }
            PropertyChanges { target: browseScreen; x: -browseScreen.width }
            PropertyChanges { target: settingsScreen; x: window.width }
            PropertyChanges { target: connectedScreen; x: 0 }
            PropertyChanges { target: changeDevicesScreen; x: window.width }
            PropertyChanges { target: failedScreen; x: window.width }
        },

        State {
            name: "change_devices"
            PropertyChanges { target: loginScreen; x: -loginScreen.width }
            PropertyChanges { target: startScreen; x: -startScreen.width }
            PropertyChanges { target: setupScreen; x: -setupScreen.width }
            PropertyChanges { target: browseScreen; x: -browseScreen.width }
            PropertyChanges { target: settingsScreen; x: window.width }
            PropertyChanges { target: connectedScreen; x: -connectedScreen.width }
            PropertyChanges { target: changeDevicesScreen; x: 0 }
            PropertyChanges { target: failedScreen; x: window.width }
        },

        State {
            name: "failed"
            PropertyChanges { target: loginScreen; x: -loginScreen.width }
            PropertyChanges { target: startScreen; x: -startScreen.width }
            PropertyChanges { target: setupScreen; x: -setupScreen.width }
            PropertyChanges { target: browseScreen; x: -browseScreen.width }
            PropertyChanges { target: settingsScreen; x: window.width }
            PropertyChanges { target: connectedScreen; x: window.width }
            PropertyChanges { target: changeDevicesScreen; x: 2*window.width }
            PropertyChanges { target: failedScreen; x: 0 }
        }
    ]

    transitions: Transition {
        NumberAnimation { properties: "x"; duration: 800; easing.type: Easing.InOutQuad }
    }

    FirstLaunch {
        id: startScreen
    }

    Setup {
        id: setupScreen
    }

    Browse {
        id: browseScreen
    }

    Login {
        id: loginScreen
    }

    Settings {
        id: settingsScreen
    }

    Connected {
        id: connectedScreen
    }

    ChangeDevices {
        id: changeDevicesScreen
    }

    Failed {
        id: failedScreen
    }

    Connections {
        target: virtualstudio
        function onAuthSucceeded() {
            if (virtualstudio.windowState !== "login") {
                // can happen on settings screen when switching between prod and test
                return;
            }
            if (virtualstudio.showDeviceSetup) {
                virtualstudio.windowState = "setup";
            } else {
                virtualstudio.windowState = "browse";
            }
        }
        function onConnected() {
            if (virtualstudio.windowState == "change_devices") {
                return;
            }
            virtualstudio.windowState = "connected";
        }
        function onFailed() {
            virtualstudio.windowState = "failed";
        }
        function onDisconnected() {
            virtualstudio.windowState = "browse";
        }
        function onWindowStateUpdated() {
            if (virtualstudio.windowState === "login") {
                virtualstudio.login();
            }
        }
    }
}
