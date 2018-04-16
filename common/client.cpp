#include <QTimer>

#include "client.h"
#include "global.hpp"
#include "zmq.h"

void Client::startImpl()
{
    connectToBroker();
}

void Client::sendRequest(const QString&  service, const QString &option)
{
     zmsg *msg = new zmsg();

    //  Stack protocol envelope to start of message
    if (option.size()>0) {                 //  Optional frame after command
        msg->push_front (option.toLocal8Bit());
    }
    msg->push_front (service.toLocal8Bit());
    msg->push_front (MDPC_CLIENT);

    //  Stack routing envelope to start of message

    if (m_verbose) {
        qDebug() << "I: sending %s to broker" << service;
        qDebug() << "message body: " << msg;
    }

    msg->send (*m_socket);

    delete msg;
}

void Client::messageReceived(const QList<QByteArray> & message)
{
    if (m_verbose) {
        qDebug() << "Client> " << message;
    }

    zmsg * msg = new zmsg(message);

    QString header = QString(msg->pop_front());

//              std::cout << "sbrok, sender: "<< sender << std::endl;
//              std::cout << "sbrok, header: "<< header << std::endl;
//              std::cout << "msg size: " << msg->parts() << std::endl;
//              msg->dump();
    if (QString::compare(header,MDPC_CLIENT) == 0)
    {
        QString service = QString(msg->pop_front());
        QString msgbody = QString(msg->body());
        emit responseReceived(service ,msgbody);
       /* if (service.compare(MDPW_REQUEST) == 0)
        {
            QString sender = msg->unwrap();
            QString msgbody = QString(msg->body());

            emit responseReceived(msgbody);
        }*/

    }
    else {
        if (m_verbose) {
            qDebug() << "Broker> " << message;
        }
        delete msg;
    }
}


void Client::connectToBroker()
{

    m_socket->connectTo(NEURAL_IPC);


    if (m_verbose)
    {
        qDebug() << "I: connecting to broker at  " << NEURAL_IPC;
    }

}
