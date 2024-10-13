import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import dtls_pair_chat 1.0 as DTLSPC

Pane {
    id: _loginRoot
    GridLayout {
        anchors.centerIn: parent
        columns: 2
        rowSpacing: 16
        Label {
            text: DTLSPC.ConnectionSettings.isIp6 ? qsTr("Your IP (v6) address:") : qsTr("Your IP address")
        }
        TextField {
            enabled: false
            text: DTLSPC.ConnectionSettings.thisMachineIpAddress
            Layout.preferredWidth: 320
        }

        Label {
            text: qsTr("Friend's IP Address:")
        }
        TextFieldWithErrorLabel {
            id: _remoteIpTextField
            placeholderText: qsTr("e.g. 2001:0db8:85a3:0000:0000:8a2e:0370:7334 or 127.0.0.1")
            Layout.preferredWidth: 320
            errorString: qsTr("Remote IP was not valid")
            onEditingFinished: DTLSPC.ConnectionSettings.setRemoteIp(text)
            onTextChanged: errorCriteria = false
            Component.onCompleted: text = DTLSPC.ConnectionSettings.getRemoteIp()
            Connections {
                target: DTLSPC.ConnectionSettings
                function onRemoteIpInvalid() {
                    _remoteIpTextField.errorCriteria = true
                }
            }
        }

        Label {
            text: qsTr("Password from friend:")
        }
        TextFieldWithErrorLabel {
            placeholderText: qsTr("Enter here the password your friend gave to you")
            Layout.preferredWidth: 320
            onEditingFinished: _localPasswordTextField.compareString = text
            onTextChanged: DTLSPC.ConnectionSettings.setRemotePassword(text)
            Component.onCompleted: text = DTLSPC.ConnectionSettings.getRemotePassword()
        }

        Label {
            text: qsTr("Your secret password:")
        }
        TextFieldWithErrorLabel {
            id: _localPasswordTextField
            property string compareString: ""
            placeholderText: qsTr("Enter here the password you gave to your friend")
            Layout.preferredWidth: 320
            errorCriteria: text.length > 0 && text === compareString
            errorString: qsTr("Passwords cannot be the same!")
            onTextChanged: DTLSPC.ConnectionSettings.setLocalPassword(text)
            Component.onCompleted: text = DTLSPC.ConnectionSettings.getLocalPassword()
        }

        Button {
            text: qsTr("Connect")
            enabled: DTLSPC.ConnectionSettings.requiredFieldsFilled
            onClicked: DTLSPC.ConnectionSettings.createConnection()
        }
    }
}
