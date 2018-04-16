#include "mainapplicaton.h"


#include <QtNetwork>
#include <QUrl>

#include "global.hpp"



#define PLCPROG     0
#define CLSPROG     1
#define UIPROG      2
#define PRTPROG     3

MainApplication::MainApplication(int& argc, char** argv)
    : super(argc, argv)
{
    bRestartRequired = false;
    bIdleState = false;

    //QSettings setting("NTN", "Nephron");

    nDeviceType = setting.value("DeviceType").toInt();  //regedit
    sDeviceID = setting.value("DeviceID").toString();
    clsDebugMode = setting.value("CLSDebugMode").toInt();
    imgBackupMode = setting.value("ImageBackupMode").toInt();

    bNetworkState =  true;

    context = createDefaultContext(this);
    context->start();
    currentLauchAppType = 0;

    requestTimer = new QTimer(this);

    connect(requestTimer, SIGNAL(timeout()), this, SLOT(requestTimeout()));

    updateTimer = new QTimer(this);
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(updateCheck()));


    updateTimer->setSingleShot(false);
    updateTimer->start(50000);//000);
    //updateTimer->start(SW_UPDATE_CHECKTIME);

    rebootCheckTimer = new QTimer(this);
    connect(rebootCheckTimer, SIGNAL(timeout()), this, SLOT(rebootCmd()));

    setRebootCheckTime();



    QTimer::singleShot(0, this, SLOT(run()));


    //rohs edit : 20171031
//    QProcess process_device_id;
//    process_device_id.execute("/home/superbin/find_device_id.sh");

}

MainApplication::~MainApplication()
{
    infoserve->sendAlert("02","00","01",NULL);
    if(pPlc)
    {
        pPlc->close();

        delete pPlc;

    }
    if(pClassifier)
    {
        pClassifier->close();

        delete pClassifier;

    }
    if(pUI)
    {
        pUI->close();

        delete pUI;

    }

    if(pPrinter)
    {
        pPrinter->close();

        delete pPrinter;

    }
}

void MainApplication::setRebootCheckTime()
{
    rebootCheckTimer->stop();

    QDateTime current = QDateTime::currentDateTime();
    QDateTime targetTime = QDateTime::currentDateTime();

    QTime targetT(4,0);
    targetTime.setTime(targetT);
    targetTime = targetTime.addDays(1);
    qint64 msecToTarget = current.msecsTo(targetTime);
    //msecToTarget = 150000;
    rebootCheckTimer->start(msecToTarget);
    qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("set Reboot Check Time(%1) ").arg(msecToTarget));

}

void MainApplication::updateCheck()
{
    qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("update check Start  (%1) ").arg(bIdleState));

    if(!bIdleState)
        return;

    if(swUpdater)
    {
        return;
    }

    swUpdater = new SoftwareUpdater(this);
    swUpdater->setDeviceID(sDeviceID);
    connect(swUpdater, SIGNAL(restartRequired()), this, SLOT(setRestartRequired()));
    connect(swUpdater, SIGNAL(checkFinish()), this, SLOT(updateCheckFinish()));


}


void MainApplication::rebootCmd()
{
    qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("Check Reboot Require Reboot(%1) : IdleState(%2) ").arg(bRestartRequired).arg(bIdleState));
    if(bRestartRequired )
    {
        if(bIdleState)
        {
            QProcess process;
            process.execute("sudo reboot");
        }
        else
        {
            rebootCheckTimer->stop();
            rebootCheckTimer->start(120000);
        }
    }
    else
    {
        setRebootCheckTime();
    }
}

void MainApplication::setRestartRequired()
{
    bRestartRequired = true;
}

void MainApplication::updateCheckFinish()
{
    if(swUpdater)
    {
        swUpdater->deleteLater();
        swUpdater = NULL;
    }
    qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("Software updateCheckFinish"));

}

bool MainApplication::notify(QObject *obj, QEvent *event)
{
    try
    {
        return super::notify(obj, event);
    }
    catch (std::exception& ex)
    {
        qWarning() << ex.what();
        return false;
    }
}

