#pragma once

#include <QByteArrayView>
#include <QStringView>
#include <QUuid>
#include <QVersionNumber>

namespace dtls_pair_chat {
class UdpMessage
{
public:
    enum class Type { Unknown, SendUuid, AckUuid, SendPassword, AckPassword, Chat };
    enum class PasswordState { Accepted, Rejected };
    explicit UdpMessage(const QUuid &uuidToUse); // Send Uuid constructor
    explicit UdpMessage(const QUuid &uuidOfSender,
                        const QUuid &receiverUuid); // Ack Uuid constructor
    explicit UdpMessage(PasswordState state);       // Ack Password constructor
    explicit UdpMessage(QStringView payload,
                        Type messageType = Type::Chat); // Chat / SendPassword message constructor

    /* received message constructor, will determine the type from byte array content */
    explicit UdpMessage(QByteArrayView receivedMessage);

    /* versioning
     * Major versions are incompatible.
     * Minor versions can be limited to work within same major version.
     * Micro versions are fully compatible.
     * e.g.
     * 2.n.m and 3.x.y wont work together, no matter what values n, m, x and y have.
     * 1.3.x and 1.5.y work together, but will only use features of 1.3.x.
     * 1.2.n and 1.2.m will be fully compatible regardless of values of n and m.
     */
    static std::optional<QVersionNumber> supportedVersion();
    static void resetSupportedVersion(); // set version to use to undefined
    static void setSupportedVersion(
        const QVersionNumber &version); // set version to use (i.e. limit to this version)
    static QVersionNumber localVersion();
    /* msgVersion can be local or remote, depending on message origin.
     * invalid message returns std::nullopt */
    std::optional<QVersionNumber> msgVersion() const;

    /* For sending */
    QByteArray toByteArray() const;

    /* For reading */
    QUuid payloadUuid() const;
    QUuid senderUuid() const;
    Type type() const;
    QString chatMsg() const;
    bool accepted() const;

private:
    static std::optional<QVersionNumber> s_supportedVersion;
    static bool versionAccepted(const std::optional<QVersionNumber> &receivedVersion);
    QUuid m_payloadUuid;
    QUuid m_senderUuid;
    Type m_type{Type::Unknown};
    QString m_chatMsg;
    bool m_accepted{false};
    std::optional<QVersionNumber> m_msgVersion;
};
} // namespace dtls_pair_chat
