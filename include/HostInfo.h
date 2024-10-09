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
    QHostAddress currentAddress() const;
    QString currentError() const;

signals:
    void addressChanged(QHostAddress newAddress);

private slots:
    void handleHostInfo(const QHostInfo newInfo);

private:
    void timerEvent(QTimerEvent *event) override;
    enum class Error { None, NoAddress, HostNotFound, Unknown };
    QHostAddress m_currentAddress;
    HostInfo::Error m_currentError{Error::NoAddress};
    QString m_currentErrorString{tr("No network connection")};
    int m_lookUpId{-1};
};
} // namespace dtls_pair_chat
