#include <UdpMessage.h>

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

using namespace dtls_pair_chat;

// XML Elements
static const auto s_xmlId_payload = QStringLiteral("DTLSCHATPAYLOAD");
static const auto s_xmlId_chatMsg = QStringLiteral("CHATMSG");
static const auto s_xmlId_ackUuid = QStringLiteral("ACKUUID");
static const auto s_xmlId_senderId = QStringLiteral("SENDERID");
static const auto s_xmlId_payloadId = QStringLiteral("PAYLOADID");
static const auto s_xmlId_sendUuid = QStringLiteral("SENDUUID");
static const auto s_xmlId_ackPassword = QStringLiteral("ACKPASSWORD");
static const auto s_xmlId_sendPassword = QStringLiteral("SENDPASSWORD");
// XML Attributes
static const auto s_xmlAttrId_accepted = QStringLiteral("accepted");

UdpMessage::UdpMessage(const QUuid &uuidToUse)
    : m_senderUuid{uuidToUse}
    , m_type{Type::SendUuid}
{}

UdpMessage::UdpMessage(const QUuid &uuidOfSender, const QUuid &receiverUuid)
    : m_payloadUuid{receiverUuid}
    , m_senderUuid{uuidOfSender}
    , m_type{Type::AckUuid}
{}

UdpMessage::UdpMessage(bool passwordWasCorrect)
    : m_type{Type::AckUuid}
    , m_accepted{passwordWasCorrect}
{}

UdpMessage::UdpMessage(QStringView payload, Type messageType)
    : m_type{messageType}
    , m_chatMsg{payload.toString()}
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

QByteArray UdpMessage::toByteArray() const
{
    QByteArray returnValue;
    if (m_type != Type::Unknown) {
        QXmlStreamWriter writer{&returnValue};
        writer.setAutoFormatting(true);
        writer.writeStartDocument();
        writer.writeStartElement(s_xmlId_payload);
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
