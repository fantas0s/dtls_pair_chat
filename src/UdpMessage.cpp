#include <UdpMessage.h>

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

using namespace dtls_pair_chat;

// Message version
static constexpr auto s_versionString = QLatin1String{"1.0.0"};

// XML Elements
static constexpr auto s_xmlId_payload = QLatin1String{"DTLSCHATPAYLOAD"};
static constexpr auto s_xmlId_chatMsg = QLatin1String{"CHATMSG"};
static constexpr auto s_xmlId_ackUuid = QLatin1String{"ACKUUID"};
static constexpr auto s_xmlId_senderId = QLatin1String{"SENDERID"};
static constexpr auto s_xmlId_payloadId = QLatin1String{"PAYLOADID"};
static constexpr auto s_xmlId_sendUuid = QLatin1String{"SENDUUID"};
static constexpr auto s_xmlId_ackPassword = QLatin1String{"ACKPASSWORD"};
static constexpr auto s_xmlId_sendPassword = QLatin1String{"SENDPASSWORD"};
// XML Attributes
static constexpr auto s_xmlAttrId_version = QLatin1String{"version"};
static constexpr auto s_xmlAttrId_accepted = QLatin1String{"accepted"};

std::optional<QVersionNumber> UdpMessage::s_supportedVersion;

UdpMessage::UdpMessage(const QUuid &uuidToUse)
    : m_senderUuid{uuidToUse}
    , m_type{Type::SendUuid}
    , m_msgVersion{QVersionNumber::fromString(s_versionString)}
{}

UdpMessage::UdpMessage(const QUuid &uuidOfSender, const QUuid &receiverUuid)
    : m_payloadUuid{receiverUuid}
    , m_senderUuid{uuidOfSender}
    , m_type{Type::AckUuid}
    , m_msgVersion{QVersionNumber::fromString(s_versionString)}
{}

UdpMessage::UdpMessage(PasswordState state)
    : m_type{Type::AckPassword}
    , m_accepted{state == PasswordState::Accepted}
    , m_msgVersion{QVersionNumber::fromString(s_versionString)}
{}

UdpMessage::UdpMessage(QStringView payload, Type messageType)
    : m_type{messageType}
    , m_chatMsg{payload.toString()}
    , m_msgVersion{QVersionNumber::fromString(s_versionString)}
{
    Q_ASSERT(!payload.isEmpty());
    Q_ASSERT(messageType == Type::Chat || messageType == Type::SendPassword);
}

UdpMessage::UdpMessage(QByteArrayView receivedMessage)
{
    QXmlStreamReader reader{receivedMessage.toByteArray()};
    if (!reader.atEnd()) {
        reader.readNext();
        if (!reader.atEnd() && reader.isStartDocument()) {
            reader.readNextStartElement();
            if (!reader.atEnd() && reader.name() == s_xmlId_payload) {
                const auto version = QVersionNumber::fromString(
                    reader.attributes().value(s_xmlAttrId_version));
                if (!version.isNull())
                    m_msgVersion = version;
                // Until handhake is done, we allow versionless messages.
                if (!s_supportedVersion.has_value() || versionAccepted(m_msgVersion)) {
                    if (!reader.atEnd() && reader.readNextStartElement()) {
                        if (reader.name() == s_xmlId_chatMsg) {
                            m_chatMsg = reader.readElementText();
                            m_type = Type::Chat;
                        } else if (reader.name() == s_xmlId_sendPassword) {
                            m_chatMsg = reader.readElementText();
                            m_type = Type::SendPassword;
                        } else if (reader.name() == s_xmlId_ackPassword) {
                            m_accepted = reader.attributes().value(s_xmlAttrId_accepted)
                                         == QStringLiteral("true");
                            m_type = Type::SendPassword;
                        } else if (reader.name() == s_xmlId_ackUuid) {
                            if (!reader.atEnd()) {
                                bool elementFound{false};
                                do {
                                    elementFound = reader.readNextStartElement();
                                    if (reader.name() == s_xmlId_senderId) {
                                        m_senderUuid = QUuid::fromString(reader.readElementText());
                                    } else if (reader.name() == s_xmlId_payloadId) {
                                        m_payloadUuid = QUuid::fromString(reader.readElementText());
                                    }
                                } while (elementFound);
                                if (!m_payloadUuid.isNull() && !m_senderUuid.isNull())
                                    m_type = Type::AckUuid;
                            }
                        } else if (reader.name() == s_xmlId_sendUuid) {
                            m_senderUuid = QUuid::fromString(reader.readElementText());
                            if (!m_senderUuid.isNull())
                                m_type = Type::SendUuid;
                        }
                    }
                }
            }
        }
    }
}

