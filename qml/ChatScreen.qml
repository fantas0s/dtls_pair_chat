import QtQuick
import QtQuick.Controls
import dtls_pair_chat 1.0 as DTLSPC

Pane {
    ListView {
        id: _chatList
        anchors.left: parent.left
        anchors.right: _scrollBar.left
        anchors.top: parent.top
        anchors.bottom: _editBox.top
        anchors.margins: 8
        verticalLayoutDirection: ListView.BottomToTop
        clip: true
        model: DTLSPC.ConnectionSettings.chatModel
        delegate: ChatListDelegate {
            width: _chatList.width
        }
        ScrollBar.vertical: _scrollBar
    }
    ScrollBar {
        id: _scrollBar
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: _editBox.top
        anchors.margins: 8
        enabled: _chatList.contentHeight > _chatList.height
    }
    Rectangle {
        id: _editBox
        anchors.left: parent.left
        anchors.right: _sendButton.left
        anchors.bottom: parent.bottom
        height: Math.max(_editor.implicitHeight + 8, _sendButton.height)
        anchors.margins: 8
        color: "white"
        border.width: 1
        border.color: "black"
        TextEdit {
            id: _editor
            anchors.fill: parent
            anchors.margins: 4
        }
    }
    Button {
        id: _sendButton
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 8
        text: qsTr("Send")
        onClicked: {
            DTLSPC.ConnectionSettings.chatModel.sendMessage(_editor.text)
            _editor.clear()
        }
    }
}
