#ifndef SOFTWAREUPDATER_H
#define SOFTWAREUPDATER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include "filedownloader.h"

struct SWVersion
{
    QString sName;
    QString sPath;
    QString sVersion;
    QString sCode;
    SWVersion(const QString &Name, const QString &Path, const QString &Code) {
       sName = Name;
       sPath = Path;
       sCode = Code;
       sVersion = "0.0.0";
    };
};

class SoftwareUpdater : public QObject
{
    Q_OBJECT
public:
    explicit SoftwareUpdater(QObject *parent = 0);
    ~SoftwareUpdater();
    void setDeviceID(const QString &deviceID);
signals:
    void restartRequired();
    void checkFinish();
public slots:
    void sendRequest();
    void Response(QNetworkReply* reply);
    void downloadFinished(int nCode);
    void downloadFail();
private:
    QNetworkAccessManager * networkManager;

    QNetworkReply *reply;

    QString sDeviceID;

    int currentModuleID;
    QString sTargetPath;
    bool bStart;

    QList<SWVersion*> m_swList={};

    FileDownloader * downloader;

    void AddApplication(const QString& Name, const QString &Path, const QString& Code);
    void GetApplicationVersion(SWVersion *pSW);
};

#endif // SOFTWAREUPDATER_H
