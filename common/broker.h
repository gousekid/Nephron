#ifndef BROKER_H
#define BROKER_H


#include <QQueue>
#include <QMap>
#include "ipcbase.h"
#include "zmsg.h"


typedef struct _server_t server_t;
typedef struct _client_t client_t;




struct service;

//  This defines one worker, idle or active
struct worker
{
    QString m_identity;   //  Address of worker
    service * m_service;      //  Owning service, if known
    uint m_expiry;         //  Expires at unless heartbeat

    worker(QString identity, service* service = 0, uint expiry = 0) {
       m_identity = identity;
       m_service = service;
       m_expiry = expiry;
    }
};

struct service
{
   ~service ()
   {
       for(int i = 0; i < m_requests.size(); i++) {
           delete m_requests.takeFirst();
       }
   }
    QString m_name;             //  Service name
    QQueue<zmsg*> m_requests;   //  List of client requests
    QList<worker*> m_waiting;  //  List of waiting workers
    size_t m_workers;               //  How many workers we have

    service(QString name)
    {
        m_name = name;
    }
};




class Broker : public IPCBase
{
    Q_OBJECT
    typedef IPCBase super;
public:

    explicit Broker(ZMQContext& context, int verbose, QObject* parent = 0)
        : super(parent)
        , m_socket(0)
    {
        qDebug() << "Broker Intialized : verbose(" << verbose << ")";

        m_socket = context.createSocket(ZMQSocket::TYP_ROUTER, this);
        m_verbose = verbose;
        m_socket->setObjectName("Broker.Socket.socket(ROU)");
        connect(m_socket, SIGNAL(messageReceived(const QList<QByteArray>&)), SLOT(messageReceived(const QList<QByteArray>&)));

    }

    virtual
       ~Broker()
       {
           while (! m_services.empty())
           {
               delete m_services.begin().value();
               m_services.erase(m_services.begin());
           }
           while (! m_workers.empty())
           {
               delete m_workers.begin().value();
               m_workers.erase(m_workers.begin());
           }
       }

    void start_brokering();
protected:
    void startImpl();
private:
    ZMQContext *m_context;                  //  0MQ context
    ZMQSocket *m_socket;                    //  Socket for clients & workers
    int m_verbose;                               //  Print activity to stdout
    QString m_endpoint;                      //  Broker binds to this endpoint
    QMap<QString, service*> m_services;  //  Hash of known services
    QMap<QString, worker*> m_workers;    //  Hash of known workers
    QSet<worker*> m_waiting;              //  List of waiting workers

    void purge_workers();
    service * service_require (const QString&  name);
    void service_dispatch (service *srv, zmsg *msg);
    void service_internal (const QString& service_name, zmsg *msg);
    worker * worker_require (const QString&  identity);
    void worker_delete (worker *&wrk, int disconnect);
    void worker_process (const QString&  sender, zmsg *msg);
    void worker_send (worker *worker, const QString& command, const QString&  option, zmsg *msg);
    void worker_waiting (worker *worker);
    void client_process (const QString&  sender, zmsg* msg);
    void bind (const QString& endpoint);
private slots:
    void messageReceived(const QList<QByteArray>& message);
    void sendHeartToWorker();
};


#endif // BROKER_H
