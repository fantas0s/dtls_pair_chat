#include <Connection.h>

#include <QDebug>

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
    m_sender = std::make_shared<UdpSender>(m_remoteIp);
    m_receiver = std::make_shared<UdpReceiver>(m_localIp);
    m_handshaker = std::make_unique<Handshake>(m_sender, m_receiver);
    connect(m_handshaker.get(),
            &Handshake::complete,
            this,
            &Connection::initialHandshakeDone);
    m_remainingSeconds = s_defaultTimeout;
    m_percentComplete = 0;
    emit progressUpdated();
    m_timeoutTimer.start();
    m_handshaker->start();
}

void Connection::abortConnection() {}

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
        return tr("Opening secure channel...");
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

void Connection::initialHandshakeDone(bool isServer)
{
    m_timeoutTimer.stop();
    // delete connection object. That will also close connection.
    m_handshaker.reset();
    m_isServer = isServer;
    m_step = Step::OpeningSecureChannel;
    m_percentComplete = 40; // initial handshake reaches 33%
    emit progressUpdated();
}

void Connection::timeoutTick()
{
    m_remainingSeconds--;
    const qreal secondsCompletedPercentAsReal = static_cast<qreal>(s_defaultTimeout - m_remainingSeconds) / s_defaultTimeout;
    m_percentComplete = qFloor(33.0 * secondsCompletedPercentAsReal); /* 0% to 33% */
    if (m_remainingSeconds > 0) {
        emit progressUpdated();
    } else {
        // delete handshake object. That will also close connection.
        m_handshaker.reset();
        m_timeoutTimer.stop();
        m_errorDescription = tr("Connection timed out.");
        m_state = State::Failed;
        m_step = Step::WaitingLoginData;
        // Also remove sender and receiver as we will go back to data entry
        m_sender.reset();
        m_receiver.reset();
        // emit signals about changes
        emit stateChanged();
        emit progressUpdated();
        emit errorDescriptionChanged();
    }
}
