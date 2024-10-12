#pragma once
#include <UdpMessage.h>

#include <QAbstractListModel>

namespace dtls_pair_chat {
class UdpConnection;

class ChatMessagesModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ChatMessagesModel();
    Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    Q_INVOKABLE QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    void setUdpConnection(std::shared_ptr<UdpConnection> udpConnection);

public slots:
    void sendMessage(const QString &message);

private slots:
    void messageReceived(const UdpMessage &message);

private:
    enum class Role { MsgText = Qt::ItemDataRole::UserRole };
    enum class Direction { Incoming, Outgoing };
    void insertNewMessage(QStringView message, Direction direction);
    QStringList m_messages;
    std::shared_ptr<UdpConnection> m_udpConnection;
};
} // namespace dtls_pair_chat
