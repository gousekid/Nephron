#ifndef MAINAPPLICATON_H
#define MAINAPPLICATON_H


#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QSettings>
#include <QProcess>
#include <QTimer>

#include "broker.h"
#include "worker.h"
#include "publisher.h"
#include "subscriber.h"
#include "infoserve.h"
#include "imageuploader.h"
#include "orderer.h"
#include "softwareupdater.h"
#include "depositservice.h"

class QNetworkReply;

struct NephronSetting{
    QString deviceID;
    uint    deviceType;

};

class MainApplication: public QCoreApplication
{
    Q_OBJECT

    typedef QCoreApplication super;

public:
    explicit MainApplication(int& argc, char** argv);
    ~MainApplication();
    bool notify(QObject *obj, QEvent *event);




signals:
    void finished();
    void failure(const QString& what);


protected slots:
    void run();
    void receivedMessage(const QList<QByteArray>& message);
    void requestReceived(const QString& sender, const QString& message);
    void requestDeposit(const QString& sender, const QString& requestBody);

    void loginResponse(QNetworkReply* reply);
    void depositResponse(QNetworkReply* reply);

    void sendRequest();

    void launchApps();

    void setRestartRequired();
    void updateCheckFinish();

    void updateCheck();
    void rebootCmd();

    void processPlcFinishied(int exitCode, QProcess::ExitStatus exitStatus);
    void processClassifierFinishied(int exitCode, QProcess::ExitStatus exitStatus);
    void processUIFinishied(int exitCode, QProcess::ExitStatus exitStatus);
    void processPrinterFinishied(int exitCode, QProcess::ExitStatus exitStatus);

    void processPlcError(QProcess::ProcessError error);
    void processClassifierError(QProcess::ProcessError error);
    void processUIError(QProcess::ProcessError error);
    void processPrinterError(QProcess::ProcessError error);

    void infoserve_networkerror(uint error);

    void requestTimeout();

protected:


private:
    ZMQContext* context;
    Broker* broker;
    Publisher *broadcaster;
    Subscriber *reporterPLC;
    Subscriber *reporterCLS;
    Subscriber *reporterUI;
    Subscriber *reporterPrinter;

    Worker* serverInterface;
    Worker* serverInterface2;
    QString requestSender;

    QNetworkAccessManager * networkManager;

    QNetworkReply *nwReply;

    InfoServe * infoserve;
    ImageUploader *imgUploader;
    Orderer * orderer;
    SoftwareUpdater *swUpdater;
    DepositService *depositService;

    QSettings setting;
    QString sDeviceID;
    int     nDeviceType;

    QProcess *pClassifier;
    QProcess *pPlc;
    QProcess *pUI;
    QProcess *pPrinter;
    QTimer* updateTimer;
    QTimer* rebootCheckTimer;

    QTimer* requestTimer;


    int currentLauchAppType;
    int clsDebugMode;
    int imgBackupMode;

    bool bRestartRequired;

    bool bIdleState;

    bool    bNetworkState;

    QString sRecentlyDepositRequestBody;


    void executeProcess(uint nType, const QString & option);

    void requestAuthentication(const QString& cNum);



    void requestDeviceLogon();

    void upgradeProceed();




    void setRebootCheckTime();


};
#endif // MAINAPPLICATON_H
