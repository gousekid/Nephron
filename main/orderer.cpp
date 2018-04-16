#include "orderer.h"
#include <QTimer>
#include <QFile>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "global.hpp"

Orderer::Orderer(QObject *parent) : QObject(parent)
{
    networkManager = new QNetworkAccessManager(this);
    bSending = false;;


    connect(networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(Response(QNetworkReply*)));
}



void Orderer::setDeviceID(const QString &deviceID)
{

    sDeviceID = deviceID;
}


void Orderer::sendOrder( const QString &command,const QString &parameter)
{

    qDebug() << "sendOrder(" << command <<", " << parameter << ")";
    //qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("Write Alert request (%1,%2)").arg(alertCategory).arg(alertCode));
    orderReq *order = new orderReq(command, parameter);

    if(order)
    {
        m_orderReq.push_back(order);

        if(!bSending)
        {
            bSending = true;
            QTimer::singleShot(0, this, SLOT(sendRequest()));
        }
    }


    qDebug() << "order procedure stacked >> " << m_orderReq.count();
}


void Orderer::Response(QNetworkReply* reply)
{


    if(reply->error() == QNetworkReply::NoError) {

        QString strReply = (QString)reply->readAll();

        qDebug() << strReply;

        QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());

        QJsonObject jsonObject = jsonResponse.object();


        //qDebug() << "Info result >> " << jsonObject["result"].toString();
        qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("Orderer result :%1").arg(jsonObject["result"].toString()));

        //qDebug() << "name >> " << jsonObject["name"].toString();
        //qDebug() << "sno >> " << jsonObject["sno"].toString();
        //qDebug() << "ucode >> " << jsonObject["ucode"].toString();
        //qDebug() << "cash >> " << jsonObject["cash"].toString();
        //qDebug() << "point >> " << jsonObject["point"].toString();

    } else {
        qDebug() << "Send order ERROR";
    }

    //delete reply;
    reply->deleteLater();


    if(m_orderReq.isEmpty())
    {
        bSending = false;
    }
    else
    {
        QTimer::singleShot(0, this, SLOT(sendRequest()));
    }
}




void Orderer::sendRequest()
{
    QDateTime currentDatetime = QDateTime::currentDateTime();

    if(!m_orderReq.isEmpty())
    {
        orderReq * a = m_orderReq.takeFirst();

        QUrl url("http://nephron.superbin.co.kr/api/waste/collect.php");
        QNetworkRequest req(url);
        //Creating the JSON-Data

        QJsonObject jsonObject;

        jsonObject["deviceId"] = sDeviceID;
        jsonObject["datetime"]= QString::number(currentDatetime.toTime_t());

        QByteArray jsonPost = QJsonDocument(jsonObject).toJson();
        qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("collect waste Request :%1").arg(QString(jsonPost)));


        req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json; charset=utf-8"));
        req.setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(jsonPost.size()));

        //Sending the Request


        QNetworkReply *reply = networkManager->post(req,jsonPost);

        if(a)
        {
            delete a;
        }
    }



}
