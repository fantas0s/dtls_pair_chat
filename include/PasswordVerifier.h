#pragma once

#include <UdpConnection.h>

#include <QFlags>
#include <QObject>

namespace dtls_pair_chat {
class UdpMessage;

class PasswordVerifier : public QObject
{
    Q_OBJECT
public:
    explicit PasswordVerifier(std::shared_ptr<UdpConnection> receiver,
                              QStringView localPassword,
                              QStringView remotePassword);
    void start();

signals:
    void complete(bool passwordsMatch);

private slots:
    void messageReceived(const UdpMessage &receivedMessage);

private:
    void tryFinalize();
    enum class ReceivedMessage { None = 0x00, Password = 0x01, Ack = 0x02, Both = 0x03 };
    Q_DECLARE_FLAGS(ReceivedMessages, ReceivedMessage)
    QString m_localPassword;
    QString m_remotePassword;
    ReceivedMessages m_received{ReceivedMessage::None};
    bool m_passwordsMatch{true}; // will be set false if either condition fails
    bool m_running{false};
    std::shared_ptr<UdpConnection> m_udpConnection;
};
}; // namespace dtls_pair_chat
