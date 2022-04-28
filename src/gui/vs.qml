import QtQuick 2.12
import QtQuick.Controls 2.12

Rectangle {
    width: 696
    height: 577
    color: "#FAFBFB"
    state: virtualstudio.showFirstRun ? "start" : "login"

    id: window
    states: [
        State {
            name: "start"
            PropertyChanges { target: startScreen; x: 0 }
            PropertyChanges { target: loginScreen; x: window.width; failTextVisible: loginScreen.failTextVisible }
            PropertyChanges { target: browseScreen; x: window.width }
            PropertyChanges { target: settingsScreen; x: window.width }
            PropertyChanges { target: connectedScreen; x: window.width }
        },

        State {
            name: "login"
            PropertyChanges { target: startScreen; x: -startScreen.width }
            PropertyChanges { target: loginScreen; x: 0; failTextVisible: false }
            PropertyChanges { target: browseScreen; x: window.width }
            PropertyChanges { target: settingsScreen; x: window.width }
            PropertyChanges { target: connectedScreen; x: window.width }
        },

        State {
            name: "browse"
            PropertyChanges { target: loginScreen; x: -loginScreen.width }
            PropertyChanges { target: startScreen; x: -startScreen.width }
            PropertyChanges { target: browseScreen; x: 0 }
            PropertyChanges { target: settingsScreen; x: window.width }
            PropertyChanges { target: connectedScreen; x: window.width }
        },

        State {
            name: "settings"
            PropertyChanges { target: loginScreen; x: -loginScreen.width }
            PropertyChanges { target: startScreen; x: -startScreen.width }
            PropertyChanges { target: browseScreen; x: -browseScreen.width }
            PropertyChanges { target: settingsScreen; x: 0 }
            PropertyChanges { target: connectedScreen; x: window.width }
        },

        State {
            name: "connected"
            PropertyChanges { target: loginScreen; x: -loginScreen.width }
            PropertyChanges { target: startScreen; x: -startScreen.width }
            PropertyChanges { target: browseScreen; x: -browseScreen.width }
            PropertyChanges { target: settingsScreen; x: window.width }
            PropertyChanges { target: connectedScreen; x: 0 }
        }
    ]

    transitions: Transition {
        NumberAnimation { properties: "x"; duration: 800; easing.type: Easing.InOutQuad }
    }

    FirstLaunch {
        id: startScreen
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

    Connections {
        target: virtualstudio
        function onAuthSucceeded() { window.state = "browse" }
        function onAuthFailed() { loginScreen.failTextVisible = true }
        //function onConnected() { }
        function onDisconnected() { window.state = "browse" }
        function onRefreshFinished() {browseScreen.refreshing = false }
    }
}
