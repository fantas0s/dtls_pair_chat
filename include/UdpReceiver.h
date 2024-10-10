#pragma once

#include <QObject>
#include <QUdpSocket>
#include <QUuid>

namespace dtls_pair_chat {
class UdpMessage;

class UdpReceiver : public QObject
{
    Q_OBJECT
public:
    explicit UdpReceiver(const QHostAddress &myAddress, const QHostAddress &remoteAddress);

signals:
    void messageReceived(const UdpMessage &receivedMessage);

private slots:
    void readPendingMessage();

private:
    static constexpr quint16 s_chatPort{49152};
    QUdpSocket m_receiveSocket;
    QHostAddress m_myAddress;
    QHostAddress m_remoteAddress;
};
}; // namespace dtls_pair_chat
