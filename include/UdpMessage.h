#pragma once

#include <QByteArrayView>
#include <QStringView>
#include <QUuid>

namespace dtls_pair_chat {
class UdpMessage
{
public:
    enum class Type { Unknown, SendUuid, AckUuid, Chat };
    UdpMessage(const QUuid &uuidToUse);                               // Send Uuid constructor
    UdpMessage(const QUuid &uuidOfSender, const QUuid &receiverUuid); // Ack Uuid constructor
    UdpMessage(QStringView message);                                  // Chat message constructor

    /* received message constructor, will determine the type from byte array content */
    UdpMessage(QByteArrayView receivedMessage);

    /* For sending */
    QByteArray toByteArray() const;

    /* For reading */
    QUuid payloadUuid() const;
    QUuid senderUuid() const;
    Type type() const;
    QString chatMsg() const;

private:
    QUuid m_payloadUuid;
    QUuid m_senderUuid;
    Type m_type{Type::Unknown};
    QString m_chatMsg;
};
} // namespace dtls_pair_chat
