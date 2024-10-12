#include <UdpConnection.h>
#include <UdpMessage.h>

#include <QNetworkDatagram>

using namespace dtls_pair_chat;

UdpConnection::UdpConnection(const QHostAddress &myAddress, const QHostAddress &remoteAddress)
    : QObject{nullptr}
    , m_myAddress{myAddress}
    , m_remoteAddress{remoteAddress}
{
    m_socket.bind(m_myAddress, s_chatPort);
    connect(&m_socket, &QUdpSocket::readyRead, this, &UdpConnection::readPendingMessage);
}

UdpConnection::~UdpConnection()
{
    if (m_dtlsConnection.get()) {
        m_dtlsConnection->shutdown(&m_socket);
    }
}

void UdpConnection::sendMessageToRemote(const UdpMessage &message)
{
    switch (m_state) {
    case SecureState::Off:
        m_socket.writeDatagram(message.toByteArray(), m_remoteAddress, s_chatPort);
        break;
    case SecureState::On:
        m_dtlsConnection->writeDatagramEncrypted(&m_socket, message.toByteArray());
        break;
    default:
        qWarning() << "Attempt to send message in middle of handshake";
    }
}

void UdpConnection::switchToSecureConnection(const QUuid &clientUuid, bool isServer)
{
    if (isServer) {
        m_dtlsConnection = std::make_unique<QDtls>(QSslSocket::SslMode::SslServerMode);
        m_dtlsConnection->setPeer(m_remoteAddress, s_chatPort, clientUuid.toString());
        // Wait until handshake from client
    } else {
        m_dtlsConnection = std::make_unique<QDtls>(QSslSocket::SslMode::SslClientMode);
        m_dtlsConnection->setPeer(m_remoteAddress, s_chatPort, clientUuid.toString());
        // Send handshake to server and then wait handshake from server
        m_dtlsConnection->doHandshake(&m_socket);
    }
    m_state = SecureState::Handshake;
}

void UdpConnection::readPendingMessage()
{
    while (m_socket.hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_socket.receiveDatagram();
        if (datagram.senderAddress() != m_remoteAddress) {
            qWarning() << "Message from unexpected sender ignored.";
            continue;
        }
        switch (m_state) {
        case SecureState::Off: {
            const UdpMessage receivedMessage{datagram.data()};
            if (receivedMessage.type() == UdpMessage::Type::Unknown) {
                qWarning() << "Message had invalid content, ignored.";
                // Jump to next message
                continue;
            }
            if (receivedMessage.type() == UdpMessage::Type::Chat) {
                qWarning() << "Unsecured chat message ignored.";
                // Jump to next message
                continue;
            }
            qDebug() << "Received" << static_cast<int>(receivedMessage.type());
            emit messageReceived(receivedMessage);
        } break;
        case SecureState::Handshake: {
            if (m_dtlsConnection->doHandshake(&m_socket, datagram.data())) {
                if (m_dtlsConnection->handshakeState() == QDtls::HandshakeState::HandshakeComplete) {
                    m_state = SecureState::On;
                    emit secureModeChanged(true);
                    // do not return, we might have received encrypted datagrams already
                }
                // else keep shaking hands
            } else {
                m_state = SecureState::Off;
                emit dtlsError(m_dtlsConnection->dtlsError());
                emit secureModeChanged(false);
                // if secure mode failed, we will shut down the socket anyway, so return right away.
                return;
            }
        } break;
        default: // secure mode
        {
            const UdpMessage receivedMessage{
                m_dtlsConnection->decryptDatagram(&m_socket, datagram.data())};
            if (receivedMessage.type() == UdpMessage::Type::Unknown) {
                qWarning() << "Encrypted message had invalid content, ignored.";
                // Jump to next message
                continue;
            }
            qDebug() << "Received encrypted" << static_cast<int>(receivedMessage.type());
            emit messageReceived(receivedMessage);
        } break;
        }
    }
}
