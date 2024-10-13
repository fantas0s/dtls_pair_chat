#pragma once

#include <QAbstractItemModel>
#include <QHostAddress>
#include <QObject>
#include <QQmlEngine>

namespace dtls_pair_chat {
class ConnectionHandler;
class HostInfo;
class ChatMessagesModel;

class ConnectionSettings : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged FINAL)
    Q_PROPERTY(int localAddressIdx READ localAddressIdx WRITE setLocalAddressIdx NOTIFY localAddressIdxChanged FINAL)
    Q_PROPERTY(qreal progress READ progress NOTIFY progressChanged FINAL)
    Q_PROPERTY(QString progressState READ progressState NOTIFY progressChanged FINAL)
    Q_PROPERTY(bool requiredFieldsFilled READ requiredFieldsFilled NOTIFY requiredFieldsFilledChanged FINAL)
    Q_PROPERTY(QStringList thisMachineIpAddresses READ thisMachineIpAddresses NOTIFY ipAddressesChanged FINAL)
    Q_PROPERTY(QAbstractItemModel *chatModel READ chatModel NOTIFY chatModelChanged FINAL)

public:
    explicit ConnectionSettings(QObject *parent = nullptr);
    // Control
    Q_INVOKABLE void abortConnection();
    Q_INVOKABLE void createConnection();

    // User input fields
    Q_INVOKABLE void setRemoteIp(const QString &newIp);
    Q_INVOKABLE void setRemotePassword(const QString &newPassword);
    Q_INVOKABLE void setLocalPassword(const QString &newPassword);
    Q_INVOKABLE QString getRemoteIp() const;
    Q_INVOKABLE QString getRemotePassword() const;
    Q_INVOKABLE QString getLocalPassword() const;

    // Property getters and setters
    QString errorString() const;
    int localAddressIdx() const;
    void setLocalAddressIdx(int value);
    qreal progress() const;
    QString progressState() const;
    bool requiredFieldsFilled() const;
    QStringList thisMachineIpAddresses() const;
    QAbstractItemModel *chatModel() const;

signals:
    // property signals
    void errorStringChanged();
    void localAddressIdxChanged();
    void ipAddressesChanged();
    void progressChanged();
    void requiredFieldsFilledChanged();
    void chatModelChanged();

    // connection status
    void connectionStarted();
    void connectionSuccessful();
    void connectionFailed();

    // from connection object
    void remoteIpInvalid();

private slots:
    void setThisMachineIpAddresses(const QList<QHostAddress>& newAddresses);
    void connectionStateChanged();

private:
    int m_localAddressIdx{-1};
    QList<QHostAddress> m_thisMachineIpAddresses;
    std::unique_ptr<ChatMessagesModel> m_chatModel;
    std::unique_ptr<ConnectionHandler> m_connectionHandler;
    std::unique_ptr<HostInfo> m_hostInfo;
};
} // namespace dtls_pair_chat
