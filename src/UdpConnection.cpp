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
    disconnect(&m_socket, &QUdpSocket::readyRead, this, &UdpConnection::readPendingMessage);
    if (m_dtlsConnection.get()) {
        m_dtlsConnection->shutdown(&m_socket);
    }
    m_dtlsConnection.reset();
    m_socket.abort();
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
    QList<UdpMessage> receivedMessages;
    std::optional<bool> secureMode;
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
            qDebug() << "Received" << receivedMessage.typeAsString();
            receivedMessages.append(receivedMessage);
        } break;
        case SecureState::Handshake: {
            qDebug() << "Received DTLS handshake";
            if (m_dtlsConnection->doHandshake(&m_socket, datagram.data())) {
                if (m_dtlsConnection->handshakeState() == QDtls::HandshakeState::HandshakeComplete) {
                    m_state = SecureState::On;
                    secureMode = true;
                    // do not return, we might have received encrypted datagrams already
                }
                // else keep shaking hands
            } else {
                m_state = SecureState::Off;
                emit dtlsError(m_dtlsConnection->dtlsError());
                secureMode = false;
                // if secure mode failed, we will shut down the socket anyway, but flush rest of the messages.
                continue;
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
            qDebug() << "Received encrypted" << receivedMessage.typeAsString();
            receivedMessages.append(receivedMessage);
        } break;
        }
    }
    // Wait until all datagrams have been processed before emitting messages.
    if (secureMode.has_value())
    {
        emit secureModeChanged(secureMode.value());
    }
    for (const auto& message : receivedMessages)
    {
        emit messageReceived(message);
    }
}
