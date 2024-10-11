#pragma once

#include <UdpConnection.h>

#include <QObject>
#include <QUuid>

namespace dtls_pair_chat {
class UdpMessage;

class Handshake : public QObject
{
    Q_OBJECT
public:
    explicit Handshake(std::shared_ptr<UdpConnection> receiver);
    void start();

signals:
    void complete(QUuid clientUuid, bool isServer);

private slots:
    void messageReceived(const UdpMessage &receivedMessage);

private:
    enum class State { Idle, WaitingAckForSentUuid, WaitingAckForAck, Complete };
    void finalize(const QUuid &remoteUuid);
    QUuid m_myId{QUuid::createUuid()};
    std::shared_ptr<UdpConnection> m_udpConnection;
    State m_state{State::Idle};
};
}; // namespace dtls_pair_chat
