#include "imageuploader.h"
#include <QFile>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "global.hpp"

ImageUploader::ImageUploader(int backupMode, QObject *parent) : QObject(parent)
{

    networkManager = new QNetworkAccessManager(this);
    bSending = false;
    nBackupMode = backupMode;

    connect(networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(Response(QNetworkReply*)));

}

void ImageUploader::setDeviceID(const QString &deviceID)
{

    sDeviceID = deviceID;
}



void ImageUploader::uploadImage(const QString category, const QString &sPath, const QString &sDate)
{
    UploadReq *ureq = new UploadReq(category, sPath,sDate);

    if(ureq)
    {
        m_uploadReq.push_back(ureq);

        if(!bSending)
        {
            bSending = true;
            QTimer::singleShot(0, this, SLOT(sendRequest()));
        }
    }

}


void ImageUploader::Response(QNetworkReply* reply)
{


    if(reply->error() == QNetworkReply::NoError) {

        QString strReply = (QString)reply->readAll();

        qDebug() << strReply;

        QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());

        QJsonObject jsonObject = jsonResponse.object();


        //qDebug() << "Info result >> " << jsonObject["result"].toString();
        qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("Image Upload result :%1").arg(strReply));

        if(nBackupMode==0)
        {
            if (QFile::exists(sDelImagePath)) {
                // already exists, don't overwrite
               QFile deleteFile(sDelImagePath);
               deleteFile.remove();
            }
        }

    } else {
        qDebug() << "Image Upload ERROR";
    }


    reply->deleteLater();


    if(m_uploadReq.isEmpty())
    {
        bSending = false;
    }
    else
    {
        QTimer::singleShot(0, this, SLOT(sendRequest()));
    }
}

void ImageUploader::sendRequest()
{
    QDateTime currentDatetime = QDateTime::currentDateTime();

    if(!m_uploadReq.isEmpty())
    {
        UploadReq * a = m_uploadReq.takeFirst();

        QUrl url("http://nephron.superbin.co.kr/api/device/recoImgUp.php");
        QNetworkRequest req(url);
        //Creating the JSON-Data

        QJsonObject jsonObject;

        jsonObject["deviceId"] = sDeviceID;
        jsonObject["datetime"]= a->sDate;

        jsonObject["code"]= a->category;

        sDelImagePath = a->sPath;

        QFile file(a->sPath);

        file.open(QIODevice::ReadOnly);

        QByteArray loadedArray(file.readAll());

        jsonObject["image"] = QString(loadedArray.toBase64());


        QByteArray jsonPost = QJsonDocument(jsonObject).toJson();

        //qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("image uploading request :%1").arg(QString(jsonPost)));
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
