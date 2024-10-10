#include <ConnectionController.h>

#include <Connection.h>
#include <HostInfo.h>

using namespace dtls_pair_chat;

ConnectionController::ConnectionController(QObject *parent)
    : QObject{parent}
    , m_connection{std::make_shared<Connection>()}
    , m_hostInfo{std::make_unique<HostInfo>()}
{
    connect(m_connection.get(),
            &Connection::errorDescriptionChanged,
            this,
            &ConnectionController::errorStringChanged);
    connect(m_connection.get(),
            &Connection::stateChanged,
            this,
            &ConnectionController::connectionStateChanged);
    connect(m_connection.get(),
            &Connection::progressUpdated,
            this,
            &ConnectionController::progressChanged);
    connect(m_connection.get(),
            &Connection::remoteIpInvalid,
            this,
            &ConnectionController::remoteIpInvalid);
    connect(m_hostInfo.get(),
            &HostInfo::addressChanged,
            this,
            &ConnectionController::setThisMachineIpAddress);
    setThisMachineIpAddress(m_hostInfo->currentAddress());
}

void ConnectionController::abortConnection()
{
    m_connection->abortConnection();
}

void ConnectionController::createConnection()
{
    m_connection->connectToRemote();
}

void ConnectionController::setRemoteIp(const QString &newIp)
{
    m_connection->remoteIpAddress(newIp);
    emit requiredFieldsFilledChanged();
}

void ConnectionController::setRemotePassword(const QString &newPassword)
{
    m_connection->remotePassword(newPassword);
    emit requiredFieldsFilledChanged();
}

void ConnectionController::setLocalPassword(const QString &newPassword)
{
    m_connection->localPassword(newPassword);
    emit requiredFieldsFilledChanged();
}

QString ConnectionController::errorString() const
{
    return m_connection->errorDescription();
}

bool ConnectionController::isIp6() const
{
    return m_isIp6;
}

qreal ConnectionController::progress() const
{
    const qreal percentAsFloat = m_connection->percentComplete();
    return percentAsFloat / 100.0;
}

QString ConnectionController::progressState() const
{
    return m_connection->currentStep();
}

bool ConnectionController::requiredFieldsFilled() const
{
    return m_connection->loginInfoSet();
}

QString ConnectionController::thisMachineIpAddress() const
{
    return m_thisMachineIpAddress;
}

void ConnectionController::setThisMachineIpAddress(QHostAddress newAddress)
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

void ConnectionController::connectionStateChanged()
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
