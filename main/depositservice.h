#ifndef DEPOSITSERVICE_H
#define DEPOSITSERVICE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QTimer>


class DepositService : public QObject
{
    Q_OBJECT
public:
    explicit DepositService(QObject *parent = 0);
    void addDeposit(const QString requestBody);
    void setDeviceID(const QString &deviceID);
    void setNetworkState(bool bState);

signals:

public slots:
    void sendRequest();
    void Response(QNetworkReply* reply);
    void requestTimeout();

private:
    QStringList requestBodyList;
    QNetworkAccessManager * networkManager;

    QNetworkReply *nwReply;

    bool bCurrentNetworkState;

    bool bSending;

    QString sDeviceID;

    QString sRecentlyRequestBody;

    QTimer* requestTimer;

    void getList();
    void setList();

};

#endif // DEPOSITSERVICE_H
