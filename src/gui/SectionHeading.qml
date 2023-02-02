import QtQuick 2.12
import QtQuick.Controls 2.12

Rectangle {
  property string filterStroke: virtualstudio.darkMode ? "#827D7D" : "#BABCBC"

  property bool listIsEmpty: false
  // required property string section: section (for 5.15)
  color: "transparent"
  height: 72 * virtualstudio.uiScale
  x: 16 * virtualstudio.uiScale
  y: listIsEmpty ? 16 * virtualstudio.uiScale : 0
  width: listIsEmpty ? parent.width - (2 * x) : ListView.view.width - (2 * x)
  Text {
      id: sectionText
      //anchors.bottom: parent.bottom
      y: 12 * virtualstudio.uiScale
      // text: parent.section (for 5.15)
      width: parent.width - 332 * virtualstudio.uiScale
      fontSizeMode: Text.HorizontalFit
      text: listIsEmpty ? "No Studios" : section
      font { family: "Poppins"; pixelSize: 28 * virtualstudio.fontScale * virtualstudio.uiScale; weight: Font.Bold }
      color: textColour
      verticalAlignment: Text.AlignBottom
  }
  Button {
      id: createButton
      background: Rectangle {
          radius: 6 * virtualstudio.uiScale
          color: createButton.down ? "#E7E8E8" : "#F2F3F3"
          border.width: 1
          border.color: createButton.down ? "#B0B5B5" : "#EAEBEB"
          layer.enabled: createButton.hovered && !createButton.down
      }
      onClicked: { virtualstudio.createStudio(); }
      anchors.right: filterButton.left
      anchors.rightMargin: 16
      anchors.verticalCenter: sectionText.verticalCenter
      width: 150 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
      Text {
          text: "Create a Studio"
          font.family: "Poppins"
          font.pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale
          font.weight: Font.Bold
          color: "#DB0A0A"
          anchors.horizontalCenter: parent.horizontalCenter
          anchors.verticalCenter: parent.verticalCenter
      }
      visible: listIsEmpty ? true : (section == virtualstudio.logoSection ? true : false)
  }
  Button {
      id: filterButton
      background: Rectangle {
          radius: 6 * virtualstudio.uiScale
          color: filterButton.down ? "#E7E8E8" : "#F2F3F3"
          border.width: 1
          border.color: filterButton.down ? "#B0B5B5" : "#EAEBEB"
          layer.enabled: filterButton.hovered && !filterButton.down
      }
      onClicked: { filterMenu.open(); }
      anchors.right: parent.right
      anchors.verticalCenter: sectionText.verticalCenter
      width: 150 * virtualstudio.uiScale; height: 30 * virtualstudio.uiScale
      Text {
          text: "Filter Studios"
          font.family: "Poppins"
          font.pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale
          anchors.horizontalCenter: parent.horizontalCenter
          anchors.verticalCenter: parent.verticalCenter
      }
      visible: listIsEmpty ? true : (section == virtualstudio.logoSection ? true : false)

      Popup {
          id: filterMenu
          y: Math.round(parent.height + 8)
          rightMargin: 16 * virtualstudio.uiScale
          width: 210 * virtualstudio.uiScale; height: 64 * virtualstudio.uiScale
          modal: false
          focus: false
          closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
          background: Rectangle {
              radius: 6 * virtualstudio.uiScale
              color: "#F6F8F8"
              border.width: 1
              border.color: filterStroke
              layer.enabled: true
          }
          contentItem: Column {
              anchors.fill: parent
              CheckBox {
                  id: inactiveCheckbox
                  text: qsTr("Show my inactive Studios")
                  checkState: virtualstudio.showInactive ? Qt.Checked : Qt.Unchecked
                  onClicked: { virtualstudio.showInactive = inactiveCheckbox.checkState == Qt.Checked;
                      refreshing = true;
                      refresh();
                  }
                  indicator: Rectangle {
                      implicitWidth: 16 * virtualstudio.uiScale
                      implicitHeight: 16 * virtualstudio.uiScale
                      x: inactiveCheckbox.leftPadding
                      y: parent.height / 2 - height / 2
                      radius: 3 * virtualstudio.uiScale
                      border.color: inactiveCheckbox.down ? "#007AFF" : "#0062cc"

                      Rectangle {
                          width: 10 * virtualstudio.uiScale
                          height: 10 * virtualstudio.uiScale
                          x: 3 * virtualstudio.uiScale
                          y: 3 * virtualstudio.uiScale
                          radius: 2 * virtualstudio.uiScale
                          color: inactiveCheckbox.down ? "#007AFF" : "#0062cc"
                          visible: inactiveCheckbox.checked
                      }
                  }
                  contentItem: Text {
                      text: inactiveCheckbox.text
                      font.family: "Poppins"
                      font.pixelSize: 10 * virtualstudio.fontScale * virtualstudio.uiScale
                      anchors.horizontalCenter: parent.horizontalCenter
                      anchors.verticalCenter: parent.verticalCenter
                      leftPadding: inactiveCheckbox.indicator.width + inactiveCheckbox.spacing
                  }
              }
              CheckBox {
                  id: selfHostedCheckbox
                  text: qsTr("Show self-hosted Studios")
                  checkState: virtualstudio.showSelfHosted ? Qt.Checked : Qt.Unchecked
                  onClicked: { virtualstudio.showSelfHosted = selfHostedCheckbox.checkState == Qt.Checked;
                      refreshing = true;
                      refresh();
                  }
                  indicator: Rectangle {
                      implicitWidth: 16 * virtualstudio.uiScale
                      implicitHeight: 16 * virtualstudio.uiScale
                      x: selfHostedCheckbox.leftPadding
                      y: parent.height / 2 - height / 2
                      radius: 3 * virtualstudio.uiScale
                      border.color: selfHostedCheckbox.down ? "#007AFF" : "#0062CC"

                      Rectangle {
                          width: 10 * virtualstudio.uiScale
                          height: 10 * virtualstudio.uiScale
                          x: 3 * virtualstudio.uiScale
                          y: 3 * virtualstudio.uiScale
                          radius: 2 * virtualstudio.uiScale
                          color: selfHostedCheckbox.down ? "#007AFF" : "#0062CC"
                          visible: selfHostedCheckbox.checked
                      }
                  }
                  contentItem: Text {
                      text: selfHostedCheckbox.text
                      font.family: "Poppins"
                      font.pixelSize: 10 * virtualstudio.fontScale * virtualstudio.uiScale
                      anchors.horizontalCenter: parent.horizontalCenter
                      anchors.verticalCenter: parent.verticalCenter
                      leftPadding: selfHostedCheckbox.indicator.width + selfHostedCheckbox.spacing
                  }
              }
          }
      }
  }
}