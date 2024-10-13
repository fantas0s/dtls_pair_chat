#include <ConnectionHandler.h>
#include <UdpMessage.h>

using namespace dtls_pair_chat;

ConnectionHandler::ConnectionHandler()
    : QObject{nullptr}
{
    m_timeoutTimer.setTimerType(Qt::TimerType::VeryCoarseTimer);
    m_timeoutTimer.setInterval(std::chrono::seconds{1});
    connect(&m_timeoutTimer, &QTimer::timeout, this, &ConnectionHandler::timeoutTick);
}

QString ConnectionHandler::localPassword()
{
    return m_localPassword;
}

QString ConnectionHandler::remoteIpAddress()
{
    if (m_remoteIp.isNull())
        return {};
    else
        return m_remoteIp.toString();
}

QString ConnectionHandler::remotePassword()
{
    return m_remotePassword;
}

void ConnectionHandler::localIpAddress(const QHostAddress &address)
{
    m_localIp = address;
}

void ConnectionHandler::localPassword(QStringView password)
{
    m_localPassword = password.toString();
}

void ConnectionHandler::remoteIpAddress(QStringView address)
{
    m_remoteIp.setAddress(address.toString());
    if (m_remoteIp.isNull())
        emit remoteIpInvalid();
}

void ConnectionHandler::remotePassword(QStringView password)
{
    m_remotePassword = password.toString();
}

void ConnectionHandler::connectToRemote()
{
    m_state = State::Connecting;
    m_step = Step::SenderReceiverHandshake;
    emit stateChanged();
    m_errorDescription.clear();
    emit errorDescriptionChanged();
    m_udpConnection = std::make_shared<UdpConnection>(m_localIp, m_remoteIp);
    m_handshaker = std::make_unique<Handshake>(m_udpConnection);
    connect(m_handshaker.get(),
            &Handshake::complete,
            this,
            &ConnectionHandler::initialHandshakeDone);
    m_remainingSeconds = s_defaultTimeout;
    m_percentComplete = 0;
    emit progressUpdated();
    m_timeoutTimer.start();
    m_handshaker->start();
}

void ConnectionHandler::abortConnection(AbortReason reason)
{
    // delete handshake objects.
    m_handshaker.reset();
    m_passwordVerifier.reset();
    m_timeoutTimer.stop();
    m_state = State::Failed; // by default aborting means a failure.
    switch (reason) {
    case AbortReason::Timeout:
        switch (m_step) {
        case Step::SenderReceiverHandshake:
            m_errorDescription = tr("Waiting other party timed out.");
            break;
        case Step::OpeningSecureChannel:
            m_errorDescription = tr("Setting up secure connection timed out.");
            break;
        case Step::ExchangingPasswords:
        default:
            m_errorDescription = tr("Exchanging passwords timed out.");
            break;
        }
        break;
    case AbortReason::VersionMismatch:
        m_errorDescription = tr("Local and remote versions of application are incompatible.");
        break;
    case AbortReason::NoVersionFromRemote:
        m_errorDescription = tr("Remote end did not provide version.");
        break;
    case AbortReason::SecureConnectFail:
        if (m_secureChannelError == QDtlsError::NoError)
            m_errorDescription = tr("Secure connection setup failed.");
        else
            m_errorDescription = tr("Secure connection setup failed (%1).")
                                     .arg(toString(m_secureChannelError));
        break;
    case AbortReason::PasswordMismatch:
        m_errorDescription = tr("Password did not match in this or remote end.");
        break;
    default: // AbortReason::User
        m_errorDescription.clear();
        m_state = State::Idle;
        break;
    }
    m_step = Step::WaitingLoginData;
    m_secureChannelError = QDtlsError::NoError;
    // Also remove udpConnection as we will go back to data entry. Lose version info.
    UdpMessage::resetSupportedVersion();
    m_udpConnection.reset();
    // emit signals about changes
    emit stateChanged();
    emit progressUpdated();
    emit errorDescriptionChanged();
}

bool ConnectionHandler::loginInfoSet() const
{
    return !(m_remoteIp.isNull() || m_localIp.isNull() || m_remotePassword.isEmpty()
             || m_localPassword.isEmpty())
           && (m_remotePassword != m_localPassword);
}

ConnectionHandler::State ConnectionHandler::state() const
{
    return m_state;
}

int ConnectionHandler::percentComplete() const
{
    return m_percentComplete;
}

QString ConnectionHandler::currentStep() const
{
    switch (m_step) {
    case Step::WaitingLoginData:
        return tr("Waiting required user input...");
    case Step::SenderReceiverHandshake:
        return tr("Waiting for other party (%n second(s) until timeout)...", "", m_remainingSeconds);
    case Step::OpeningSecureChannel:
        return tr("Opening secure channel (%n second(s) until timeout)...", "", m_remainingSeconds);
    case Step::ExchangingPasswords:
        return tr("Verifying passwords (%n second(s) until timeout)...", "", m_remainingSeconds);
    default:
        qWarning() << "Unhandled step!";
        return tr("Waiting...");
    }
}

