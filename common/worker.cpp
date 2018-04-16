
#include <QTimer>

#include "worker.h"
#include "global.hpp"

void Worker::startImpl()
{
    reconnectToBroker();
    QTimer::singleShot(HEARTBEAT_INTERVAL, this, SLOT(sendHeartToBroker()));
}


void Worker::sendToBroker(const QString&  command, const QString &option, zmsg *msg)
{
    msg = (msg ? new zmsg(*msg) : new zmsg ());

    //  Stack protocol envelope to start of message
    if (option.size()>0) {                 //  Optional frame after command
        msg->push_front (option.toLocal8Bit());
    }
    msg->push_front (command.toLocal8Bit());
    msg->push_front (MDPW_WORKER);
    msg->push_front ("");
    //  Stack routing envelope to start of message


    if (m_verbose && command.compare(MDPW_HEARTBEAT) != 0) {
        qDebug() << "I: sending to broker" << command;
        qDebug() << "message body: " << msg;
    }
    msg->send (*m_socket);
    delete msg;
}

void Worker::replyRequest (const QString& to , const QString& message)
{
    zmsg *reply = new zmsg();

    // Add client address
    reply->wrap(to, message);

    sendToBroker(MDPW_REPLY, "",reply);
    delete reply;
}

void Worker::messageReceived(const QList<QByteArray> & message)
{
    if (m_verbose) {
        qDebug() << "Worker incomming message : " << message;
    }

    m_liveness = HEARTBEAT_LIVENESS;

    zmsg * msg = new zmsg(message);
    msg->dump();
    //QString sender = QString(msg->pop_front());
    //msg->pop_front(); //empty message
    QString header = QString(msg->pop_front());

    if (m_verbose) {
        qDebug() << "Worker Header > " << header;
    }

    if (header.compare(MDPW_WORKER) == 0)
    {
        QString command = QString(msg->pop_front());
        if (command.compare(MDPW_REQUEST) == 0)
        {
            QString sender = msg->unwrap();
            QString msgbody = QString(msg->body());

            qDebug() << "message from > " << sender;
            qDebug() << "        body > " << msgbody;

            emit requestReceived(sender, msgbody);
            delete msg;
        }
        else if(command.compare(MDPW_HEARTBEAT) == 0)
        {
            delete msg;
        }
        else if(command.compare(MDPW_DISCONNECT) == 0)
        {
            reconnectToBroker();
            delete msg;
        }
    }
    else {
        qDebug() << "not worker message> " << message;
        delete msg;
    }
}

void Worker::sendHeartToBroker()
{

   // qDebug() << "sendHeartToBroker (" << m_liveness << ")";
    if(m_liveness == 0)
    {
        sleep(HEARTBEAT_INTERVAL);
        reconnectToBroker();
    }
    else
    {
        sendToBroker(MDPW_HEARTBEAT, m_service, NULL);
    }

    QTimer::singleShot(HEARTBEAT_INTERVAL, this, SLOT(sendHeartToBroker()));
}


void Worker::reconnectToBroker()
{

    m_socket->connectTo(NEURAL_IPC);


    if (m_verbose)
    {
        qDebug() << "I: connecting to broker at  " << NEURAL_IPC;
    }

    // Register service with broker
    sendToBroker(MDPW_READY, m_service, NULL);

    // If liveness hits zero, queue is considered disconnected
    m_liveness = HEARTBEAT_LIVENESS;


}
