#include <Handshake.h>
#include <UdpMessage.h>

using namespace dtls_pair_chat;

Handshake::Handshake(std::shared_ptr<UdpConnection> udpConnection)
    : QObject{nullptr}
    , m_udpConnection{udpConnection}
{}

void Handshake::start()
{
    if (m_state != State::Idle) {
        qWarning() << "Multiple calls to start!";
        return;
    }
    /* Start handshake */
    m_state = State::WaitingAckForSentUuid;
    connect(m_udpConnection.get(),
            &UdpConnection::messageReceived,
            this,
            &Handshake::messageReceived);
    m_udpConnection->sendMessageToRemote(UdpMessage{m_myId});
}

void Handshake::messageReceived(const UdpMessage &receivedMessage)
{
    switch (receivedMessage.type()) {
    case UdpMessage::Type::SendUuid:
        if (m_state == State::WaitingAckForSentUuid) {
            // remote end did not receive our message and has sent his ID.
            // Acknowledge the ID and wait for ack from remote. Remote will be the Server.
            m_state = State::WaitingAckForAck;
            m_udpConnection->sendMessageToRemote(UdpMessage{m_myId, receivedMessage.senderUuid()});
        } else {
            qWarning() << "new UUID received in middle of handshake";
        }
        break;
    case UdpMessage::Type::AckUuid:
        // We always receive Ack. This is a good place to check version.
        if (!m_remoteVersion.has_value()) {
            m_remoteVersion = receivedMessage.msgVersion();
            if (m_remoteVersion.has_value()) {
                emit versionNumberFromRemote(m_remoteVersion.value());
            }
        }
        if (receivedMessage.payloadUuid() == m_myId) {
            // We sent UUID and received ack with our UUID
            switch (m_state) {
            case State::WaitingAckForSentUuid:
                // Response to our UUID. Acknowledge the Ack. Handshake is complete, this side is the server.
                m_udpConnection->sendMessageToRemote(
                    UdpMessage{m_myId, receivedMessage.senderUuid()});
                finalize(receivedMessage.senderUuid());
                break;
            case State::WaitingAckForAck:
                // We sent ack for remote server UUID and now received ack for ack. Handshake is complete, this side is client.
                finalize(QUuid{});
                break;
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

void Handshake::finalize(const QUuid &remoteUuid)
{
    m_state = State::Complete;
    disconnect(m_udpConnection.get(),
               &UdpConnection::messageReceived,
               this,
               &Handshake::messageReceived);
    // if remote Uuid was not given, we don't need it because this side is the client and it's our UUID
    if (remoteUuid.isNull())
        emit complete(m_myId, false);
    else
        emit complete(remoteUuid, true);
}
