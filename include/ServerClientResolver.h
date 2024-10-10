#pragma once

#include <QObject>
#include <QUdpSocket>
#include <QUuid>

namespace dtls_pair_chat {
class UdpMessage;

class ServerClientResolver : public QObject
{
    Q_OBJECT
public:
    explicit ServerClientResolver(const QHostAddress &myAddress, const QHostAddress &remoteAddress);
    void start();

signals:
    void complete(bool isServer);

private slots:
    void readPendingMessage();

private:
    enum class State { Idle, WaitingAckForSentUuid, WaitingAckForAck, Complete };
    void sendMessageToRemote(const UdpMessage &message);
    void finalize(bool isServer);
    static constexpr quint16 s_chatPort{49152};
    QUuid m_myId{QUuid::createUuid()};
    QUdpSocket m_receiveSocket;
    QUdpSocket m_sendSocket;
    QHostAddress m_myAddress;
    QHostAddress m_remoteAddress;
    State m_state{State::Idle};
};
}; // namespace dtls_pair_chat
