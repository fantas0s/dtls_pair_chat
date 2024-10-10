#include <UdpMessage.h>
#include <UdpReceiver.h>

#include <QNetworkDatagram>

using namespace dtls_pair_chat;

UdpReceiver::UdpReceiver(const QHostAddress &myAddress, const QHostAddress &remoteAddress)
    : QObject{nullptr}
    , m_myAddress{myAddress}
    , m_remoteAddress{remoteAddress}
{
    m_receiveSocket.bind(m_myAddress, s_chatPort);
    connect(&m_receiveSocket, &QUdpSocket::readyRead, this, &UdpReceiver::readPendingMessage);
}

void UdpReceiver::readPendingMessage()
{
    while (m_receiveSocket.hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_receiveSocket.receiveDatagram();
        if (datagram.senderAddress() != m_remoteAddress) {
            qWarning() << "Message from unexpected sender ignored.";
            return;
        }
        const UdpMessage receivedMessage{datagram.data()};
        if (receivedMessage.type() == UdpMessage::Type::Unknown) {
            qWarning() << "Message had invalid content, ignored.";
            return;
        }
        qDebug() << "Received" << static_cast<int>(receivedMessage.type());
        emit messageReceived(receivedMessage);
    }
}
