#include <PasswordVerifier.h>
#include <UdpMessage.h>

using namespace dtls_pair_chat;

PasswordVerifier::PasswordVerifier(std::shared_ptr<UdpConnection> udpConnection,
                                   QStringView localPassword,
                                   QStringView remotePassword)
    : QObject{nullptr}
    , m_localPassword{localPassword.toString()}
    , m_remotePassword{remotePassword.toString()}
    , m_udpConnection{udpConnection}
{
    connect(m_udpConnection.get(),
            &UdpConnection::messageReceived,
            this,
            &PasswordVerifier::messageReceived);
}

void PasswordVerifier::start()
{
    if (m_running) {
        qWarning() << "Multiple calls to start!";
        return;
    }
    m_running = true;
    // Send remote password to remote for verification
    m_udpConnection->sendMessageToRemote(
        UdpMessage{m_remotePassword, UdpMessage::Type::SendPassword});
    // We may have received password already, but wait as Ack can not have been received.
}

void PasswordVerifier::messageReceived(const UdpMessage &receivedMessage)
{
    switch (receivedMessage.type()) {
    case UdpMessage::Type::SendPassword: {
        const bool passwordAccepted{receivedMessage.chatMsg() == m_localPassword};
        m_received.setFlag(ReceivedMessage::Password);
        m_passwordsMatch = m_passwordsMatch && passwordAccepted;
        // Send ack
        m_udpConnection->sendMessageToRemote(UdpMessage{passwordAccepted});
        tryFinalize();
    } break;
    case UdpMessage::Type::AckPassword:
        m_received.setFlag(ReceivedMessage::Ack);
        m_passwordsMatch = m_passwordsMatch && receivedMessage.accepted();
        tryFinalize();
        break;
    default:
        // It is possible to receive other messages, ignore them silently.
        break;
    }
}

void PasswordVerifier::tryFinalize()
{
    if (m_running && m_received.testFlags(ReceivedMessages{ReceivedMessage::Both})) {
        // nothing left to do.
        disconnect(m_udpConnection.get(),
                   &UdpConnection::messageReceived,
                   this,
                   &PasswordVerifier::messageReceived);
        emit complete(m_passwordsMatch);
    }
}
