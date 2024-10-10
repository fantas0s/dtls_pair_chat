#include <UdpMessage.h>

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

using namespace dtls_pair_chat;

static const auto s_xml_payloadId = QStringLiteral("DTLSCHATPAYLOAD");
static const auto s_xml_chatMsgId = QStringLiteral("CHATMSG");
static const auto s_xml_ackServerId = QStringLiteral("ACKSERVER");
static const auto s_xml_sendServerId = QStringLiteral("SENDSERVER");

UdpMessage::UdpMessage(const QUuid &uuid, Type type)
    : m_serverId{uuid}
    , m_type{type}
{
    Q_ASSERT(type == Type::SendServerUuid || type == Type::AckServerUuid);
}

UdpMessage::UdpMessage(QStringView message)
    : m_type{Type::Chat}
{
    Q_ASSERT(!message.isEmpty());
}

UdpMessage::UdpMessage(QByteArrayView receivedMessage)
{
    QXmlStreamReader reader{receivedMessage.toByteArray()};
    if (!reader.atEnd()) {
        reader.readNext();
        if (!reader.atEnd() && reader.isStartDocument()) {
            reader.readNextStartElement();
            if (!reader.atEnd() && reader.name() == s_xml_payloadId) {
                reader.readNextStartElement();
                if (!reader.atEnd()) {
                    if (reader.name() == s_xml_chatMsgId) {
                        m_chatMsg = reader.readElementText();
                        m_type = Type::Chat;
                    } else if (reader.name() == s_xml_ackServerId) {
                        m_serverId = QUuid::fromString(reader.readElementText());
                        if (!m_serverId.isNull())
                            m_type = Type::AckServerUuid;
                    } else if (reader.name() == s_xml_sendServerId) {
                        m_serverId = QUuid::fromString(reader.readElementText());
                        if (!m_serverId.isNull())
                            m_type = Type::SendServerUuid;
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
        writer.writeStartElement(s_xml_payloadId);
        switch (m_type) {
        case Type::Chat:
            writer.writeTextElement(s_xml_chatMsgId, m_chatMsg);
            break;
        case Type::AckServerUuid:
            writer.writeTextElement(s_xml_ackServerId, m_serverId.toString());
            break;
        case Type::SendServerUuid:
            writer.writeTextElement(s_xml_sendServerId, m_serverId.toString());
            break;
        default:
            break;
        }
        writer.writeEndElement(); // s_xml_payloadId
        writer.writeEndDocument();
    }
    return returnValue;
}

QUuid UdpMessage::serverId() const
{
    return m_serverId;
}

UdpMessage::Type UdpMessage::type() const
{
    return m_type;
}

QString UdpMessage::chatMsg() const
{
    return m_chatMsg;
}
