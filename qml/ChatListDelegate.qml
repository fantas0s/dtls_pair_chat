import QtQuick
import QtQuick.Controls

Rectangle {
    color: "white"
    implicitHeight: _chatMsg.implicitHeight + 20
    Label {
        id: _chatMsg
        color: "black"
        text: model.msgText
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        wrapMode: Text.WordWrap
    }
}
