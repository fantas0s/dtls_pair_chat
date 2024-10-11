#pragma once

#include <Handshake.h>
#include <UdpConnection.h>

#include <QHostAddress>
#include <QObject>
#include <QTimer>

namespace dtls_pair_chat {
class Connection : public QObject
{
    Q_OBJECT
public:
    enum class State { Idle, Connecting, Connected, Failed };
    enum class AbortReason { Timeout, User, SecureConnectFail };
    explicit Connection();

    // Setters
    void localIpAddress(const QHostAddress &address);
    void localPassword(QStringView password);
    void remoteIpAddress(QStringView address);
    void remotePassword(QStringView password);

    // Control
    void connectToRemote();
    void abortConnection(AbortReason reason);

    // Status
    bool loginInfoSet() const;
    State state() const;
    int percentComplete() const;
    QString currentStep() const;
    QString errorDescription() const;

signals:
    void stateChanged();
    void progressUpdated();
    void remoteIpInvalid();
    void errorDescriptionChanged();

private slots:
    void initialHandshakeDone(QUuid clientUuid, bool isServer);
    void secureChannelOpenError(QDtlsError error);
    void secureChannelOpened(bool isSecure);
    void timeoutTick();

private:
    enum class Step {
        WaitingLoginData,
        SenderReceiverHandshake,
        OpeningSecureChannel,
        ExchangingPasswords
    };
    static QString toString(QDtlsError error);
    static constexpr int s_defaultTimeout{60};
    Step m_step{Step::WaitingLoginData};
    State m_state{State::Idle};
    QDtlsError m_secureChannelError{QDtlsError::NoError};
    int m_remainingSeconds{s_defaultTimeout};
    QHostAddress m_localIp;
    QHostAddress m_remoteIp;
    QString m_localPassword;
    QString m_remotePassword;
    QString m_errorDescription;
    QTimer m_timeoutTimer;
    std::unique_ptr<Handshake> m_handshaker;
    std::shared_ptr<UdpConnection> m_udpConnection;
    int m_percentComplete{0};
};
} // namespace dtls_pair_chat
