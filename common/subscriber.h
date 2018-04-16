#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

#include <QObject>
#include "ipcbase.h"
class Subscriber : public IPCBase
{

    Q_OBJECT
    typedef IPCBase super;

public:
    explicit Subscriber(ZMQContext& context, const QString& address, const QString& indentity, QObject *parent = 0)
        : super(parent)
        , address_(address)
        , socket_(0)
    {
        socket_ = context.createSocket(ZMQSocket::TYP_SUB, this);
        socket_->setObjectName("Subscriber.Socket.socket(SUB)");
        socket_->setIdentity(indentity);

        connect(socket_, SIGNAL(messageReceived(const QList<QByteArray>&)), SLOT(messageReceived(const QList<QByteArray>&)));
    }
    void setSubscribe(const QString& topic);
    void unsetSubscribe(const QString& topic);

signals:
    void receivedMessage(const QList<QByteArray>&);

protected:
    void startImpl();

protected slots:
    void messageReceived(const QList<QByteArray>& message);

private:
    QString address_;

    ZMQSocket* socket_;
};

#endif // SUBSCRIBER_H
