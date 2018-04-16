#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include "nzmqt.hpp"
#include "zmsg.h"
#include "ipcbase.h"

class Worker : public IPCBase
{
    Q_OBJECT
    typedef IPCBase super;
public:

    explicit Worker(ZMQContext& context, const QString &service , const QString &identity, int verbose, QObject* parent = 0)
        : super(parent)
        , m_socket(0)
    {

        m_socket = context.createSocket(ZMQSocket::TYP_DEALER, this);
        m_socket->setObjectName("Worker.Socket.socket(DEA)");

        m_verbose = verbose;
        m_identity = identity;
        m_service = service;
        m_socket->setIdentity(m_identity);
        connect(m_socket, SIGNAL(messageReceived(const QList<QByteArray>&)), SLOT(messageReceived(const QList<QByteArray>&)));

    }

    virtual ~Worker()
    {

    }

    void replyRequest (const QString& to , const QString& message);
protected:
    void startImpl();

private:
    ZMQContext *m_context;                  //  0MQ context
    ZMQSocket *m_socket;                    //  Socket for broker
    int m_verbose;
    int m_liveness;
    QString m_identity;
    QString m_service;
    void sendToBroker(const QString&  command, const QString &option, zmsg *msg);
    void reconnectToBroker();


signals:
    void requestReceived(const QString& sender, const QString& message);

private slots:
    void messageReceived(const QList<QByteArray>& message);
    void sendHeartToBroker();

};

#endif // WORKER_H
