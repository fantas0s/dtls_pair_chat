import QtQuick
import QtQuick.Controls
import QtQuick.Layouts


Dialog {
    id: _dialogRoot
    width: 400
    property bool visibleCondition: false
    property bool isErrorDialog: false
    property alias progress: _progressBar.value
    property string progressDescription: ""
    property string errorDescription: ""
    title: isErrorDialog ? qsTr("Connection failed") : qsTr("Connecting to remote party")
    standardButtons: _dialogRoot.isErrorDialog ? Dialog.Ok : Dialog.Abort
    modal: true
    onVisibleConditionChanged: {
        if (visibleCondition)
        {
            if (!_dialogRoot.opened)
                _dialogRoot.open()
        }
        else
        {
            if (_dialogRoot.opened)
                _dialogRoot.close()
        }
    }
    Item {
        width: parent.width
        height: _layout.implicitHeight
        implicitHeight: _layout.implicitHeight
        ColumnLayout {
            id: _layout
            width: parent.width
            spacing: 12
            ProgressBar {
                id: _progressBar
                Layout.fillWidth: true
                visible: !_dialogRoot.isErrorDialog
            }
            Label {
                Layout.fillWidth: true
                text: _dialogRoot.isErrorDialog ? _dialogRoot.errorDescription : _dialogRoot.progressDescription
            }
        }
    }
}