QString ConnectionHandler::errorDescription() const
{
    return m_errorDescription;
}

std::shared_ptr<UdpConnection> ConnectionHandler::udpConnection() const
{
    return m_udpConnection;
}

void ConnectionHandler::remoteVersionReceived(const QVersionNumber &version)
{
    const auto localVersion = UdpMessage::localVersion();
    if (version.majorVersion() != localVersion.majorVersion())
        abortConnection(AbortReason::VersionMismatch);
    else if (version.minorVersion() < localVersion.minorVersion())
        UdpMessage::setSupportedVersion(version);
    else
        UdpMessage::setSupportedVersion(UdpMessage::localVersion());
}

void ConnectionHandler::initialHandshakeDone(QUuid clientUuid, bool isServer)
{
    if (UdpMessage::supportedVersion().has_value()) {
        // delete connection object. That will also close connection.
        m_handshaker.reset();
        m_step = Step::OpeningSecureChannel;
        m_percentComplete = 34; // initial handshake reaches 33%
        m_remainingSeconds = s_defaultTimeout;
        emit progressUpdated();

        /* Create password verifier already as password from remote could be
     * sent immediately after secure mode is enabled.
     * Then switch to secure connection.
     */
        m_passwordVerifier = std::make_unique<PasswordVerifier>(m_udpConnection,
                                                                m_localPassword,
                                                                m_remotePassword);
        connect(m_udpConnection.get(),
                &UdpConnection::secureModeChanged,
                this,
                &ConnectionHandler::secureChannelOpened);
        connect(m_udpConnection.get(),
                &UdpConnection::dtlsError,
                this,
                &ConnectionHandler::secureChannelOpenError);
        m_udpConnection->switchToSecureConnection(clientUuid, isServer);
    } else {
        // if no supported version set, abort
        abortConnection(AbortReason::NoVersionFromRemote);
    }
}

void ConnectionHandler::secureChannelOpenError(QDtlsError error)
{
    m_secureChannelError = error;
}

void ConnectionHandler::secureChannelOpened(bool isSecure)
{
    disconnect(m_udpConnection.get(),
               &UdpConnection::secureModeChanged,
               this,
               &ConnectionHandler::secureChannelOpened);
    disconnect(m_udpConnection.get(),
               &UdpConnection::dtlsError,
               this,
               &ConnectionHandler::secureChannelOpenError);
    if (isSecure) {
        m_step = Step::ExchangingPasswords;
        m_percentComplete = 67; // secure channel handshake reaches 67%
        m_remainingSeconds = s_defaultTimeout;
        connect(m_passwordVerifier.get(),
                &PasswordVerifier::complete,
                this,
                &ConnectionHandler::passwordVerificationDone);
        m_passwordVerifier->start();
        emit progressUpdated();
    } else {
        abortConnection(AbortReason::SecureConnectFail);
    }
}

void ConnectionHandler::passwordVerificationDone(bool success)
{
    disconnect(m_passwordVerifier.get(),
               &PasswordVerifier::complete,
               this,
               &ConnectionHandler::passwordVerificationDone);
    if (success) {
        // All done, connected.
        m_passwordVerifier.reset();
        m_percentComplete = 100;
        m_state = State::Connected;
        emit progressUpdated();
        emit stateChanged();
    } else {
        abortConnection(AbortReason::PasswordMismatch);
    }
}

void ConnectionHandler::timeoutTick()
{
    m_remainingSeconds--;
    qreal secondsCompletedPercentAsReal = static_cast<qreal>(s_defaultTimeout - m_remainingSeconds);
    secondsCompletedPercentAsReal /= s_defaultTimeout;
    m_percentComplete = qFloor(33.0 * secondsCompletedPercentAsReal); /* 0% to 33% */
    if (m_step == Step::OpeningSecureChannel)
        m_percentComplete += 34; /* 34% to 67% */
    else if (m_step == Step::ExchangingPasswords)
        m_percentComplete += 67; /* 67% to 100% */
    if (m_remainingSeconds > 0) {
        emit progressUpdated();
    } else {
        abortConnection(AbortReason::Timeout);
    }
}

QString ConnectionHandler::toString(QDtlsError error)
{
    switch (error) {
    case QDtlsError::InvalidInputParameters:
        return tr("Invalid input parameters");
    case QDtlsError::InvalidOperation:
        return tr("Invalid operation");
    case QDtlsError::RemoteClosedConnectionError:
        return tr("Remote closed connection");
    case QDtlsError::PeerVerificationError:
        return tr("Peer verification error");
    default:
        return tr("Code %1").arg(static_cast<int>(error));
    }
}
