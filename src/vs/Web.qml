import QtQuick
import QtQuick.Controls

Loader {
    anchors.fill: parent
    
    // Function to compare version strings properly
    function compareVersions(version1, version2) {
        var v1parts = version1.split('.');
        var v2parts = version2.split('.');
        var maxLength = Math.max(v1parts.length, v2parts.length);
        
        for (var i = 0; i < maxLength; i++) {
            var v1part = parseInt(v1parts[i] || '0');
            var v2part = parseInt(v2parts[i] || '0');
            
            if (v1part < v2part) return -1;
            if (v1part > v2part) return 1;
        }
        return 0;
    }
    
    // Extract Qt version from buildString (format: "Qt version 6.x.x")
    function getQtVersion() {
        var buildStr = virtualstudio.buildString;
        var match = buildStr.match(/Qt version (\d+\.\d+\.\d+)/);
        return match ? match[1] : "6.0.0"; // fallback to 6.0.0 if not found
    }
    
    // Use OldWebEngine.qml for versions before 6.7.0 (which don't have onDesktopMediaRequested)
    // Use WebEngine.qml for 6.7.0 and later (which support onDesktopMediaRequested)
    source: compareVersions(getQtVersion(), "6.7.0") >= 0 ? "WebEngine.qml" : "OldWebEngine.qml"

    // TODO: Add support for QtWebView
    // source: useWebEngine ? "WebEngine.qml" : "WebView.qml"
}
