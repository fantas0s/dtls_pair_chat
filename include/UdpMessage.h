#pragma once

#include <QByteArrayView>
#include <QStringView>
#include <QUuid>

namespace dtls_pair_chat {
class UdpMessage
{
public:
    enum class Type { Unknown, SendUuid, AckUuid, Chat };
    UdpMessage(const QUuid &uuidToUse, Type type); // Send/Ack Uuid constructor
    UdpMessage(QStringView message);          // Chat constructor
    /* received message constructor, will determine the type */
    UdpMessage(QByteArrayView receivedMessage);

    /* For sending */
    QByteArray toByteArray() const;

    /* For reading */
    QUuid uuid() const;
    Type type() const;
    QString chatMsg() const;

private:
    QUuid m_uuid;
    Type m_type{Type::Unknown};
    QString m_chatMsg;
};
} // namespace dtls_pair_chat
