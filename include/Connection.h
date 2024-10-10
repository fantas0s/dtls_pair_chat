#pragma once

#include <QHostAddress>
#include <QObject>
#include <QTimer>
#include <ServerClientResolver.h>

namespace dtls_pair_chat {
class Connection : public QObject
{
    Q_OBJECT
public:
    enum class State { Idle, Connecting, Connected, Failed };
    explicit Connection();

    // Setters
    void localIpAddress(const QHostAddress &address);
    void localPassword(QStringView password);
    void remoteIpAddress(QStringView address);
    void remotePassword(QStringView password);

    // Control
    void connectToRemote();
    void abortConnection();

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
    void initialHandshakeDone(bool isServer);
    void timeoutTick();

private:
    enum class Step {
        WaitingLoginData,
        HostClientRoleResolve,
        OpeningSecureChannel,
        ExchangingPasswords
    };
    static constexpr int s_defaultTimeout{60};
    bool m_isServer{false};
    Step m_step{Step::WaitingLoginData};
    State m_state{State::Idle};
    int m_remainingSeconds{s_defaultTimeout};
    QHostAddress m_localIp;
    QHostAddress m_remoteIp;
    QString m_localPassword;
    QString m_remotePassword;
    QString m_errorDescription;
    QTimer m_timeoutTimer;
    std::unique_ptr<ServerClientResolver> m_serverClientConnect;
};
} // namespace dtls_pair_chat
