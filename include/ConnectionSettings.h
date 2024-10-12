#pragma once

#include <QHostAddress>
#include <QObject>
#include <QQmlEngine>
#include <QAbstractItemModel>

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
    Q_PROPERTY(bool isIp6 READ isIp6 NOTIFY ipAddressChanged FINAL)
    Q_PROPERTY(qreal progress READ progress NOTIFY progressChanged FINAL)
    Q_PROPERTY(QString progressState READ progressState NOTIFY progressChanged FINAL)
    Q_PROPERTY(bool requiredFieldsFilled READ requiredFieldsFilled NOTIFY requiredFieldsFilledChanged FINAL)
    Q_PROPERTY(QString thisMachineIpAddress READ thisMachineIpAddress NOTIFY ipAddressChanged FINAL)
    Q_PROPERTY(QAbstractItemModel* chatModel READ chatModel NOTIFY chatModelChanged FINAL)

public:
    explicit ConnectionSettings(QObject *parent = nullptr);
    // Control
    Q_INVOKABLE void abortConnection();
    Q_INVOKABLE void createConnection();

    // User input
    Q_INVOKABLE void setRemoteIp(const QString &newIp);
    Q_INVOKABLE void setRemotePassword(const QString &newPassword);
    Q_INVOKABLE void setLocalPassword(const QString &newPassword);

    // Property getters and setters
    QString errorString() const;
    bool isIp6() const;
    qreal progress() const;
    QString progressState() const;
    bool requiredFieldsFilled() const;
    QString thisMachineIpAddress() const;
    QAbstractItemModel* chatModel() const;

signals:
    // property signals
    void errorStringChanged();
    void ipAddressChanged();
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
    void setThisMachineIpAddress(QHostAddress newAddress);
    void connectionStateChanged();

private:
    bool m_isIp6{false};
    QString m_thisMachineIpAddress;
    std::unique_ptr<ChatMessagesModel> m_chatModel;
    std::unique_ptr<ConnectionHandler> m_connectionHandler;
    std::unique_ptr<HostInfo> m_hostInfo;
};
} // namespace dtls_pair_chat