void MainApplication::run()
{

    try
    {

        broker = new Broker(*context, 0, this);

        connect(broker, SIGNAL(finished()), SLOT(quit()));
        broker->start();

        broadcaster = new Publisher(*context,BROADCAST_IPC,"MAIN" );
        broadcaster->start();

        reporterPLC = new Subscriber(*context, REPORT_IPC_PLC,"MAIN");
        reporterCLS = new Subscriber(*context, REPORT_IPC_CLS,"MAIN");
        reporterUI = new Subscriber(*context, REPORT_IPC_UI,"MAIN");
        reporterPrinter = new Subscriber(*context, REPORT_IPC_PRINTER,"MAIN");

        reporterPLC->setSubscribe("");
        reporterCLS->setSubscribe("");
        reporterUI->setSubscribe("");
        reporterPrinter->setSubscribe("");

        connect(reporterPLC,SIGNAL(receivedMessage(const QList<QByteArray>&)), SLOT(receivedMessage(const QList<QByteArray>&)) );
        connect(reporterCLS,SIGNAL(receivedMessage(const QList<QByteArray>&)), SLOT(receivedMessage(const QList<QByteArray>&)) );
        connect(reporterUI,SIGNAL(receivedMessage(const QList<QByteArray>&)), SLOT(receivedMessage(const QList<QByteArray>&)) );
        connect(reporterPrinter,SIGNAL(receivedMessage(const QList<QByteArray>&)), SLOT(receivedMessage(const QList<QByteArray>&)) );

        reporterPLC->start();
        reporterCLS->start();
        reporterUI->start();
        reporterPrinter->start();

        serverInterface = new Worker(*context,"LoginService", "Main", 0 , this );
        connect(serverInterface, SIGNAL(finished()), SLOT(quit()));
        connect(serverInterface, SIGNAL(requestReceived(const QString& , const QString& )), SLOT(requestReceived(const QString& , const QString&)));

        serverInterface->start();

        serverInterface2 = new Worker(*context,"DepositService", "Main2", 0 , this );
        connect(serverInterface2, SIGNAL(finished()), SLOT(quit()));
        connect(serverInterface2, SIGNAL(requestReceived(const QString& , const QString& )), SLOT(requestDeposit(const QString& , const QString&)));

        serverInterface2->start();

        infoserve = new InfoServe(this);
        connect(infoserve, SIGNAL(NetworkError(uint)), SLOT(infoserve_networkerror(uint)));

        infoserve->setDeviceID(sDeviceID);
        infoserve->setHeartBeatInterval(3000);

        infoserve->sendAlert("02","00","00",NULL);
        infoserve->start();

        imgUploader = new ImageUploader(imgBackupMode,this);
        imgUploader->setDeviceID(sDeviceID);

        orderer = new Orderer(this);
        orderer->setDeviceID(sDeviceID);

        depositService = new DepositService(this);
        depositService->setDeviceID(sDeviceID);
        depositService->setNetworkState(bNetworkState);


        networkManager = new QNetworkAccessManager(this);        




        QTimer::singleShot(1000, this, SLOT(launchApps()));

    }
    catch (std::exception& ex)
    {
        qWarning() << ex.what();
        exit(-1);
    }
}

void MainApplication::launchApps()
{
    executeProcess(currentLauchAppType, "");


    if(currentLauchAppType < (nDeviceType)?PRTPROG:UIPROG)
    {
        currentLauchAppType++;
        QTimer::singleShot(1000, this, SLOT(launchApps()));
    }

}

