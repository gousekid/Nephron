#include "publisher.h"




void Publisher::startImpl()
{
    socket_->bindTo(address_);
}


void Publisher::sendMessage(const QString& topic,const QString & message)
{
    QList< QByteArray > msg;
    msg += topic.toLocal8Bit();
    msg += message.toLocal8Bit();

    socket_->sendMessage(msg);    
    qLog(LOGLEVEL_VERY_HIGH, "Common", "Info", QString("Publish Message :  (%1 : %2)").arg(topic).arg(message));

    emit messageSent(msg);
}

void Publisher::sendMessage(const QByteArray& topic,const QByteArray & message)
{
    //static quint64 counter = 0;

    QList< QByteArray > msg;
    msg += topic;
    msg += message;

    socket_->sendMessage(msg);
    qLog(LOGLEVEL_VERY_HIGH, "Common", "Info", QString("Publish Message :  (%1 : %2)").arg(int(topic.at(0))).arg(int(message.at(0))));

    emit messageSent(msg);
}

void Publisher::sendMessage(const QByteArray& topic,const QList<QByteArray> & message)
{
    //static quint64 counter = 0;

    QList< QByteArray > msg;
    msg += topic;
    msg << message;

    socket_->sendMessage(msg);
    qLog(LOGLEVEL_VERY_HIGH, "Common", "Info", QString("Publish Message :  (%1 : %2)").arg(int(topic.at(0))).arg(message.count()));

    emit messageSent(msg);
}
