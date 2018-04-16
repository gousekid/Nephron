#ifndef PUBLISHER_H
#define PUBLISHER_H

#include "nzmqt.hpp"
#include "ipcbase.h"
#include <QObject>

class Publisher : public IPCBase
{
    Q_OBJECT
    typedef IPCBase super;
public:
    explicit Publisher(ZMQContext& context, const QString& address, const QString& identity, QObject* parent = 0)
            : super(parent)
            , address_(address)
            , socket_(0)
    {
        socket_ = context.createSocket(ZMQSocket::TYP_PUB, this);
        socket_->setObjectName("Publisher.Socket.socket(PUB)");
        socket_->setIdentity(identity);
    }

    void sendMessage(const QString& topic,const QString & message);
    void sendMessage(const QByteArray& topic,const QByteArray & message);
    void sendMessage(const QByteArray& topic,const QList<QByteArray> & message);
signals:

    void messageSent(const QList<QByteArray>& message);

protected:
    void startImpl();

protected slots:


private:
    QString address_;
    ZMQSocket* socket_;
};

#endif // PUBLISHER_H