void MainApplication::executeProcess(uint nType, const QString & option)
{

    QStringList slarg;
    if(option.length() > 0)
    {
        slarg << option;
    }

    switch(nType)
    {
    case PLCPROG:
        {
            if(pPlc)
            {
                pPlc->close();

                delete pPlc;

            }

            //dev mode
            pPlc = new QProcess;
            connect(pPlc, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(processPlcFinishied(int, QProcess::ExitStatus)));
            connect(pPlc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processPlcError(QProcess::ProcessError)));
            pPlc->start("/home/superbin/Nephron/bin/plc", slarg);

        }

        break;
    case CLSPROG:
    {
        if(pClassifier)
        {
            pClassifier->close();

            delete pClassifier;

        }

        if (clsDebugMode>0)
        {
            switch(clsDebugMode)
            {
                case 1:
                slarg << "CAN";
                break;
            case 2:
            slarg << "PET";
            break;
            case 3:
            slarg << "RECYCLE";
            break;
            case 4:
            slarg << "REUSE40";
            break;
            case 5:
            slarg << "REUSE50";
            break;
            case 6:
            slarg << "ETC";
            break;

            }
        }

        qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("CLS RUN Mode(%1)").arg(clsDebugMode));

        pClassifier = new QProcess;
        connect(pClassifier, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(processClassifierFinishied(int, QProcess::ExitStatus)));
        connect(pClassifier, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processClassifierError(QProcess::ProcessError)));
        pClassifier->start("/home/superbin/Nephron/bin/classifier", slarg);

    }

    break;
    case UIPROG:
    {
        if(pUI)
        {
            pUI->close();

            delete pUI;

        }

        pUI = new QProcess;
        connect(pUI, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(processUIFinishied(int, QProcess::ExitStatus)));
        connect(pUI, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processUIError(QProcess::ProcessError)));
        pUI->start("/home/superbin/Nephron/bin/ui", slarg);

    }

    break;
    case PRTPROG:
    {
        if(pPrinter)
        {
            pPrinter->close();

            delete pPrinter;

        }

        pPrinter = new QProcess;
        connect(pPrinter, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(processPrinterFinishied(int, QProcess::ExitStatus)));
        connect(pPrinter, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processPrinterError(QProcess::ProcessError)));
        pPrinter->start("/home/superbin/Nephron/bin/printer", slarg);

    }
    break;

    }

}


void MainApplication::processPlcFinishied(int exitCode, QProcess::ExitStatus exitStatus)
{
    if(exitStatus == QProcess::CrashExit)
    {
        qLog(LOGLEVEL_VERY_HIGH, "Main", "Error", QString("PLC Exit abnormal case"));
    }
    else
    {
        qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("PLC Exit normal case"));
    }

    exit(1001);
}

void MainApplication::processClassifierFinishied(int exitCode, QProcess::ExitStatus exitStatus)
{
    if(exitStatus == QProcess::CrashExit)
    {
        qLog(LOGLEVEL_VERY_HIGH, "Main", "Error", QString("Classifier Exit abnormal case"));
    }
    else
    {
        qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("Classifier Exit normal case"));
    }

     exit(1002);

}

void MainApplication::processUIFinishied(int exitCode, QProcess::ExitStatus exitStatus)
{
    if(exitStatus == QProcess::CrashExit)
    {
        qLog(LOGLEVEL_VERY_HIGH, "Main", "Error", QString("UI Exit abnormal case"));
    }
    else
    {
        qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("UI Exit normal case"));
    }

    exit(1003);

}

void MainApplication::processPrinterFinishied(int exitCode, QProcess::ExitStatus exitStatus)
{
    if(exitStatus == QProcess::CrashExit)
    {
        qLog(LOGLEVEL_VERY_HIGH, "Main", "Error", QString("Printer Exit abnormal case"));
    }
    else
    {
        qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("Printer Exit normal case"));
    }

    exit(1004);

}


void MainApplication::processPlcError(QProcess::ProcessError error)
{

}
void MainApplication::processClassifierError(QProcess::ProcessError error)
{


}
void MainApplication::processUIError(QProcess::ProcessError error)
{


}

void MainApplication::processPrinterError(QProcess::ProcessError error)
{


}


void MainApplication::infoserve_networkerror(uint error)
{

    qLog(LOGLEVEL_VERY_HIGH, "Main", "Warning", QString("infoserve_networkerror:  Error code(%1) current state(%2)").arg(error).arg(bNetworkState?"Connected":"DisConnected"));


    if(error == 0)
    {
        if(!bNetworkState)
        {
            bNetworkState = true;
            depositService->setNetworkState(bNetworkState);

        }

    }
    else if(error >0 && error < 200)
    {
        //network Error
        if(bNetworkState)
        {
            bNetworkState = false;
            //send network fail
            depositService->setNetworkState(bNetworkState);

        }

    }
    QByteArray cmd;
    QByteArray data;
    cmd.clear();
    data.clear();
    cmd.append(QChar(PUBLISH_COMMAND));

    if(bNetworkState)
    {
        data.append(QChar(NS_CONNECTED));
    }
    else
    {
        data.append(QChar(NS_DISCONNECTED));
    }

    broadcaster->sendMessage(cmd, data);
}

