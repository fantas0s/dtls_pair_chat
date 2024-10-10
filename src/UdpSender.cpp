#include <UdpMessage.h>
#include <UdpSender.h>

#include <QNetworkDatagram>

using namespace dtls_pair_chat;

UdpSender::UdpSender(const QHostAddress &remoteAddress)
    : m_remoteAddress{remoteAddress}
{}

void UdpSender::sendMessageToRemote(const UdpMessage &message)
{
    m_sendSocket.writeDatagram(message.toByteArray(), m_remoteAddress, s_chatPort);
}
