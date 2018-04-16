#include "subscriber.h"




void Subscriber::startImpl()
{
    socket_->connectTo(address_);
}


void Subscriber::messageReceived(const QList<QByteArray>& message)
{

    emit receivedMessage(message);
}


void Subscriber::setSubscribe(const QString& topic)
{

    socket_->subscribeTo(topic);
}


void Subscriber::unsetSubscribe(const QString& topic)
{

    socket_->unsubscribeFrom(topic);
}
