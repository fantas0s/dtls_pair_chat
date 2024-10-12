#include <ChatMessagesModel.h>
#include <UdpConnection.h>

using namespace dtls_pair_chat;

ChatMessagesModel::ChatMessagesModel()
    : QAbstractListModel{nullptr}
{}

int ChatMessagesModel::rowCount(const QModelIndex &parent) const
{
    return m_messages.size();
}

QVariant ChatMessagesModel::data(const QModelIndex &index, int role) const
{
    if (role == static_cast<int>(Role::MsgText))
        return m_messages.at(index.row());
    else
        return QVariant{};
}

QHash<int, QByteArray> ChatMessagesModel::roleNames() const
{
    QHash<int, QByteArray> returnValue;
    returnValue.insert(static_cast<int>(Role::MsgText), "msgText");
    return returnValue;
}

void ChatMessagesModel::setUdpConnection(std::shared_ptr<UdpConnection> udpConnection)
{
    beginResetModel();
    if (m_udpConnection.get()) {
        disconnect(m_udpConnection.get(),
                   &UdpConnection::messageReceived,
                   this,
                   &ChatMessagesModel::messageReceived);
    }
    m_udpConnection = udpConnection;
    if (udpConnection.get()) {
        connect(udpConnection.get(),
                &UdpConnection::messageReceived,
                this,
                &ChatMessagesModel::messageReceived);
    }
    endResetModel();
}

void ChatMessagesModel::sendMessage(const QString &message)
{
    insertNewMessage(message, Direction::Outgoing);
    m_udpConnection->sendMessageToRemote(UdpMessage{message});
}

void ChatMessagesModel::messageReceived(const UdpMessage &message)
{
    if (message.type() == UdpMessage::Type::Chat) {
        insertNewMessage(message.chatMsg(), Direction::Incoming);
    }
}

void ChatMessagesModel::insertNewMessage(QStringView message, Direction direction)
{
    /* We use HTML formatting so escape all HTML tags. */
    QString formattedMessage{message.toString().toHtmlEscaped()};
    /* replace line breaks with their counterparts. */
    formattedMessage.replace(QStringLiteral("\n"), QStringLiteral("<br/>"));
    /* Add prefix. */
    if (direction == Direction::Incoming)
        formattedMessage.prepend(tr("<b>They:</b> "));
    else
        formattedMessage.prepend(tr("<b>You:</b> "));
    beginInsertRows(QModelIndex{}, 0, 0);
    m_messages.prepend(formattedMessage);
    endInsertRows();
}