void MainApplication::receivedMessage(const QList<QByteArray>& message)
{
    /*
    //qDebug() << "SUB >> ";
    for(int i = 0 ; i < message.count(); i++)
    {
        //qDebug() << i << ": " << message[i];
    }
*/
    if(message.count() < 2)
    {
        qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("Subscribe Message :  Parameter Error(length(%1))").arg(message.count()));
        return;
    }

    qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("Subscribe Message(%1) : %2 ").arg(int(message[0].at(0))).arg(int(message[1].at(0))));

    switch(message[0].at(0))
    {
        case REPORT_PLCSTATE:
        case REPORT_SENSORSTATE:
        {
            QByteArray cmd;
            cmd.clear();
            if(message[0].at(0) == REPORT_PLCSTATE)
            {
                cmd.append(QChar(PUBLISH_PLCSTATE));
                if(message[1].at(0) == PS_DOOROPEN)
                {
                    //qDebug() << "PLC SPEACK : PS_DOOROPEN" << message[1].at(0);
                }
            }
            else
            {
                cmd.append(QChar(PUBLISH_SENSORSTATE));
            }

            QByteArray data;
            data.clear();
            for(int i = 0; i < message[1].size(); i++)
            {
                data.append(message[1].at(i));
            }


            broadcaster->sendMessage(cmd, data);
        }
        break;
//        case REPORT_PRINTSTATE:
//        {
//            QByteArray cmd;
//            cmd.clear();
//            cmd.append(QChar(PUBLISH_PRINTSTATE));
//            QByteArray data;
//            data.clear();
//            data.append(message[1].at(0));

//            broadcaster->sendMessage(cmd, data);

//            switch(message[1].at(0))
//            {
//            case PRINTER_STATE_OK:
//                infoserve->sendAlert("03","00","00",NULL);
//                break;
//            case PRINTER_STATE_EMPTY_PAPER:
//                infoserve->sendAlert("03","00","01",NULL);
//                break;
//            case PRINTER_STATE_PAPER_JAM:
//                infoserve->sendAlert("03","00","02",NULL);
//                break;
//            case PRINTER_STATE_DISCONNECT:
//                infoserve->sendAlert("03","00","03",NULL);
//                break;
//            }
//        }
        break;
        case REPORT_ERROR:
        {
            QByteArray cmd;
            QByteArray data;
            cmd.clear();
            data.clear();
            cmd.append(QChar(PUBLISH_PLCSTATE));
            data.append(message[1].at(0));
            switch(message[1].at(0))
            {
            case PS_DOOROPEN:
            {
                if(message[1].length()>=2)
                {                    
                    data.append(message[1].at(1));
                    int nRet = data.at(1);
                    QBitArray resultBits(2);
                    resultBits.setBit(0, (nRet & (1<<0)));
                    resultBits.setBit(1, (nRet & (1<<1)));

//                    if(resultBits.at(0)){
//                        infoserve->sendAlert("01","00","01",NULL);

//                    }else{

//                        infoserve->sendAlert("01","00","00",NULL);

//                    }

//                    if(resultBits.at(1)){
//                        infoserve->sendAlert("01","01","01",NULL);

//                    }else{

//                        infoserve->sendAlert("01","01","00",NULL);

//                    }

                    if(resultBits.at(0)){
                        infoserve->sendAlert("01","00","01",NULL);

                    }else{

                        //infoserve->sendAlert("01","00","00",NULL);

                    }

                    if(resultBits.at(1)){
                        infoserve->sendAlert("01","01","01",NULL);

                    }else{

                        //infoserve->sendAlert("01","01","00",NULL);

                    }


                }
            }
            break;

            case PS_JAM:
            {

                if(message[1].length()>=2)
                {
                    data.append(message[1].at(1));
                    if(data.at(1)==0){
                        infoserve->sendAlert("01","02","00",NULL);
                    }
                    else
                    {
                        infoserve->sendAlert("01","02","01",NULL);
                    }

                }

            }
            break;
//            case PS_STORAGE_EMPTY:
//            {

//                if(message[1].length()>=2)
//                {

//                    data.append(message[1].at(1));
//                    int nRet = data.at(1);
//                    if(nDeviceType)
//                    {
//                        infoserve->sendAlert("00","02",(nRet)?"03":"00",NULL);

//                    }
//                    else
//                    {
//                        QBitArray resultBits(2);
//                        resultBits.setBit(0, (nRet & (1<<0)));
//                        resultBits.setBit(1, (nRet & (1<<1)));
//                        if(resultBits.at(0)){
//                            infoserve->sendAlert("00","01","03",NULL);

//                        }else{

//                            infoserve->sendAlert("00","01","00",NULL);

//                        }
//                        if(resultBits.at(1)){
//                            infoserve->sendAlert("00","00","03",NULL);

//                        }else{

//                            infoserve->sendAlert("00","00","00",NULL);

//                        }
//                    }



//                }


//            }
//            break;
            case PS_STORAGE_FULL:
            {

                if(message[1].length()>=2)
                {
                    int nRet = message[1].at(1);

                    if(nDeviceType)
                    {
                        data.append((nRet)?1:0);
                        infoserve->sendAlert("00","02","02",NULL);
                    }
                    else
                    {

                        QBitArray resultBits(4);
                        bool bFull = false;
                        resultBits.setBit(0, (nRet & (1<<0)));
                        resultBits.setBit(1, (nRet & (1<<1)));
                        resultBits.setBit(2, (nRet & (1<<2)));
                        resultBits.setBit(3, (nRet & (1<<3)));
                        //qDebug() << resultBits.at(3) << resultBits.at(2)<<resultBits.at(1)<<resultBits.at(0)<< endl;

                        if(resultBits.at(0) == true && resultBits.at(1) == true){
                            infoserve->sendAlert("00","01","02",NULL);
                            bFull = true;
                        }else if(resultBits.at(0) == true && resultBits.at(1) == false){
                            infoserve->sendAlert("00","01","02",NULL);
                        }else if(resultBits.at(0) == false && resultBits.at(1) == true){
                            infoserve->sendAlert("00","01","01",NULL);
                        }if(resultBits.at(0) == false && resultBits.at(1) == false){
                            infoserve->sendAlert("00","01","00",NULL);
                        }

                        if(resultBits.at(2) == true && resultBits.at(3) == true){
                            infoserve->sendAlert("00","00","02",NULL);
                            bFull = true;
                        }else if(resultBits.at(2) == true && resultBits.at(3) == false){
                            infoserve->sendAlert("00","00","02",NULL);
                        }else if(resultBits.at(2) == false && resultBits.at(3) == true){
                            infoserve->sendAlert("00","00","01",NULL);
                        }if(resultBits.at(2) == false && resultBits.at(3) == false){
                            infoserve->sendAlert("00","00","00",NULL);
                        }

                        data.append((bFull)?1:0);
                    }


                }


            }
            break;            
            }


            broadcaster->sendMessage(cmd, data);

        }
        break;
        case REPORT_LOG:
        {

        }
        break;
        case REPORT_INFO:
        {
            if(message[1].at(0)==1)
            {
                if(!bIdleState)
                {
                    qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("Nephron system idle mode"));

                }
                bIdleState = true;

            }
            else
            {
                if(bIdleState)
                {
                    qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("Nephron system busy mode"));

                }
                bIdleState = false;

            }

        }
        break;

        case REPORT_CLS:
        {
            if(imgUploader)
            {
                if(message.count() >= 3)
                {
                    QString category = QString(message[1]);

                    QString sPath = QString(message[2]);
                    QString sDate = QString(message[3]);

                    imgUploader->uploadImage(category,sPath,sDate);

                }
                else
                {
                    QByteArray cmd;
                    cmd.clear();
                    cmd.append(QChar(PUBLISH_BARCODE));
                    QByteArray data;
                    data.append(message[1]);
                    broadcaster->sendMessage(cmd, data);
                }

            }
        }
        break;
        case REPORT_REMOVEBOX:
        {
            //infoserve->sendAlert("00","01","02",NULL);
            orderer->sendOrder("test","test");
        }
        break;


    }

}


