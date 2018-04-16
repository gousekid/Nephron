#ifndef INFOSERVE_H
#define INFOSERVE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QTimer>


struct alertReq
{
    QString alertCategory;
    QString alertCode;
    QString alertDescription;
    QByteArray alertData;


    alertReq(const QString &category, const QString &code, const QString &description, const QByteArray &data) {
       alertCategory = category;
       alertCode = code;
       alertDescription = description;
       alertData = data;
    }
};

class InfoServe : public QObject
{
    Q_OBJECT
public:
    explicit InfoServe(QObject *parent = 0);

    void setDeviceID(const QString &deviceID);
    void setHeartBeatInterval(uint interval);

    void sendAlert(const QString &alertCategory,const QString &alertCode,const QString &alertDescription,const QByteArray &data );

    void start();
    void stop();



private:
    uint nInterval;

    QNetworkAccessManager * networkManager;

    QNetworkReply *nwReply;



    QList<alertReq*> m_alertReq={};
    QString sDeviceID;

    bool bStop;

    QTimer* requestTimer;

signals:
    void NetworkError(uint);
public slots:

    void sendRequest();
    void Response(QNetworkReply* reply);
    void replyError(QNetworkReply::NetworkError error);


    void requestTimeout();


};

#endif // INFOSERVE_H
