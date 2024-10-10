#include <Handshake.h>
#include <UdpMessage.h>

#include <QNetworkDatagram>

using namespace dtls_pair_chat;

Handshake::Handshake(std::shared_ptr<UdpSender> sender, std::shared_ptr<UdpReceiver> receiver)
    : QObject{nullptr}
    , m_receiver{receiver}
    , m_sender{sender}
{}

void Handshake::start()
{
    if (m_state != State::Idle) {
        qWarning() << "Multiple calls to start!";
        return;
    }
    /* Start handshake */
    m_state = State::WaitingAckForSentUuid;
    connect(m_receiver.get(), &UdpReceiver::messageReceived, this, &Handshake::messageReceived);
    m_sender->sendMessageToRemote(UdpMessage{m_myId});
}

void Handshake::messageReceived(const UdpMessage &receivedMessage)
{
    switch (receivedMessage.type()) {
    case UdpMessage::Type::SendUuid:
        if (m_state == State::WaitingAckForSentUuid) {
            // remote end did not receive our message and has sent his ID.
            // Acknowledge the ID and wait for ack from remote
            m_state = State::WaitingAckForAck;
            m_sender->sendMessageToRemote(UdpMessage{m_myId, receivedMessage.senderUuid()});
        } else {
            qWarning() << "new UUID received in middle of handshake";
        }
        break;
    case UdpMessage::Type::AckUuid:
        if (receivedMessage.payloadUuid() == m_myId) {
            // We sent UUID and received ack with our UUID
            switch (m_state) {
            case State::WaitingAckForSentUuid:
                // Response to our UUID. Acknowledge the Ack. Handshake is complete
                m_sender->sendMessageToRemote(UdpMessage{m_myId, receivedMessage.senderUuid()});
                finalize();
                return; // do not process anything after finalize
            case State::WaitingAckForAck:
                // We sent ack for remote server UUID and now received ack for ack. Handshake is complete.
                finalize();
                return; // do not process anything after finalize
            default:
                qWarning() << "Valid formed ack received, but in wrong phase of the handshake";
                break;
            }
        } else {
            qWarning() << "ACK received but it did not contain our UUID";
        }
        break;
    default:
        qWarning() << "Unexpected message type" << static_cast<int>(receivedMessage.type())
                   << "received during handshake";
        break;
    }
}

void Handshake::finalize()
{
    m_state = State::Complete;
    disconnect(m_receiver.get(), &UdpReceiver::messageReceived, this, &Handshake::messageReceived);
    emit complete();
}