void MainApplication::requestReceived(const QString& sender, const QString& message)
{
    QString result = "OK";
    //qDebug() << "request Arrived : From " << sender;
    //qDebug() << "request Arrived : Message >> " << message;

    requestSender = sender;
    /*
    QJsonDocument jsonParam(QJsonDocument::fromJson(message.toLocal8Bit()));

    QJsonObject jobj = jsonParam.object();
    if(!jobj.empty())
    {



    }
    */
    if(bNetworkState)
    {
        requestAuthentication(message);
    }
    else
    {
        serverInterface->replyRequest(requestSender,"FAILED");
    }



    /*
    if(message.compare("Start")==0)
    {
        //qDebug() << "Start OK!!! ";
        result = "Start OK!!";
        PLCInterface->writeRequest(4, IO_START_PROC, 1);
        //client->sendRequest("Classifier", "InitCamera");
    }
    else if(message.compare("Stop")==0)
    {
        //gotoReadyMode();
        PLCInterface->writeRequest(4, IO_START_PROC, 0);
    client->sendRequest("Classifier", "CloseCamera");
    }

    worker->replyRequest(sender, result);
*/


}



void MainApplication::requestAuthentication(const QString& cNum)
{
    disconnect(networkManager,SIGNAL(finished(QNetworkReply*)),0,0);
    connect(networkManager, SIGNAL(finished(QNetworkReply*)),
                     this, SLOT(loginResponse(QNetworkReply*)));

    QUrl url("http://nephron.superbin.co.kr/api/auth/login.php?cellPhone="+cNum);
    nwReply = networkManager->get(QNetworkRequest(url));
    requestTimer->start(REQ_TIMEOUT);
}

