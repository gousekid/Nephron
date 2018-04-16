#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include "ipcbase.h"

#include "zmsg.h"

class Client : public IPCBase
{
    Q_OBJECT
    typedef IPCBase super;
public:

    explicit Client(ZMQContext& context, const QString &identity, int verbose, QObject* parent = 0)
        : super(parent)
        , m_socket(0)
    {

        m_socket = context.createSocket(ZMQSocket::TYP_REQ, this);
        m_verbose = verbose;
        m_identity = identity;
        m_socket->setIdentity(identity);

        connect(m_socket, SIGNAL(messageReceived(const QList<QByteArray>&)), SLOT(messageReceived(const QList<QByteArray>&)));

    }

    virtual
    ~Client()
    {

    }

    void sendRequest(const QString&  service, const QString &option);
    void connectToBroker();
protected:
    void startImpl();
private:
    ZMQContext *m_context;                  //  0MQ context
    ZMQSocket *m_socket;                    //  Socket for broker
    int m_verbose;
    int m_liveness;
    QString m_identity;
    QString m_service;

signals:
    void responseReceived(const QString& service, const QString& msgbody);

private slots:
    void messageReceived(const QList<QByteArray>& message);

};

#endif // CLIENT_H
