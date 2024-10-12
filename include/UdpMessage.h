#pragma once

#include <QByteArrayView>
#include <QStringView>
#include <QUuid>

namespace dtls_pair_chat {
class UdpMessage
{
public:
    enum class Type { Unknown, SendUuid, AckUuid, SendPassword, AckPassword, Chat };
    enum class PasswordState { Accepted, Rejected };
    explicit UdpMessage(const QUuid &uuidToUse);                               // Send Uuid constructor
    explicit UdpMessage(const QUuid &uuidOfSender, const QUuid &receiverUuid); // Ack Uuid constructor
    explicit UdpMessage(PasswordState state);                                  // Ack Password constructor
    explicit UdpMessage(QStringView payload, Type messageType = Type::Chat);   // Chat / SendPassword message constructor

    /* received message constructor, will determine the type from byte array content */
    explicit UdpMessage(QByteArrayView receivedMessage);

    /* For sending */
    QByteArray toByteArray() const;

    /* For reading */
    QUuid payloadUuid() const;
    QUuid senderUuid() const;
    Type type() const;
    QString chatMsg() const;
    bool accepted() const;

private:
    QUuid m_payloadUuid;
    QUuid m_senderUuid;
    Type m_type{Type::Unknown};
    QString m_chatMsg;
    bool m_accepted{false};
};
} // namespace dtls_pair_chat
