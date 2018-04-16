#include "infoserve.h"
#include <QDateTime>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>


#include "global.hpp"



InfoServe::InfoServe(QObject *parent) : QObject(parent)
{
    networkManager = new QNetworkAccessManager(this);
    bStop = false;
    nInterval = 100;
    requestTimer = new QTimer(this);
    m_alertReq.clear();

    connect(requestTimer, SIGNAL(timeout()), this, SLOT(requestTimeout()));


    connect(networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(Response(QNetworkReply*)));

}



void InfoServe::setDeviceID(const QString &deviceID)
{
    sDeviceID = deviceID;
}


void InfoServe::setHeartBeatInterval(uint interval)
{
    nInterval = interval;
}


void InfoServe::sendAlert( const QString &alertCategory,const QString &alertCode,const QString &alertDescription,const QByteArray &data )
{

    qDebug() << "sendAlert(" << alertCategory <<", " << alertCode <<", "  << alertDescription << ")";
    //qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("Write Alert request (%1,%2)").arg(alertCategory).arg(alertCode));
    alertReq *alert = new alertReq(alertCategory, alertCode,alertDescription,data);

    if(alert)
    {
        m_alertReq.push_back(alert);
    }

    qDebug() << "alert procedure stacked >> " << m_alertReq.count();
}


void InfoServe::Response(QNetworkReply* reply)
{
    requestTimer->stop();

    if(reply->error() == QNetworkReply::NoError) {

        QString strReply = (QString)reply->readAll();

        qDebug() << strReply;

        QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());

        QJsonObject jsonObject = jsonResponse.object();


        //qDebug() << "Info result >> " << jsonObject["result"].toString();
        qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("Info result :%1").arg(strReply));//arg(jsonObject["result"].toString()));

        //qDebug() << "name >> " << jsonObject["name"].toString();
        //qDebug() << "sno >> " << jsonObject["sno"].toString();
        //qDebug() << "ucode >> " << jsonObject["ucode"].toString();
        //qDebug() << "cash >> " << jsonObject["cash"].toString();
        //qDebug() << "point >> " << jsonObject["point"].toString();
        emit NetworkError(0);

    } else {
        qDebug() << "Network ERROR";
    }

    //emit NetworkError(reply->error());

    //delete reply;
    reply->deleteLater();
    nwReply = NULL;


    if(bStop)
    {
        bStop = false;
    }
    else
    {
        QTimer::singleShot(nInterval, this, SLOT(sendRequest()));
    }
}

void InfoServe::sendRequest()
{
    //rohs edit 20180225 msg
    //qDebug() << "Alert Send Request" << nInterval;
    QDateTime currentDatetime = QDateTime::currentDateTime();

    if(m_alertReq.isEmpty())
    {
        //rohs edit 20180225 msg
        //qDebug() << "m_aleart empty" << endl;

        QUrl url("http://nephron.superbin.co.kr/api/device/heartBeat.php");
        QNetworkRequest req(url);
        //Creating the JSON-Data

        QJsonObject jsonObject;

        jsonObject["deviceId"] = sDeviceID;
        jsonObject["datetime"]= QString::number(currentDatetime.toTime_t());

        QByteArray jsonPost = QJsonDocument(jsonObject).toJson();


        req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json; charset=utf-8"));
        req.setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(jsonPost.size()));

        //Sending the Request


        nwReply= networkManager->post(req,jsonPost);
        if(nwReply)
        {
            connect(nwReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(replyError(QNetworkReply::NetworkError)));
        }
        else
        {
            qLog(LOGLEVEL_VERY_HIGH, "Main", "Error", QString("Heartbeat Sending Error "));

            //emit NetworkError(99);
        }
        //qDebug() << "HeartBeat request : " << jsonPost;

    }
    else
    {

        QUrl url("http://nephron.superbin.co.kr/api/device/alert.php");
        QNetworkRequest req(url);
        //Creating the JSON-Data

        QJsonObject jsonObject;
        QJsonArray alertItems;

        jsonObject["deviceId"] = sDeviceID;
        jsonObject["datetime"]= QString::number(currentDatetime.toTime_t());

        qDebug() << "time " << currentDatetime;

        while(!m_alertReq.isEmpty())
        {
            alertReq * a = m_alertReq.takeFirst();
            QJsonObject alertObject;

            alertObject["cate"]= a->alertCategory;
            alertObject["code"]= a->alertCode;
            alertObject["state"]= a->alertDescription;
            alertItems.append(alertObject);

            if(a)
            {
                delete a;
            }
        }
        jsonObject["alerts"] = alertItems;


        QByteArray jsonPost = QJsonDocument(jsonObject).toJson();


        req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json; charset=utf-8"));
        req.setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(jsonPost.size()));

        //Sending the Request

        qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("alert Info Request :%1").arg(QString(jsonPost)));

        nwReply = networkManager->post(req,jsonPost);
        if(nwReply)
        {
            connect(nwReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(replyError(QNetworkReply::NetworkError)));
        }
        else
        {
            qLog(LOGLEVEL_VERY_HIGH, "Main", "Error", QString("Heartbeat Sending Error "));

            //emit NetworkError(99);
        }


        //qDebug() << "alert request : " << jsonPost;
    }

    requestTimer->start(REQ_TIMEOUT);


}

void InfoServe::replyError(QNetworkReply::NetworkError error)
{
    qLog(LOGLEVEL_VERY_HIGH, "Main", "Error", QString("Heartbeat Reply Error :%1").arg(int(error)));

    emit NetworkError(error);
}

void InfoServe::requestTimeout()
{
    qLog(LOGLEVEL_VERY_HIGH, "Main", "Error", QString("Heartbeat Reply request timeout"));

    if(nwReply)
    {
        nwReply->abort();
    }

}

void InfoServe::start()
{
    QTimer::singleShot(nInterval, this, SLOT(sendRequest()));
}

void InfoServe::stop()
{
    bStop = true;

}
