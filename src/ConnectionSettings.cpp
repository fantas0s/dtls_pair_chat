#include <ConnectionSettings.h>

#include <ChatMessagesModel.h>
#include <ConnectionHandler.h>
#include <HostInfo.h>

using namespace dtls_pair_chat;

ConnectionSettings::ConnectionSettings(QObject *parent)
    : QObject{parent}
    , m_chatModel{std::make_unique<ChatMessagesModel>()}
    , m_connectionHandler{std::make_unique<ConnectionHandler>()}
    , m_hostInfo{std::make_unique<HostInfo>()}
{
    connect(m_connectionHandler.get(),
            &ConnectionHandler::errorDescriptionChanged,
            this,
            &ConnectionSettings::errorStringChanged);
    connect(m_connectionHandler.get(),
            &ConnectionHandler::stateChanged,
            this,
            &ConnectionSettings::connectionStateChanged);
    connect(m_connectionHandler.get(),
            &ConnectionHandler::progressUpdated,
            this,
            &ConnectionSettings::progressChanged);
    connect(m_connectionHandler.get(),
            &ConnectionHandler::remoteIpInvalid,
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
    m_chatModel->setUdpConnection({});
    m_connectionHandler->abortConnection(ConnectionHandler::AbortReason::User);
}

void ConnectionSettings::createConnection()
{
    m_connectionHandler->connectToRemote();
}

void ConnectionSettings::setRemoteIp(const QString &newIp)
{
    m_connectionHandler->remoteIpAddress(newIp);
    emit requiredFieldsFilledChanged();
}

void ConnectionSettings::setRemotePassword(const QString &newPassword)
{
    m_connectionHandler->remotePassword(newPassword);
    emit requiredFieldsFilledChanged();
}

void ConnectionSettings::setLocalPassword(const QString &newPassword)
{
    m_connectionHandler->localPassword(newPassword);
    emit requiredFieldsFilledChanged();
}

QString ConnectionSettings::errorString() const
{
    return m_connectionHandler->errorDescription();
}

bool ConnectionSettings::isIp6() const
{
    return m_isIp6;
}

qreal ConnectionSettings::progress() const
{
    const qreal percentAsFloat = m_connectionHandler->percentComplete();
    return percentAsFloat / 100.0;
}

QString ConnectionSettings::progressState() const
{
    return m_connectionHandler->currentStep();
}

bool ConnectionSettings::requiredFieldsFilled() const
{
    return m_connectionHandler->loginInfoSet();
}

QString ConnectionSettings::thisMachineIpAddress() const
{
    return m_thisMachineIpAddress;
}

QAbstractItemModel *ConnectionSettings::chatModel() const
{
    return m_chatModel.get();
}

void ConnectionSettings::setThisMachineIpAddress(QHostAddress newAddress)
{
    if (m_hostInfo->currentError().isEmpty()) {
        m_connectionHandler->localIpAddress(newAddress);
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
    switch (m_connectionHandler->state()) {
    case ConnectionHandler::State::Connecting:
        emit connectionStarted();
        break;
    case ConnectionHandler::State::Connected:
        m_chatModel->setUdpConnection(m_connectionHandler->udpConnection());
        emit connectionSuccessful();
        break;
    case ConnectionHandler::State::Failed:
        emit connectionFailed();
        break;
    default:
        // no signal when changing to idle
        break;
    }
}
