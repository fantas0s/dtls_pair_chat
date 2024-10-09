import QtQuick
import QtQuick.Controls

TextField {
    id: _labelRoot
    property bool errorCriteria: false
    property string errorString: ""
    onErrorCriteriaChanged: {
        if (_labelRoot.errorCriteria)
            _errorLabel.text = _labelRoot.errorString
        else
            _errorLabel.text = ""
    }

    Rectangle {
        color: "transparent"
        border.color: "red"
        border.width: 1
        anchors.fill: parent
        visible: _errorLabel.text !== ""
        Label {
            id: _errorLabel
            anchors.top: parent.bottom
            color: "red"
            font.pixelSize: _labelRoot.font.pixelSize * 3 / 4
        }
    }
    // Set error on focus change and clear error on edit
    onEditingFinished: {
        if (_labelRoot.errorCriteria)
            _errorLabel.text = _labelRoot.errorString
    }
    onTextChanged: _errorLabel.text = ""
}
