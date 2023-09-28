import QtQuick
import QtQuick.Controls

Rectangle {
    property string backgroundColour: virtualstudio.darkMode ? "#272525" : "#FAFBFB"
    property string textColour: virtualstudio.darkMode ? "#FAFBFB" : "#0F0D0D"

    color: backgroundColour
    state: virtualstudio.windowState
    anchors.fill: parent

    id: window
    states: [
        State {
            name: "start"
            PropertyChanges { target: startScreen; x: 0 }
            PropertyChanges { target: loginScreen; x: window.width; }
            PropertyChanges { target: recommendationsScreen; x: window.width }
            PropertyChanges { target: permissionsScreen; x: window.width }
            PropertyChanges { target: setupScreen; x: window.width }
            PropertyChanges { target: browseScreen; x: window.width }
            PropertyChanges { target: settingsScreen; x: window.width }
            PropertyChanges { target: createStudioScreen; x: window.width }
            PropertyChanges { target: connectedScreen; x: window.width }
            PropertyChanges { target: changeDevicesScreen; x: 2*window.width }
            PropertyChanges { target: failedScreen; x: window.width }
        },

        State {
            name: "login"
            PropertyChanges { target: startScreen; x: -startScreen.width }
            PropertyChanges { target: loginScreen; x: 0; }
            PropertyChanges { target: recommendationsScreen; x: window.width }
            PropertyChanges { target: permissionsScreen; x: window.width }
            PropertyChanges { target: setupScreen; x: window.width }
            PropertyChanges { target: browseScreen; x: window.width }
            PropertyChanges { target: settingsScreen; x: window.width }
            PropertyChanges { target: createStudioScreen; x: window.width }
            PropertyChanges { target: connectedScreen; x: window.width }
            PropertyChanges { target: changeDevicesScreen; x: 2*window.width }
            PropertyChanges { target: failedScreen; x: window.width }
        },

        State {
            name: "recommendations"
            PropertyChanges { target: loginScreen; x: -loginScreen.width }
            PropertyChanges { target: startScreen; x: -startScreen.width }
            PropertyChanges { target: recommendationsScreen; x: 0 }
            PropertyChanges { target: permissionsScreen; x: window.width }
            PropertyChanges { target: setupScreen; x: window.width }
            PropertyChanges { target: browseScreen; x: window.width }
            PropertyChanges { target: settingsScreen; x: window.width }
            PropertyChanges { target: createStudioScreen; x: window.width }
            PropertyChanges { target: connectedScreen; x: window.width }
            PropertyChanges { target: changeDevicesScreen; x: 2*window.width }
            PropertyChanges { target: failedScreen; x: window.width }
        },

        State {
            name: "permissions"
            PropertyChanges { target: loginScreen; x: -loginScreen.width }
            PropertyChanges { target: startScreen; x: -startScreen.width }
            PropertyChanges { target: recommendationsScreen; x: -recommendationsScreen.width }
            PropertyChanges { target: permissionsScreen; x: 0 }
            PropertyChanges { target: setupScreen; x: window.width }
            PropertyChanges { target: browseScreen; x: window.width }
            PropertyChanges { target: settingsScreen; x: window.width }
            PropertyChanges { target: createStudioScreen; x: window.width }
            PropertyChanges { target: connectedScreen; x: window.width }
            PropertyChanges { target: changeDevicesScreen; x: 2*window.width }
            PropertyChanges { target: failedScreen; x: window.width }
        },

        State {
            name: "setup"
            PropertyChanges { target: loginScreen; x: -loginScreen.width }
            PropertyChanges { target: startScreen; x: -startScreen.width }
            PropertyChanges { target: recommendationsScreen; x: -recommendationsScreen.width }
            PropertyChanges { target: permissionsScreen; x: -permissionsScreen.width }
            PropertyChanges { target: setupScreen; x: 0 }
            PropertyChanges { target: browseScreen; x: -browseScreen.width }
            PropertyChanges { target: settingsScreen; x: window.width }
            PropertyChanges { target: createStudioScreen; x: window.width }
            PropertyChanges { target: connectedScreen; x: window.width }
            PropertyChanges { target: changeDevicesScreen; x: 2*window.width }
            PropertyChanges { target: failedScreen; x: window.width }
        },

        State {
            name: "browse"
            PropertyChanges { target: loginScreen; x: -loginScreen.width }
            PropertyChanges { target: startScreen; x: -startScreen.width }
            PropertyChanges { target: recommendationsScreen; x: -recommendationsScreen.width }
            PropertyChanges { target: permissionsScreen; x: -permissionsScreen.width }
            PropertyChanges { target: setupScreen; x: window.width }
            PropertyChanges { target: browseScreen; x: 0 }
            PropertyChanges { target: settingsScreen; x: window.width }
            PropertyChanges { target: createStudioScreen; x: window.width }
            PropertyChanges { target: connectedScreen; x: window.width }
            PropertyChanges { target: changeDevicesScreen; x: 2*window.width }
            PropertyChanges { target: failedScreen; x: window.width }
        },

        State {
            name: "settings"
            PropertyChanges { target: loginScreen; x: -loginScreen.width }
            PropertyChanges { target: startScreen; x: -startScreen.width }
            PropertyChanges { target: recommendationsScreen; x: -recommendationsScreen.width }
            PropertyChanges { target: permissionsScreen; x: -permissionsScreen.width }
            PropertyChanges { target: setupScreen; x: window.width }
            PropertyChanges { target: browseScreen; x: -browseScreen.width }
            PropertyChanges { target: settingsScreen; x: 0 }
            PropertyChanges { target: createStudioScreen; x: window.width }
            PropertyChanges { target: connectedScreen; x: window.width }
            PropertyChanges { target: changeDevicesScreen; x: 2*window.width }
            PropertyChanges { target: failedScreen; x: window.width }
        },

        State {
            name: "create_studio"
            PropertyChanges { target: loginScreen; x: -loginScreen.width }
            PropertyChanges { target: startScreen; x: -startScreen.width }
            PropertyChanges { target: recommendationsScreen; x: -recommendationsScreen.width }
            PropertyChanges { target: permissionsScreen; x: -permissionsScreen.width }
            PropertyChanges { target: setupScreen; x: window.width }
            PropertyChanges { target: browseScreen; x: -browseScreen.width }
            PropertyChanges { target: settingsScreen; x: window.width }
            PropertyChanges { target: createStudioScreen; x: 0 }
            PropertyChanges { target: connectedScreen; x: window.width }
            PropertyChanges { target: changeDevicesScreen; x: window.width }
            PropertyChanges { target: failedScreen; x: window.width }
        },

        State {
            name: "connected"
            PropertyChanges { target: loginScreen; x: -loginScreen.width }
            PropertyChanges { target: startScreen; x: -startScreen.width }
            PropertyChanges { target: recommendationsScreen; x: -recommendationsScreen.width }
            PropertyChanges { target: permissionsScreen; x: -permissionsScreen.width }
            PropertyChanges { target: setupScreen; x: 0 }
            PropertyChanges { target: browseScreen; x: -browseScreen.width }
            PropertyChanges { target: settingsScreen; x: window.width }
            PropertyChanges { target: createStudioScreen; x: -createStudioScreen.width }
            PropertyChanges { target: connectedScreen; x: 0 }
            PropertyChanges { target: changeDevicesScreen; x: window.width }
            PropertyChanges { target: failedScreen; x: window.width }
        },

        State {
            name: "change_devices"
            PropertyChanges { target: loginScreen; x: -loginScreen.width }
            PropertyChanges { target: startScreen; x: -startScreen.width }
            PropertyChanges { target: recommendationsScreen; x: -recommendationsScreen.width }
            PropertyChanges { target: permissionsScreen; x: -permissionsScreen.width }
            PropertyChanges { target: setupScreen; x: 0 }
            PropertyChanges { target: browseScreen; x: -browseScreen.width }
            PropertyChanges { target: settingsScreen; x: window.width }
            PropertyChanges { target: createStudioScreen; x: -createStudioScreen.width }
            PropertyChanges { target: connectedScreen; x: 0 }
            PropertyChanges { target: changeDevicesScreen; x: 0 }
            PropertyChanges { target: failedScreen; x: window.width }
        },

        State {
            name: "failed"
            PropertyChanges { target: loginScreen; x: -loginScreen.width }
            PropertyChanges { target: startScreen; x: -startScreen.width }
            PropertyChanges { target: recommendationsScreen; x: -recommendationsScreen.width }
            PropertyChanges { target: permissionsScreen; x: -permissionsScreen.width }
            PropertyChanges { target: setupScreen; x: -setupScreen.width }
            PropertyChanges { target: browseScreen; x: -browseScreen.width }
            PropertyChanges { target: settingsScreen; x: window.width }
            PropertyChanges { target: createStudioScreen; x: window.width }
            PropertyChanges { target: connectedScreen; x: window.width }
            PropertyChanges { target: changeDevicesScreen; x: 2*window.width }
            PropertyChanges { target: failedScreen; x: 0 }
        }
    ]

    transitions: Transition {
        NumberAnimation { properties: "x"; duration: 500; easing.type: Easing.InOutQuad }
    }

    FirstLaunch {
        id: startScreen
    }

    Login {
        id: loginScreen
    }

    Recommendations {
        id: recommendationsScreen
    }

    Permissions {
        id: permissionsScreen
    }

    Browse {
        id: browseScreen
    }

    Setup {
        id: setupScreen
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

    CreateStudio {
        id: createStudioScreen
    }

    Failed {
        id: failedScreen
    }

    onWidthChanged: {
        if (virtualstudio.windowState === "start") {
            startScreen.x = 0
        } else if (virtualstudio.windowState === "login") {
            loginScreen.x = 0
        } else if (virtualstudio.windowState === "recommendations") {
            recommendationsScreen.x = 0;
        } else if (virtualstudio.windowState === "permissions") {
            permissionsScreen.x = 0;
        } else if (virtualstudio.windowState === "setup") {
            setupScreen.x = 0
        } else if (virtualstudio.windowState === "browse") {
            browseScreen.x = 0
        } else if (virtualstudio.windowState === "settings") {
            settingsScreen.x = 0
        } else if (virtualstudio.windowState === "create_studio") {
            createStudioScreen.x = 0
        } else if (virtualstudio.windowState === "connected") {
            connectedScreen.x = 0
        } else if (virtualstudio.windowState === "change_devices") {
            changeDevicesScreen.x = 0
        } else if (virtualstudio.windowState === "failed") {
            failedScreen.x = 0
        }
    }

    onHeightChanged: {
        if (virtualstudio.windowState === "start") {
            startScreen.x = 0
        } else if (virtualstudio.windowState === "login") {
            loginScreen.x = 0
        } else if (virtualstudio.windowState === "recommendations") {
            recommendationsScreen.x = 0;
        } else if (virtualstudio.windowState === "permissions") {
            permissionsScreen.x = 0;
        } else if (virtualstudio.windowState === "setup") {
            setupScreen.x = 0
        } else if (virtualstudio.windowState === "browse") {
            browseScreen.x = 0
        } else if (virtualstudio.windowState === "settings") {
            settingsScreen.x = 0
        } else if (virtualstudio.windowState === "create_studio") {
            createStudioScreen.x = 0
        } else if (virtualstudio.windowState === "connected") {
            connectedScreen.x = 0
        } else if (virtualstudio.windowState === "change_devices") {
            changeDevicesScreen.x = 0
        } else if (virtualstudio.windowState === "failed") {
            failedScreen.x = 0
        }
    }

    Connections {
        target: auth
        function onAuthSucceeded() {
            if (virtualstudio.windowState !== "login") {
                // can happen on settings screen when switching between prod and test
                return;
            }
            if (virtualstudio.showWarnings) {
                virtualstudio.windowState = "recommendations";
            } else if (virtualstudio.studioToJoin.toString() === "") {
                virtualstudio.windowState = "browse";
            } else if (virtualstudio.showDeviceSetup) {
                virtualstudio.windowState = "setup";
                audio.startAudio();
            } else {
                virtualstudio.windowState = "connected";
                virtualstudio.joinStudio();
            }
        }
    }
    Connections {
        target: virtualstudio
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
    }
}
