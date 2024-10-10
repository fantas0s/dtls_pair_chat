#pragma once

#include <QUdpSocket>
#include <QUuid>

namespace dtls_pair_chat {
class UdpMessage;

class UdpSender
{
public:
    explicit UdpSender(const QHostAddress &remoteAddress);
    void sendMessageToRemote(const UdpMessage &message);

private:
    static constexpr quint16 s_chatPort{49152};
    QUdpSocket m_sendSocket;
    QHostAddress m_remoteAddress;
};
}; // namespace dtls_pair_chat
