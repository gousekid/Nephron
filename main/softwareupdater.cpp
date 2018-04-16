#include "softwareupdater.h"
#include <QTimer>
#include <QDateTime>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcess>

#include "global.hpp"
#define MAX_APPLICATION_LIST    7

SoftwareUpdater::SoftwareUpdater(QObject *parent) : QObject(parent)
{
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(Response(QNetworkReply*)));
    currentModuleID  = 0;
    //on-the-air programming for auto-update
//    AddApplication("Main", "/home/superbin/Nephron/bin/main","00");
//    AddApplication("Classifier", "/home/superbin/Nephron/bin/classifier","01");
//    AddApplication("Plc", "/home/superbin/Nephron/bin/plc","02");
//    AddApplication("UI", "/home/superbin/Nephron/bin/ui","03");
//    AddApplication("Model", "/home/superbin/Nephron/data","05");
//    AddApplication("Resource", "/home/superbin/Nephron/resource","06");

    QTimer::singleShot(0, this, SLOT(sendRequest()));
}
SoftwareUpdater::~SoftwareUpdater()
{


}

void SoftwareUpdater::AddApplication(const QString& Name, const QString &Path, const QString& Code)
{
    SWVersion *sw = new SWVersion(Name, Path,Code);

    if(sw)
    {
        GetApplicationVersion(sw);
        qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("SoftwareUpdater add Application(%1) version : %2").arg(sw->sName).arg(sw->sVersion));

        m_swList.push_back(sw);

    }

}

void SoftwareUpdater::GetApplicationVersion(SWVersion *pSW)
{
    QProcess dummyProcess;
    QStringList params;
    if((pSW->sPath.compare("/home/superbin/Nephron/data") == 0) ||
            (pSW->sPath.compare("/home/superbin/Nephron/resource") == 0))
    {
        QString sTarget = pSW->sPath + "/version";
        params << sTarget;
        qDebug() << "get version string command -->> cat " << params;
        dummyProcess.start("cat", params);
    }
    else
    {
        qDebug() << "get version string command -->> "<< pSW->sPath << " " << params;

        params << "-v";
        dummyProcess.start(pSW->sPath, params);
    }

    dummyProcess.waitForFinished(); // sets current thread to sleep and waits for pingProcess end
    pSW->sVersion = QString(dummyProcess.readAllStandardOutput()).simplified();
    dummyProcess.close();
}

void SoftwareUpdater::setDeviceID(const QString &deviceID)
{

    sDeviceID = deviceID;
}


void SoftwareUpdater::Response(QNetworkReply* reply)
{
    bool bDownloading = false;

    if(reply->error() == QNetworkReply::NoError) {

        QString strReply = (QString)reply->readAll();

        //rohs edit 20180225 msg
        //qDebug() << strReply;

        QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());

        QJsonObject jsonObject = jsonResponse.object();

        if(jsonObject["result"].toString().compare("0000",Qt::CaseInsensitive) == 0)
        {
            //qDebug() << "Info result >> " << jsonObject["result"].toString();
            if(jsonObject["needUpdate"].toString() == "Y")
            {

                //download contents
                bDownloading = true;
                QString sTarget = "http://nephron.superbin.co.kr" + jsonObject["updateUrl"].toString().replace("\\", "");
                downloader = new FileDownloader(sTarget,
                                                currentModuleID,
                                                jsonObject["md5"].toString().toLocal8Bit(),
                                                jsonObject["sha1"].toString().toLocal8Bit(), this);
                connect(downloader, SIGNAL(downloaded(int)), SLOT(downloadFinished(int)));
                connect(downloader, SIGNAL(downloadFailed()), SLOT(downloadFail()));

                /*
                switch(currentModuleID)
                {
                case 0:

                    break;

                case 1:
                    break;

                case 2:
                    break;

                case 3:
                    break;
                }
                //*/


            }
        }
        currentModuleID++;
        qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("SoftwareUpdater result :%1").arg(strReply));


    } else {

    }


    reply->deleteLater();


    if(!bDownloading)
    {
        if(m_swList.isEmpty())
        {
            emit checkFinish();
        }
        else
        {
            QTimer::singleShot(0, this, SLOT(sendRequest()));
        }
    }
}
void SoftwareUpdater::downloadFail()
{
    if(downloader)
    {
        downloader->deleteLater();
        downloader = NULL;

    }
    if(m_swList.isEmpty())
    {
        emit checkFinish();
    }
    else
    {
        QTimer::singleShot(0, this, SLOT(sendRequest()));
    }

}

void SoftwareUpdater::downloadFinished(int nCode)
{
    if(downloader)
    {
        downloader->saveData(sTargetPath);
        downloader->deleteLater();
        downloader = NULL;


        emit restartRequired();
    }
    if(m_swList.isEmpty())
    {
        emit checkFinish();
    }
    else
    {
        QTimer::singleShot(0, this, SLOT(sendRequest()));
    }

}

void SoftwareUpdater::sendRequest()
{

    if(!m_swList.isEmpty())
    {
        SWVersion * a = m_swList.takeFirst();

        if(a->sVersion.length() > 0)
        {
            QUrl url("http://nephron.superbin.co.kr/api/device/checkUpdate.php");
            QNetworkRequest req(url);
            //Creating the JSON-Data

            QJsonObject jsonObject;

            jsonObject["deviceId"] = sDeviceID;
            jsonObject["module"]= a->sCode;
            jsonObject["version"]= a->sVersion;
            sTargetPath = a->sPath;

            QByteArray jsonPost = QJsonDocument(jsonObject).toJson();

            //qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("image uploading request :%1").arg(QString(jsonPost)));
            req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json; charset=utf-8"));
            req.setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(jsonPost.size()));

            //Sending the Request


            QNetworkReply *reply = networkManager->post(req,jsonPost);
        }
        else
        {
            qLog(LOGLEVEL_VERY_HIGH, "Main", "Error", QString("SoftwareUpdater Application version string error(%1) version : %2").arg(a->sName).arg(a->sVersion));

        }

        if(a)
        {
            delete a ;
        }

    }



}
