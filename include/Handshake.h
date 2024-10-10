#pragma once
#include <UdpReceiver.h>
#include <UdpSender.h>

#include <QObject>
#include <QUuid>

namespace dtls_pair_chat {
class UdpMessage;

class Handshake : public QObject
{
    Q_OBJECT
public:
    explicit Handshake(std::shared_ptr<UdpSender> sender, std::shared_ptr<UdpReceiver> receiver);
    void start();

signals:
    void complete();

private slots:
    void messageReceived(const UdpMessage &receivedMessage);

private:
    enum class State { Idle, WaitingAckForSentUuid, WaitingAckForAck, Complete };
    void finalize();
    QUuid m_myId{QUuid::createUuid()};
    std::shared_ptr<UdpReceiver> m_receiver;
    std::shared_ptr<UdpSender> m_sender;
    State m_state{State::Idle};
};
}; // namespace dtls_pair_chat