std::optional<QVersionNumber> UdpMessage::supportedVersion()
{
    return s_supportedVersion;
}

void UdpMessage::resetSupportedVersion()
{
    s_supportedVersion = std::nullopt;
}

void UdpMessage::setSupportedVersion(const QVersionNumber &version)
{
    s_supportedVersion = version;
}

QVersionNumber UdpMessage::localVersion()
{
    return QVersionNumber::fromString(s_versionString);
}

std::optional<QVersionNumber> UdpMessage::msgVersion() const
{
    return m_msgVersion;
}

QByteArray UdpMessage::toByteArray() const
{
    QByteArray returnValue;
    if (m_type != Type::Unknown) {
        QXmlStreamWriter writer{&returnValue};
        writer.setAutoFormatting(true);
        writer.writeStartDocument();
        writer.writeStartElement(s_xmlId_payload);
        if (s_supportedVersion.has_value())
            writer.writeAttribute(s_xmlAttrId_version, s_supportedVersion->toString());
        else
            writer.writeAttribute(s_xmlAttrId_version, s_versionString);
        switch (m_type) {
        case Type::Chat:
            writer.writeTextElement(s_xmlId_chatMsg, m_chatMsg);
            break;
        case Type::AckUuid:
            writer.writeStartElement(s_xmlId_ackUuid);
            writer.writeTextElement(s_xmlId_senderId, m_senderUuid.toString());
            writer.writeTextElement(s_xmlId_payloadId, m_payloadUuid.toString());
            writer.writeEndElement(); //s_xml_ackUuid
            break;
        case Type::SendUuid:
            writer.writeTextElement(s_xmlId_sendUuid, m_senderUuid.toString());
            break;
        case Type::SendPassword:
            writer.writeTextElement(s_xmlId_sendPassword, m_chatMsg);
            break;
        case Type::AckPassword:
            writer.writeEmptyElement(s_xmlId_ackPassword);
            if (m_accepted)
                writer.writeAttribute(s_xmlAttrId_accepted, QStringLiteral("true"));
            else
                writer.writeAttribute(s_xmlAttrId_accepted, QStringLiteral("false"));
            break;
        default:
            break;
        }
        writer.writeEndElement(); // s_xml_payloadId
        writer.writeEndDocument();
    }
    return returnValue;
}

QUuid UdpMessage::payloadUuid() const
{
    return m_payloadUuid;
}

QUuid UdpMessage::senderUuid() const
{
    return m_senderUuid;
}

UdpMessage::Type UdpMessage::type() const
{
    return m_type;
}

QString UdpMessage::chatMsg() const
{
    return m_chatMsg;
}

bool UdpMessage::accepted() const
{
    return m_accepted;
}

QString UdpMessage::typeAsString() const
{
    switch (type()) {
    case Type::SendUuid:
        return QStringLiteral("SendUuid");
    case Type::AckUuid:
        return QStringLiteral("AckUuid");
    case Type::SendPassword:
        return QStringLiteral("SendPassword");
    case Type::AckPassword:
        return QStringLiteral("AckPassword");
    case Type::Chat:
        return QStringLiteral("Chat");
    case Type::Unknown:
        return QStringLiteral("Unknown");
    default:
        return QStringLiteral("Unsupported");
    }
}

bool UdpMessage::versionAccepted(const std::optional<QVersionNumber> &receivedVersion)
{
    return receivedVersion.has_value() && s_supportedVersion.has_value()
           && receivedVersion->majorVersion() == s_supportedVersion->majorVersion()
           && receivedVersion->minorVersion() <= s_supportedVersion->minorVersion();
}
