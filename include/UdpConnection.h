#pragma once

#include <QDtls>
#include <QObject>
#include <QUuid>

class QUdpSocket;

namespace dtls_pair_chat {
class UdpMessage;

class UdpConnection : public QObject
{
    Q_OBJECT
public:
    explicit UdpConnection(const QHostAddress &myAddress, const QHostAddress &remoteAddress);
    ~UdpConnection();
    void sendMessageToRemote(const UdpMessage &message);
    void switchToSecureConnection(const QUuid &clientUuid, bool isServer);

signals:
    void messageReceived(const UdpMessage &receivedMessage);
    void secureModeChanged(bool isSecure);
    void dtlsError(QDtlsError error);

private slots:
    void readPendingMessage();

private:
    enum class SecureState { Off, Handshake, On };
    static constexpr quint16 s_chatPort{49152};
    QUdpSocket* m_socket;
    QHostAddress m_myAddress;
    QHostAddress m_remoteAddress;
    SecureState m_state{SecureState::Off};
    std::unique_ptr<QDtls> m_dtlsConnection;
};
}; // namespace dtls_pair_chat
