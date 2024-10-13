#pragma once
#include <QHostAddress>
#include <QHostInfo>
#include <QObject>

namespace dtls_pair_chat {
class HostInfo : public QObject
{
    Q_OBJECT
public:
    explicit HostInfo();
    QList<QHostAddress> currentAddresses() const;
    QString currentError() const;

signals:
    void addressesChanged(const QList<QHostAddress>& newAddresses);

private slots:
    void handleHostInfo(const QHostInfo newInfo);

private:
    void timerEvent(QTimerEvent *event) override;
    enum class Error { None, NoAddress, HostNotFound, Unknown };
    QList<QHostAddress> m_currentAddresses;
    HostInfo::Error m_currentError{Error::NoAddress};
    QString m_currentErrorString{tr("No network connection")};
    int m_lookUpId{-1};
};
} // namespace dtls_pair_chat
