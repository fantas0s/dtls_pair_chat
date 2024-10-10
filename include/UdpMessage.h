#pragma once

#include <QStringView>
#include <QUuid>
#include <QByteArrayView>

namespace dtls_pair_chat {
class UdpMessage
{
public:
    enum class Type { Unknown, SendServerUuid, AckServerUuid, Chat };
    UdpMessage(const QUuid &uuid, Type type); // Send/Ack ServerUuid constructor
    UdpMessage(QStringView message);       // Chat constructor
    /* received message constructor, will determine the type */
    UdpMessage(QByteArrayView receivedMessage);

    /* For sending */
    QByteArray toByteArray() const;

    /* For reading */
    QUuid serverId() const;
    Type type() const;
    QString chatMsg() const;
private:
    QUuid m_serverId;
    Type m_type{Type::Unknown};
    QString m_chatMsg;
};
} // namespace dtls_pair_chat
