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
    returnValue.insert(static_cast<int>(Role::MsgText), "msgtext");
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
    beginInsertRows(QModelIndex{}, 0, 0);
    m_messages.prepend(tr("<b>You:</b> %1").arg(message));
    endInsertRows();
    m_udpConnection->sendMessageToRemote(UdpMessage{message});
}

void ChatMessagesModel::messageReceived(const UdpMessage &message)
{
    if (message.type() == UdpMessage::Type::Chat) {
        beginInsertRows(QModelIndex{}, 0, 0);
        m_messages.prepend(tr("<b>They:</b> %1").arg(message.chatMsg()));
        endInsertRows();
    }
}
