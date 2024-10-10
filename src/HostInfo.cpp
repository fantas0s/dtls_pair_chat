#include <QTimerEvent>
#include <HostInfo.h>

using namespace dtls_pair_chat;
HostInfo::HostInfo()
    : QObject{nullptr}
{
    const auto hostName = QHostInfo::localHostName();
    if (hostName.isEmpty()) {
        startTimer(1000, Qt::TimerType::VeryCoarseTimer);
    } else {
        QHostInfo::lookupHost(hostName, this, &dtls_pair_chat::HostInfo::handleHostInfo);
    }
}

QHostAddress HostInfo::currentAddress() const
{
    return m_currentAddress;
}

QString HostInfo::currentError() const
{
    return m_currentErrorString;
}

void HostInfo::handleHostInfo(const QHostInfo newInfo)
{
    const auto oldErrorString = m_currentErrorString;
    const auto oldError = m_currentError;
    if (newInfo.error() == QHostInfo::HostInfoError::NoError) {
        if (newInfo.addresses().empty()) {
            m_currentError = Error::NoAddress;
        } else {
            std::optional<QHostAddress> foundAddress;
            for (const auto &address : newInfo.addresses()) {
                if (!foundAddress.has_value()
                    || address.protocol() == QHostAddress::NetworkLayerProtocol::IPv6Protocol) {
                    // No address yet or first IPv6 address
                    foundAddress = address;
                }
                if (address.protocol() == QHostAddress::NetworkLayerProtocol::IPv6Protocol)
                    break;
            }
            m_currentError = Error::None;
            m_currentAddress = foundAddress.value();
        }
    } else {
        if (newInfo.error() == QHostInfo::HostInfoError::HostNotFound)
            m_currentError = Error::HostNotFound;
        else
            m_currentError = Error::Unknown;
    }
    switch (m_currentError) {
    case Error::HostNotFound:
        m_currentErrorString = tr("No network connection");
        break;
    case Error::NoAddress:
        m_currentErrorString = tr("No address assigned");
        break;
    case Error::Unknown:
        m_currentErrorString = newInfo.errorString();
    default: // no error
        m_currentErrorString.clear();
        break;
    }
    if (oldError != m_currentError || oldErrorString != m_currentErrorString)
        emit addressChanged(m_currentAddress);
}

void HostInfo::timerEvent(QTimerEvent *event)
{
    const auto hostName = QHostInfo::localHostName();
    if (!hostName.isEmpty()) {
        killTimer(event->timerId());
        QHostInfo::lookupHost(hostName, this, &dtls_pair_chat::HostInfo::handleHostInfo);
    }
}
