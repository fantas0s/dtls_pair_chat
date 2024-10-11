#include <Connection.h>

using namespace dtls_pair_chat;

Connection::Connection()
    : QObject{nullptr}
{
    m_timeoutTimer.setTimerType(Qt::TimerType::VeryCoarseTimer);
    m_timeoutTimer.setInterval(std::chrono::seconds{1});
    connect(&m_timeoutTimer, &QTimer::timeout, this, &Connection::timeoutTick);
}

void Connection::localIpAddress(const QHostAddress &address)
{
    m_localIp = address;
}

void Connection::localPassword(QStringView password)
{
    m_localPassword = password.toString();
}

void Connection::remoteIpAddress(QStringView address)
{
    m_remoteIp.setAddress(address.toString());
    if (m_remoteIp.isNull())
        emit remoteIpInvalid();
}

void Connection::remotePassword(QStringView password)
{
    m_remotePassword = password.toString();
}

void Connection::connectToRemote()
{
    m_state = State::Connecting;
    m_step = Step::SenderReceiverHandshake;
    emit stateChanged();
    m_errorDescription.clear();
    emit errorDescriptionChanged();
    m_udpConnection = std::make_shared<UdpConnection>(m_localIp, m_remoteIp);
    m_handshaker = std::make_unique<Handshake>(m_udpConnection);
    connect(m_handshaker.get(), &Handshake::complete, this, &Connection::initialHandshakeDone);
    m_remainingSeconds = s_defaultTimeout;
    m_percentComplete = 0;
    emit progressUpdated();
    m_timeoutTimer.start();
    m_handshaker->start();
}

void Connection::abortConnection(AbortReason reason)
{
    // delete handshake objects.
    m_handshaker.reset();
    m_timeoutTimer.stop();
    switch (reason) {
    case AbortReason::Timeout:
        if (m_step == Step::OpeningSecureChannel)
            m_errorDescription = tr("Setting up secure connection timed out.");
        else
            m_errorDescription = tr("Waiting other party timed out.");
        m_state = State::Failed;
        break;
    case AbortReason::SecureConnectFail:
        if (m_secureChannelError == QDtlsError::NoError)
            m_errorDescription = tr("Secure connection setup failed.");
        else
            m_errorDescription = tr("Secure connection setup failed (%1).")
                                     .arg(toString(m_secureChannelError));
        m_state = State::Failed;
        break;
    default: // AbortReason::User
        m_errorDescription.clear();
        m_state = State::Idle;
        break;
    }
    m_step = Step::WaitingLoginData;
    m_secureChannelError = QDtlsError::NoError;
    // Also remove udpConnection as we will go back to data entry
    m_udpConnection.reset();
    // emit signals about changes
    emit stateChanged();
    emit progressUpdated();
    emit errorDescriptionChanged();
}

bool Connection::loginInfoSet() const
{
    return !(m_remoteIp.isNull() || m_localIp.isNull() || m_remotePassword.isEmpty()
             || m_localPassword.isEmpty())
           && (m_remotePassword != m_localPassword);
}

Connection::State Connection::state() const
{
    return m_state;
}

int Connection::percentComplete() const
{
    return m_percentComplete;
}

QString Connection::currentStep() const
{
    switch (m_step) {
    case Step::WaitingLoginData:
        return tr("Waiting required user input...");
    case Step::SenderReceiverHandshake:
        return tr("Waiting for other party (%n second(s) until timeout)...", "", m_remainingSeconds);
    case Step::OpeningSecureChannel:
        return tr("Opening secure channel (%n second(s) until timeout)...", "", m_remainingSeconds);
    case Step::ExchangingPasswords:
        return tr("Verifying passwords...");
    default:
        qWarning() << "Unhandled step!";
        return tr("Waiting...");
    }
}

QString Connection::errorDescription() const
{
    return m_errorDescription;
}

void Connection::initialHandshakeDone(QUuid clientUuid, bool isServer)
{
    // delete connection object. That will also close connection.
    m_handshaker.reset();
    m_step = Step::OpeningSecureChannel;
    m_percentComplete = 34; // initial handshake reaches 33%
    m_remainingSeconds = s_defaultTimeout;
    emit progressUpdated();

    // Switch to secure connection
    connect(m_udpConnection.get(),
            &UdpConnection::secureModeChanged,
            this,
            &Connection::secureChannelOpened);
    connect(m_udpConnection.get(),
            &UdpConnection::dtlsError,
            this,
            &Connection::secureChannelOpenError);
    m_udpConnection->switchToSecureConnection(clientUuid, isServer);
}

void Connection::secureChannelOpenError(QDtlsError error)
{
    m_secureChannelError = error;
}

void Connection::secureChannelOpened(bool isSecure)
{
    disconnect(m_udpConnection.get(),
               &UdpConnection::secureModeChanged,
               this,
               &Connection::secureChannelOpened);
    disconnect(m_udpConnection.get(),
               &UdpConnection::dtlsError,
               this,
               &Connection::secureChannelOpenError);
    if (isSecure) {
        m_step = Step::ExchangingPasswords;
        m_percentComplete = 70; // secure channel handshake reaches 67%, so advance a bit
        m_remainingSeconds = s_defaultTimeout;
        // TODO: passwordexchange object - sends and receives a password and verifies they match
        emit progressUpdated();
    } else {
        abortConnection(AbortReason::SecureConnectFail);
    }
}

void Connection::timeoutTick()
{
    m_remainingSeconds--;
    qreal secondsCompletedPercentAsReal = static_cast<qreal>(s_defaultTimeout - m_remainingSeconds);
    secondsCompletedPercentAsReal /= s_defaultTimeout;
    m_percentComplete = qFloor(33.0 * secondsCompletedPercentAsReal); /* 0% to 33% */
    if (m_step == Step::OpeningSecureChannel)
        m_percentComplete += 34; /* 34% to 67% */
    if (m_remainingSeconds > 0) {
        emit progressUpdated();
    } else {
        abortConnection(AbortReason::Timeout);
    }
}

QString Connection::toString(QDtlsError error)
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
