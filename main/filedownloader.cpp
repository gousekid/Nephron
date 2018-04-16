#include "filedownloader.h"
#include <QFile>
#include <QDir>
#include <QProcess>
#include "global.hpp"


FileDownloader::FileDownloader(QUrl imageUrl,int code, const QByteArray &md5, const QByteArray& sha1,  QObject *parent) : QObject(parent)
{

    nCode = code;
    m_MD5 = md5;
    m_SHA1 = sha1;

    connect(&m_WebCtrl, SIGNAL (finished(QNetworkReply*)), SLOT (fileDownloaded(QNetworkReply*)));
    qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("fileDownloaded Start(%1)").arg(imageUrl.toString()));

    QNetworkRequest request(imageUrl);
    m_WebCtrl.get(request);
}

FileDownloader::~FileDownloader()
{

}

void FileDownloader::fileDownloaded(QNetworkReply* pReply)
{

    if(pReply->error() == QNetworkReply::NoError) {

        m_DownloadedData = pReply->readAll();
        QByteArray md5 = QCryptographicHash::hash(m_DownloadedData, QCryptographicHash::Md5).toHex();
        qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("md5 hash server(%1) : client(%2)").arg(QString::fromLocal8Bit(m_MD5)).arg(QString::fromLocal8Bit(md5)));

        if( m_MD5 != md5)
        {
            qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("md5 hash key failed"));
            emit downloadFailed();
            return;
        }
        QByteArray sha1 = QCryptographicHash::hash(m_DownloadedData, QCryptographicHash::Sha1).toHex();
        qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("sha1 hash server(%1) : client(%2)").arg(QString::fromLocal8Bit(m_SHA1)).arg(QString::fromLocal8Bit(sha1)));
        if( m_SHA1 != sha1)
        {
            qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("sha1 hash key failed"));
            emit downloadFailed();
            return;
        }


        //qDebug() << "Info result >> " << jsonObject["result"].toString();
        qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("fileDownloaded finish"));
        emit downloaded(nCode);

    } else {
        qLog(LOGLEVEL_VERY_HIGH, "Main", "Error", QString("fileDownloaded failed with code %1").arg(pReply->errorString()));
        emit downloadFailed();
    }


 //emit a signal
 pReply->deleteLater();

}

QByteArray FileDownloader::downloadedData() const
{
 return m_DownloadedData;
}

void FileDownloader::saveData(const QString &sPath)
{
    //file delete
    QString path = sPath;


    if(path.isEmpty())
    {
        return;
    }

    if(sPath.compare("/home/superbin/Nephron/resource")==0)
    {
        //delete temp file

        if (QFile::exists("/home/superbin/Downloads/resource.tar.gz")) {
            // already exists, don't overwrite
           QFile deleteFile("/home/superbin/Downloads/resource.tar.gz");
           deleteFile.remove();
        }
        //save tar to temp foler
        QFile sFile("/home/superbin/Downloads/resource.tar.gz");
        if(!sFile.open(QIODevice::WriteOnly))
        {
            return;
        }
        sFile.write(m_DownloadedData);
        sFile.setPermissions(QFile::ReadOwner|QFile::WriteOwner|QFile::ReadGroup|QFile::WriteGroup|QFile::ReadOther|QFile::WriteOther);
        sFile.close();

        //delete folder

        QDir dir(sPath);
        dir.removeRecursively();


        //extract to target
        QProcess process;

        process.execute("mkdir " + sPath);
        process.waitForFinished();

        process.execute("tar xvfz /home/superbin/Downloads/resource.tar.gz -C " + sPath);
        process.waitForFinished();

    }
    if(sPath.compare("/home/superbin/Nephron/data")==0)
    {
        //delete temp file

        if (QFile::exists("/home/superbin/Downloads/model.tar.gz")) {
            // already exists, don't overwrite
           QFile deleteFile("/home/superbin/Downloads/model.tar.gz");
           deleteFile.remove();
        }
        //save tar to temp foler
        QFile sFile("/home/superbin/Downloads/model.tar.gz");
        if(!sFile.open(QIODevice::WriteOnly))
        {
            return;
        }
        sFile.write(m_DownloadedData);
        sFile.setPermissions(QFile::ReadOwner|QFile::WriteOwner|QFile::ReadGroup|QFile::WriteGroup|QFile::ReadOther|QFile::WriteOther);
        sFile.close();

        //delete folder

        QDir dir(sPath);
        dir.removeRecursively();

        //extract to target
        QProcess process;
        process.execute("mkdir " + sPath);
        process.waitForFinished();

        process.execute("tar xvfz /home/superbin/Downloads/model.tar.gz -C " + sPath);
        process.waitForFinished();

    }
    else
    {
        if (QFile::exists(path)) {
            // already exists, don't overwrite
           QFile deleteFile(sPath);
           deleteFile.remove();
        }

        QFile sFile(path);
        if(!sFile.open(QIODevice::WriteOnly))
        {
            return;
        }
        sFile.write(m_DownloadedData);
        sFile.setPermissions(QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner|QFile::ReadGroup|QFile::WriteGroup|QFile::ExeGroup|QFile::ReadOther|QFile::ExeOther);
        sFile.close();
    }



}

