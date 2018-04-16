#ifndef IMAGEUPLOADER_H
#define IMAGEUPLOADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>

struct UploadReq
{
    QString category;
    QString sPath;
    QString sDate;
    UploadReq(const QString &imgCategory, const QString &imagePath, const QString &date) {
       category = imgCategory;
       sPath = imagePath;
       sDate = date;
    }
};

class ImageUploader : public QObject
{
    Q_OBJECT
public:
    explicit ImageUploader(int backupMode, QObject *parent = 0);
    void uploadImage(const QString category, const QString &sPath, const QString &sDate);
    void setDeviceID(const QString &deviceID);
signals:

public slots:
    void sendRequest();
    void Response(QNetworkReply* reply);

private:
    QList<UploadReq*> m_uploadReq={};

    QNetworkAccessManager * networkManager;

    QNetworkReply *reply;

    QString sDeviceID;

    bool bSending;

    int nBackupMode;

    QString sDelImagePath;

};

#endif // IMAGEUPLOADER_H
