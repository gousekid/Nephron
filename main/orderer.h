#ifndef ORDERER_H
#define ORDERER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>


struct orderReq
{
    QString command;
    QString parameter;

    orderReq(const QString &sCommand, const QString &sParameter) {
       command = sCommand;
       parameter = sParameter;
    }
};

class Orderer : public QObject
{
    Q_OBJECT
public:
    explicit Orderer(QObject *parent = 0);

    void setDeviceID(const QString &deviceID);
    void sendOrder(const QString &command,const QString &parameter);


private:

    QNetworkAccessManager * networkManager;

    QNetworkReply *reply;

    QList<orderReq*> m_orderReq={};
    QString sDeviceID;

    bool bSending;


signals:

public slots:
    void sendRequest();
    void Response(QNetworkReply* reply);

};

#endif // ORDERER_H
