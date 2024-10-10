#include <ServerClientResolver.h>
#include <UdpMessage.h>

#include <QNetworkDatagram>

using namespace dtls_pair_chat;

ServerClientResolver::ServerClientResolver(const QHostAddress &myAddress,
                                           const QHostAddress &remoteAddress)
    : QObject{nullptr}
    , m_myAddress{myAddress}
    , m_remoteAddress{remoteAddress}
{}

void ServerClientResolver::start()
{
    if (m_state != State::Idle) {
        qWarning() << "Multiple calls to start!";
        return;
    }
    m_receiveSocket.bind(m_myAddress, s_chatPort);
    connect(&m_receiveSocket,
            &QUdpSocket::readyRead,
            this,
            &ServerClientResolver::readPendingMessage);
    /* Start handshake */
    m_state = State::WaitingAckForSentUuid;
    sendMessageToRemote(UdpMessage{m_myId, UdpMessage::Type::SendServerUuid});
}

void ServerClientResolver::readPendingMessage()
{
    while (m_receiveSocket.hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_receiveSocket.receiveDatagram();
        if (datagram.senderAddress() != m_remoteAddress) {
            qWarning() << "Unexpected message ignored.";
        } else {
            const UdpMessage receivedMessage{datagram.data()};
            qDebug() << "Received" << static_cast<int>(receivedMessage.type());
            switch (receivedMessage.type()) {
            case UdpMessage::Type::SendServerUuid:
                if (m_state == State::WaitingAckForSentUuid) {
                    // remote end did not receive our message and has sent his server ID. Remote end is now server.
                    // Acknowledge the server and wait for ack from remote
                    m_state = State::WaitingAckForAck;
                    sendMessageToRemote(
                        UdpMessage{receivedMessage.serverId(), UdpMessage::Type::AckServerUuid});
                } else {
                    qWarning() << "new server UUID received in middle of handshake";
                }
                break;
            case UdpMessage::Type::AckServerUuid:
                switch (m_state) {
                case State::WaitingAckForSentUuid:
                    // We sent server UUid and received ack. This end is server.
                    // acknowledge the Ack. Handshake is complete
                    sendMessageToRemote(UdpMessage{m_myId, UdpMessage::Type::AckServerUuid});
                    finalize(true);
                    return; // do not process anything after finalize
                case State::WaitingAckForAck:
                    // We sent ack for remote server UUID and now received ack for ack.
                    // Remote end is server. Handshake is complete.
                    finalize(false);
                    return; // do not process anything after finalize
                default:
                    qWarning() << "Ack received in wrong phase of the handshake";
                    break;
                }
                break;
            default:
                qWarning() << "Unexpected message type" << static_cast<int>(receivedMessage.type())
                           << "received during handshake";
                break;
            }
        }
    }
}

void ServerClientResolver::sendMessageToRemote(const UdpMessage &message)
{
    m_sendSocket.writeDatagram(message.toByteArray(), m_remoteAddress, s_chatPort);
}

void ServerClientResolver::finalize(bool isServer)
{
    m_state = State::Complete;
    disconnect(&m_receiveSocket,
               &QUdpSocket::readyRead,
               this,
               &ServerClientResolver::readPendingMessage);
    emit complete(isServer);
}
