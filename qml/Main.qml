import QtQuick
import QtQuick.Controls
import QtQml.StateMachine as DSM
import dtls_pair_chat 1.0 as DTLSPC

Window {
    id: _mainWindow
    width: 640
    height: 480
    visible: true
    title: qsTr("DTLS Pair Chat")

    // State machine
    signal dialogCanceled
    signal chatExited
    DSM.StateMachine {
        id: _statemachine
        initialState: _stateLogin
        running: true
        DSM.State {
            id: _stateLogin
            onEntered: DTLSPC.ConnectionSettings.abortConnection()
            DSM.SignalTransition {
                targetState: _stateConnectingDialog
                signal: DTLSPC.ConnectionSettings.connectionStarted
            }
        }
        DSM.State {
            id: _stateConnectingDialog
            DSM.SignalTransition {
                targetState: _stateChat
                signal: DTLSPC.ConnectionSettings.connectionSuccessful
            }
            DSM.SignalTransition {
                targetState: _stateFailDialog
                signal: DTLSPC.ConnectionSettings.connectionFailed
            }
            DSM.SignalTransition {
                targetState: _stateLogin
                signal: _connectFailDialog.rejected
            }
        }
        DSM.State {
            id: _stateFailDialog
            DSM.SignalTransition {
                targetState: _stateLogin
                signal: _connectFailDialog.rejected
            }
            DSM.SignalTransition {
                targetState: _stateLogin
                signal: _connectFailDialog.closed
            }
        }
        DSM.State {
            id: _stateChat
            DSM.SignalTransition {
                targetState: _stateLogin
                signal: _mainWindow.chatExited
            }
        }
    }

    // UI elements
    Loader {
        anchors.fill: parent
        source: _stateLogin.active ? "qrc:/qt/qml/dtls_pair_chat/qml/LoginScreen.qml" :
                                     _stateChat.active ? "qrc:/qt/qml/dtls_pair_chat/qml/ChatScreen.qml" :
                                                         ""
    }
    ConnectionDialog {
        id: _connectFailDialog
        x: (parent.width / 2) - (_connectFailDialog.width / 2)
        y: (parent.height / 2) - (_connectFailDialog.height / 2)
        visibleCondition: _stateConnectingDialog.active || _stateFailDialog.active
        isErrorDialog: _stateFailDialog.active
        progress: DTLSPC.ConnectionSettings.progress
        progressDescription: DTLSPC.ConnectionSettings.progressState
        errorDescription: DTLSPC.ConnectionSettings.errorString
    }
}
