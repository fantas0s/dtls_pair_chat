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
            &HostInfo::addressesChanged,
            this,
            &ConnectionSettings::setThisMachineIpAddresses);
    setThisMachineIpAddresses(m_hostInfo->currentAddresses());
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

QString ConnectionSettings::getRemoteIp() const
{
    return m_connectionHandler->remoteIpAddress();
}

QString ConnectionSettings::getRemotePassword() const
{
    return m_connectionHandler->remotePassword();
}

QString ConnectionSettings::getLocalPassword() const
{
    return m_connectionHandler->localPassword();
}

QString ConnectionSettings::errorString() const
{
    return m_connectionHandler->errorDescription();
}

int ConnectionSettings::localAddressIdx() const
{
    return m_localAddressIdx;
}

void ConnectionSettings::setLocalAddressIdx(int value)
{
    if (m_localAddressIdx != value) {
        m_localAddressIdx = value;
        emit localAddressIdxChanged();
        if (m_localAddressIdx >= 0 && m_localAddressIdx < m_thisMachineIpAddresses.size()) {
            m_connectionHandler->localIpAddress(m_thisMachineIpAddresses.at(m_localAddressIdx));
        } else {
            m_connectionHandler->localIpAddress({});
        }
    }
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

QStringList ConnectionSettings::thisMachineIpAddresses() const
{
    QStringList strings;
    for (const auto &hostAddress : m_thisMachineIpAddresses) {
        strings.append(hostAddress.toString());
    }
    return strings;
}

QAbstractItemModel *ConnectionSettings::chatModel() const
{
    return m_chatModel.get();
}

void ConnectionSettings::setThisMachineIpAddresses(const QList<QHostAddress> &newAddresses)
{
    setLocalAddressIdx(-1); // none selected
    if (m_hostInfo->currentError().isEmpty()) {
        m_thisMachineIpAddresses = newAddresses;
        emit ipAddressesChanged();
        m_connectionHandler->localIpAddress({});
        emit requiredFieldsFilledChanged();
    } else {
        m_thisMachineIpAddresses.clear();
        emit ipAddressesChanged();
    }
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
