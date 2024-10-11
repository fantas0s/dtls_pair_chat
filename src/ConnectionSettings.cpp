#include <ConnectionSettings.h>

#include <Connection.h>
#include <HostInfo.h>

using namespace dtls_pair_chat;

ConnectionSettings::ConnectionSettings(QObject *parent)
    : QObject{parent}
    , m_connection{std::make_unique<Connection>()}
    , m_hostInfo{std::make_unique<HostInfo>()}
{
    connect(m_connection.get(),
            &Connection::errorDescriptionChanged,
            this,
            &ConnectionSettings::errorStringChanged);
    connect(m_connection.get(),
            &Connection::stateChanged,
            this,
            &ConnectionSettings::connectionStateChanged);
    connect(m_connection.get(),
            &Connection::progressUpdated,
            this,
            &ConnectionSettings::progressChanged);
    connect(m_connection.get(),
            &Connection::remoteIpInvalid,
            this,
            &ConnectionSettings::remoteIpInvalid);
    connect(m_hostInfo.get(),
            &HostInfo::addressChanged,
            this,
            &ConnectionSettings::setThisMachineIpAddress);
    setThisMachineIpAddress(m_hostInfo->currentAddress());
}

void ConnectionSettings::abortConnection()
{
    m_connection->abortConnection(Connection::AbortReason::User);
}

void ConnectionSettings::createConnection()
{
    m_connection->connectToRemote();
}

void ConnectionSettings::setRemoteIp(const QString &newIp)
{
    m_connection->remoteIpAddress(newIp);
    emit requiredFieldsFilledChanged();
}

void ConnectionSettings::setRemotePassword(const QString &newPassword)
{
    m_connection->remotePassword(newPassword);
    emit requiredFieldsFilledChanged();
}

void ConnectionSettings::setLocalPassword(const QString &newPassword)
{
    m_connection->localPassword(newPassword);
    emit requiredFieldsFilledChanged();
}

QString ConnectionSettings::errorString() const
{
    return m_connection->errorDescription();
}

bool ConnectionSettings::isIp6() const
{
    return m_isIp6;
}

qreal ConnectionSettings::progress() const
{
    const qreal percentAsFloat = m_connection->percentComplete();
    return percentAsFloat / 100.0;
}

QString ConnectionSettings::progressState() const
{
    return m_connection->currentStep();
}

bool ConnectionSettings::requiredFieldsFilled() const
{
    return m_connection->loginInfoSet();
}

QString ConnectionSettings::thisMachineIpAddress() const
{
    return m_thisMachineIpAddress;
}

void ConnectionSettings::setThisMachineIpAddress(QHostAddress newAddress)
{
    if (m_hostInfo->currentError().isEmpty()) {
        m_connection->localIpAddress(newAddress);
        m_isIp6 = newAddress.protocol() == QHostAddress::NetworkLayerProtocol::IPv6Protocol;
        m_thisMachineIpAddress = newAddress.toString();
        emit requiredFieldsFilledChanged();
    } else {
        m_isIp6 = false;
        m_thisMachineIpAddress = tr("Error: %1").arg(m_hostInfo->currentError());
    }
    emit ipAddressChanged();
}

void ConnectionSettings::connectionStateChanged()
{
    // this signal is only received when state actually changed, so we can signal every time
    switch (m_connection->state()) {
    case Connection::State::Connecting:
        emit connectionStarted();
        break;
    case Connection::State::Connected:
        emit connectionSuccessful();
        break;
    case Connection::State::Failed:
        emit connectionFailed();
        break;
    default:
        // no signal when changing to idle
        break;
    }
}