void MainApplication::requestDeposit(const QString& sender, const QString& requestBody)
{
    requestSender = sender;

    if(!bNetworkState)
    {
        depositService->addDeposit(requestBody);
        serverInterface2->replyRequest(requestSender,"FAILED");
        return;
    }

    QUrl url("http://nephron.superbin.co.kr/api/waste/reserve.php");
    QNetworkRequest req(url);
    //Creating the JSON-Data
    sRecentlyDepositRequestBody = requestBody;

    disconnect(networkManager,SIGNAL(finished(QNetworkReply*)),0,0);
    connect(networkManager, SIGNAL(finished(QNetworkReply*)),
                     this, SLOT(depositResponse(QNetworkReply*)));


    QByteArray jsonPost = requestBody.toLocal8Bit();

    req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json; charset=utf-8"));
    req.setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(jsonPost.size()));

    //Sending the Request
    nwReply = networkManager->post(req,jsonPost);

    ////qDebug() << "Deposit request : " << jsonPost;
    requestTimer->start(REQ_TIMEOUT);
    qLog(LOGLEVEL_VERY_HIGH, "Main", "Info", QString("Deposit Request :%1").arg( requestBody));


}

void MainApplication::loginResponse(QNetworkReply* reply)
{

    //qDebug() << "loginResponse ";
    requestTimer->stop();

    if(reply->error() == QNetworkReply::NoError) {

        QString strReply = (QString)reply->readAll();

        //qDebug() << strReply;

        QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());

        QJsonObject jsonObject = jsonResponse.object();


        serverInterface->replyRequest(requestSender,strReply);

    } else {

        serverInterface->replyRequest(requestSender,"FAILED");
    }

    //delete reply;
    reply->deleteLater();
    nwReply = NULL;

}

void MainApplication::depositResponse(QNetworkReply* reply)
{
    requestTimer->stop();
    if(reply->error() == QNetworkReply::NoError) {

        QString strReply = (QString)reply->readAll();

        //qDebug() << strReply;

        QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());

        QJsonObject jsonObject = jsonResponse.object();


        serverInterface2->replyRequest(requestSender,strReply);

    } else {
        depositService->addDeposit(sRecentlyDepositRequestBody);
        serverInterface2->replyRequest(requestSender,"FAILED");
    }

    //delete reply;
    reply->deleteLater();
    nwReply = NULL;
}

void MainApplication::requestTimeout()
{
    qLog(LOGLEVEL_VERY_HIGH, "Main", "Error", QString("Heartbeat Reply request timeout"));

    if(nwReply)
    {
        nwReply->abort();
    }

}


void MainApplication::sendRequest()
{

    requestAuthentication("01011111111");
}

