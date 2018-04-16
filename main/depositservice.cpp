#include "depositservice.h"
#include "global.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>


DepositService::DepositService(QObject *parent) : QObject(parent)
{

    networkManager = new QNetworkAccessManager(this);
    bSending = false;
    getList();
    requestTimer = new QTimer(this);

    connect(requestTimer, SIGNAL(timeout()), this, SLOT(requestTimeout()));

    connect(networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(Response(QNetworkReply*)));

}

void DepositService::setNetworkState(bool bState)
{
    if(bCurrentNetworkState != bState)
    {
        bCurrentNetworkState = bState;

        if(bCurrentNetworkState)
        {
            if(!bSending && !requestBodyList.isEmpty())
            {
                bSending = true;
                QTimer::singleShot(0, this, SLOT(sendRequest()));
            }
        }

    }

}

void DepositService::addDeposit(const QString requestBody)
{
    requestBodyList.push_back(requestBody);
    setList();
    if(!bSending && bCurrentNetworkState)
    {
        bSending = true;
        QTimer::singleShot(0, this, SLOT(sendRequest()));
    }

}

void DepositService::setDeviceID(const QString &deviceID)
{

    sDeviceID = deviceID;
}



void DepositService::Response(QNetworkReply* reply)
{
    bool bError = false;
    requestTimer->stop();

    if(reply->error() == QNetworkReply::NoError) {

        QString strReply = (QString)reply->readAll();

        qDebug() << strReply;

        QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());

        QJsonObject jsonObject = jsonResponse.object();


        //qDebug() << "Info result >> " << jsonObject["result"].toString();
        qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("DepositService result :%1").arg(strReply));

    } else {
        qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("Deposit Service Failed :%1 current request move last").arg( sRecentlyRequestBody));
        bError = true;
        requestBodyList.push_back(sRecentlyRequestBody);
        setList();
    }

    reply->deleteLater();
    nwReply = NULL;
    if(bCurrentNetworkState)
    {

        if(requestBodyList.isEmpty())
        {
            bSending = false;
        }
        else
        {
            QTimer::singleShot((bError)?100:0, this, SLOT(sendRequest()));
        }
    }
    else
    {
        bSending = false;
    }
}

void DepositService::sendRequest()
{

    if(!requestBodyList.isEmpty())
    {
        sRecentlyRequestBody = requestBodyList.takeFirst();
        setList();
        QUrl url("http://nephron.superbin.co.kr/api/waste/reserve.php");
        QNetworkRequest req(url);
        QByteArray jsonPost = sRecentlyRequestBody.toLocal8Bit();
        //qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("image uploading request :%1").arg(QString(jsonPost)));
        req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json; charset=utf-8"));
        req.setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(jsonPost.size()));

        //Sending the Request


        nwReply = networkManager->post(req,jsonPost);

        qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("Deposit Service Request :%1").arg( sRecentlyRequestBody));

        requestTimer->start(REQ_TIMEOUT);

    }



}

void DepositService::requestTimeout()
{
    qLog(LOGLEVEL_VERY_HIGH, "Main", "Error", QString("Deposit Service Reply request timeout"));

    if(nwReply)
    {
        nwReply->abort();
    }

}

void DepositService::getList()
{
    QSettings settings("NTN", "Nephron");
    requestBodyList = settings.value("Remains").toStringList();

    qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("DepositService Remains Count :%1").arg(requestBodyList.count()));

}


void DepositService::setList()
{
    QSettings settings("NTN", "Nephron");
    settings.setValue("Remains",requestBodyList);
    settings.sync();
}
