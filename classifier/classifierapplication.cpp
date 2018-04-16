#include "classifierapplication.h"
#include <QCameraInfo>
#include <QDateTime>
#include <QTimer>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QSettings>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>


#include "nzmqt.hpp"

//#define BARCODEUSE


#ifdef ANNOTATION

#define CNN_MODEL   "../data/cls_deploy.prototxt"
#define CNN_TRAINED_0 "../data/cls_network0.caffemodel"
#define CNN_TRAINED_1 "../data/cls_network1.caffemodel"
#define CNN_TRAINED_2 "../data/cls_network2.caffemodel"

#define CNN_ATT_MODEL   "../data/att_deploy.prototxt"
#define CNN_ATT_TRAINED_0 "../data/att_network0.caffemodel"
#define CNN_ATT_TRAINED_1 "../data/att_network1.caffemodel"
#define CNN_ATT_TRAINED_2 "../data/att_network2.caffemodel"

#define CNN_MEAN    "../data/cls_image_mean.binaryproto"

#else

#define CNN_MODEL   "../data/deploy.prototxt"
#define CNN_TRAINED_0 "../data/network0.caffemodel"
#define CNN_TRAINED_1 "../data/network1.caffemodel"
#define CNN_TRAINED_2 "../data/network2.caffemodel"
#define CNN_MEAN    "../data/image_mean.binaryproto"

#endif





ClassifierApplication::ClassifierApplication(int& argc, char** argv)
    : super(argc, argv),
      camera(0),
      imageCapture(0),
      bReadyCamera(false),
      bCameraOn(false),
      bCameraInitializing(false),
      #ifdef ANNOTATION
      detector0(CNN_ATT_MODEL, CNN_ATT_TRAINED_0, true),
      detector1(CNN_ATT_MODEL, CNN_ATT_TRAINED_0, true),
      detector2(CNN_ATT_MODEL, CNN_ATT_TRAINED_0, true),
      #endif
      classifier0(CNN_MODEL, CNN_TRAINED_0, CNN_MEAN, true),
      classifier1(CNN_MODEL, CNN_TRAINED_1, CNN_MEAN, true),
      classifier2(CNN_MODEL, CNN_TRAINED_2, CNN_MEAN, true)

{


    QSettings setting("NTN", "Nephron");
    deviceType = setting.value("DeviceType").toInt();
    nImageUploadMode = setting.value("ImageUploadMode").toInt();
    nTestMode = setting.value("CLSDebugMode").toInt();

    bCamTest = (nTestMode > 0);


    /*
    if(argc >= 2)
    {
        QString strMode;
        bCamTest = true;
        strMode = argv[1];
        if(strMode.compare("CAN")== 0)
        {
            nTestMode = 0;
        }
        else if(strMode.compare("PET")== 0)
        {
            nTestMode = 1;

        }
        else if(strMode.compare("RECYCLE")== 0)
        {

            nTestMode = 2;
        }
        else if(strMode.compare("REUSE40")== 0)
        {
            nTestMode = 3;

        }
        else if(strMode.compare("REUSE50")== 0)
        {
            nTestMode = 4;

        }
        else if(strMode.compare("ETC")== 0)
        {
            nTestMode = 5;

        }
        qDebug() << "Classifier in Test Mode : " << strMode << "(" << nTestMode << ")";


    }
    else
    {
        bCamTest = false;
        nTestMode = 0;
    }
    */

    context = createDefaultContext(this);
    context->start();
#ifdef BARCODEUSE
    if(deviceType)
    {
        loadBarcodeValue();

        reader = new BarcodeReader();

        connect(reader, SIGNAL(reportError(uint)), SLOT(reportError(uint)));
        connect(reader,SIGNAL(coderead(const QString&)), this, SLOT(barcodeArrived(const QString&)));
    }
#endif

#ifdef ANNOTATION
    ng0 = new Neurogenie(detector0, classifier0);
    ng1 = new Neurogenie(detector1, classifier1);
    ng2 = new Neurogenie(detector2, classifier2);
#endif

    initCamera();
    setCamera();

    QTimer::singleShot(0, this, SLOT(run()));

}
void ClassifierApplication::addBarcode(int code, const QString & barcodeVal)
{

    barcodeInfo *bar = new barcodeInfo(code, barcodeVal);
    if(bar)
    {
        lBarcode.push_back(bar);
    }

}


void ClassifierApplication::loadBarcodeValue()
{
    addBarcode(CLS_REUSE40,"8801030999126" );
    addBarcode(CLS_REUSE40,"8801030998198" );
    addBarcode(CLS_REUSE50,"8801030999096" );
    addBarcode(CLS_REUSE50,"8801030996811" );
    addBarcode(CLS_REUSE40,"8801152233825" );
    addBarcode(CLS_REUSE40,"8801152134443" );
    addBarcode(CLS_REUSE40,"8801152233801");
    addBarcode(CLS_REUSE40,"8801152122792");
    addBarcode(CLS_REUSE40,"8801152234464");
    addBarcode(CLS_REUSE40,"8801152234501");
    addBarcode(CLS_REUSE40,"8801152135044");
    addBarcode(CLS_REUSE40,"8801152134795");
    addBarcode(CLS_REUSE40,"8801152134764");
    addBarcode(CLS_REUSE40,"8801030370017");
    addBarcode(CLS_REUSE40,"8801030999539");
    addBarcode(CLS_REUSE50,"8801030031321");
    addBarcode(CLS_REUSE100,"8801030031123");
    addBarcode(CLS_REUSE40,"8801030810070");
    addBarcode(CLS_REUSE40,"8801030999270");
    addBarcode(CLS_REUSE40,"8801030996699");
    addBarcode(CLS_REUSE40,"8801030810223");
    addBarcode(CLS_REUSE40,"8801030370048");
    addBarcode(CLS_REUSE40,"8801030810049");
    addBarcode(CLS_REUSE40,"8801030997870");
    addBarcode(CLS_REUSE40,"8801030997658");
    addBarcode(CLS_REUSE40,"8801030997474");
    addBarcode(CLS_REUSE40,"8801030996484");
    addBarcode(CLS_REUSE40,"8801080133105");
    addBarcode(CLS_REUSE40,"8801080124202");
    addBarcode(CLS_REUSE40,"8801080123144");
    addBarcode(CLS_REUSE40,"8801080323117");
    addBarcode(CLS_REUSE40,"8801080313101");
    addBarcode(CLS_REUSE40,"8801080313118");
    addBarcode(CLS_REUSE40,"8801080313125");
    addBarcode(CLS_REUSE40,"8801080313132");
    addBarcode(CLS_REUSE40,"8801080313170");
    addBarcode(CLS_REUSE40,"8801080124103");
    addBarcode(CLS_REUSE40,"8801080120181");
    addBarcode(CLS_REUSE40,"8801080224100");
    addBarcode(CLS_REUSE40,"8801137110028");
    addBarcode(CLS_REUSE40,"8801137110011");
    addBarcode(CLS_REUSE40,"8801137110035");
    addBarcode(CLS_REUSE40,"8801137140018");
    addBarcode(CLS_REUSE40,"8801137140025");
    addBarcode(CLS_REUSE40,"8801137140032");
    addBarcode(CLS_REUSE40,"8801137140155");
    addBarcode(CLS_REUSE40,"8801137150017");
    addBarcode(CLS_REUSE40,"8801137150024");
    addBarcode(CLS_REUSE40,"8801137150031");
    addBarcode(CLS_REUSE40,"8801137180014");
    addBarcode(CLS_REUSE40,"8801137180021");
    addBarcode(CLS_REUSE40,"8801137180038");
    addBarcode(CLS_REUSE40,"8801137160016");
    addBarcode(CLS_REUSE40,"8801137160023");
    addBarcode(CLS_REUSE40,"8801137190013");
    addBarcode(CLS_REUSE40,"8801137190020");
    addBarcode(CLS_REUSE40,"8801137100210");
    addBarcode(CLS_REUSE40,"8801137100227");
    addBarcode(CLS_REUSE40,"8801137100319");
    addBarcode(CLS_REUSE40,"8801137141015");
    addBarcode(CLS_REUSE40,"8801137141022");
    addBarcode(CLS_REUSE40,"8801137100012");
    addBarcode(CLS_REUSE40,"8801137100029");
    addBarcode(CLS_REUSE40,"8801137100036");
    addBarcode(CLS_REUSE40,"8801460105012");
    addBarcode(CLS_REUSE40,"8801100214999");
    addBarcode(CLS_REUSE40,"8801100128951");
    addBarcode(CLS_REUSE40,"8801100210274");
    addBarcode(CLS_REUSE40,"8801100129729");
    addBarcode(CLS_REUSE40,"8801100119133");
    addBarcode(CLS_REUSE40,"8801100128821");
    addBarcode(CLS_REUSE40,"8801100128852");
    addBarcode(CLS_REUSE40,"8801100128845");
    addBarcode(CLS_REUSE40,"8801100128906");
    addBarcode(CLS_REUSE40,"8801100128937");
    addBarcode(CLS_REUSE40,"8801100129309");
    addBarcode(CLS_REUSE40,"8801100119119");
    addBarcode(CLS_REUSE40,"8801100129743");
    addBarcode(CLS_REUSE40,"8801147100149");
    addBarcode(CLS_REUSE40,"8801147120857");
    addBarcode(CLS_REUSE40,"8801147121106");
    addBarcode(CLS_REUSE40,"8801147121656");
    addBarcode(CLS_REUSE40,"8801147120178");
    addBarcode(CLS_REUSE40,"8801147120383");
    addBarcode(CLS_REUSE50,"88005317");
    addBarcode(CLS_REUSE50,"88005324");
    addBarcode(CLS_REUSE50,"88005331");
    addBarcode(CLS_REUSE40,"88005362");
    addBarcode(CLS_REUSE40,"8801858042783");
    addBarcode(CLS_REUSE40,"8801858041632");
    addBarcode(CLS_REUSE50,"88008806");
    addBarcode(CLS_REUSE50,"88008813");
    addBarcode(CLS_REUSE40,"8801021104669");
    addBarcode(CLS_REUSE40,"88004259");
    addBarcode(CLS_REUSE40,"88004266");
    addBarcode(CLS_REUSE40,"8809414660020");
    addBarcode(CLS_REUSE40,"8809414660037");
    addBarcode(CLS_REUSE40,"8801798031021");
    addBarcode(CLS_REUSE40,"8801798052019");
    addBarcode(CLS_REUSE50,"8801798041112");
    addBarcode(CLS_REUSE20,"8801798031090");
    addBarcode(CLS_REUSE20,"8801798051098");
    addBarcode(CLS_REUSE40,"8801119833358");
    addBarcode(CLS_REUSE40,"8801119833303");
    addBarcode(CLS_REUSE50,"8801119832207");
    addBarcode(CLS_REUSE50,"8801119831101");
    addBarcode(CLS_REUSE50,"8801119262202");
    addBarcode(CLS_REUSE50,"8801119261106");
    addBarcode(CLS_REUSE40,"8801119763303");
    addBarcode(CLS_REUSE50,"8801119762108");
    addBarcode(CLS_REUSE40,"88005508");
    addBarcode(CLS_REUSE50,"8801119852205");
    addBarcode(CLS_REUSE50,"8801119220011");
    addBarcode(CLS_REUSE40,"8801048951000");
    addBarcode(CLS_REUSE40,"8801048951109");
    addBarcode(CLS_REUSE40,"8801048911004");
    addBarcode(CLS_REUSE40,"8801048101009");
    addBarcode(CLS_REUSE40,"8802036120033");
    addBarcode(CLS_REUSE40,"8802036120026");
    addBarcode(CLS_REUSE40,"8802036111130");
    addBarcode(CLS_REUSE40,"8802036222027");
    addBarcode(CLS_REUSE40,"8802036222034");
    addBarcode(CLS_REUSE40,"8802036222058");
    addBarcode(CLS_REUSE40,"8802036000106");
    addBarcode(CLS_REUSE40,"8802036000045");
    addBarcode(CLS_REUSE40,"8802036000069");
    addBarcode(CLS_REUSE40_N,"8801030995579");
    addBarcode(CLS_REUSE40_N,"8801030995548");
    addBarcode(CLS_REUSE50_N,"8801030995517");
    addBarcode(CLS_REUSE50_N,"8801030995494");
    addBarcode(CLS_REUSE40_N,"8801152135174");
    addBarcode(CLS_REUSE40_N,"8801152135198");
    addBarcode(CLS_REUSE40_N,"8801152135235");
    addBarcode(CLS_REUSE40_N,"8801152135266");
    addBarcode(CLS_REUSE40_N,"8801152135297");
    addBarcode(CLS_REUSE40_N,"8801152135327");
    addBarcode(CLS_REUSE40_N,"8801152135341");
    addBarcode(CLS_REUSE40_N,"8801152135365");
    addBarcode(CLS_REUSE40_N,"8801152135389");
    addBarcode(CLS_REUSE40_N,"8801152135402");
    addBarcode(CLS_REUSE40_N,"8801152135419");
    addBarcode(CLS_REUSE40_N,"8801152135426");
    addBarcode(CLS_REUSE40_N,"8801030995470");
    addBarcode(CLS_REUSE40_N,"8801030995449");
    addBarcode(CLS_REUSE50_N,"8801030995425");
    addBarcode(CLS_REUSE100_N,"8801030995401");
    addBarcode(CLS_REUSE40_N,"8801030995388");
    addBarcode(CLS_REUSE40_N,"8801030955139");
    addBarcode(CLS_REUSE40_N,"8801030995364");
    addBarcode(CLS_REUSE40_N,"8801030995340");
    addBarcode(CLS_REUSE40_N,"8801030995326");
    addBarcode(CLS_REUSE40_N,"8801030995296");
    addBarcode(CLS_REUSE40_N,"8801030995272");
    addBarcode(CLS_REUSE40_N,"8801030995241");
    addBarcode(CLS_REUSE40_N,"8801030995227");
    addBarcode(CLS_REUSE40_N,"8801030995203");
    addBarcode(CLS_REUSE40_N,"8801080123182");
    addBarcode(CLS_REUSE40_N,"8801080124219");
    addBarcode(CLS_REUSE40_N,"8801080123175");
    addBarcode(CLS_REUSE40_N,"8801080313149");
    addBarcode(CLS_REUSE40_N,"8801080313187");
    addBarcode(CLS_REUSE40_N,"8801080313194");
    addBarcode(CLS_REUSE40_N,"8801080303119");
    addBarcode(CLS_REUSE40_N,"8801080323100");
    addBarcode(CLS_REUSE40_N,"8801080303102");
    addBarcode(CLS_REUSE40_N,"8801080153110");
    addBarcode(CLS_REUSE40_N,"8801080120204");
    addBarcode(CLS_REUSE40_N,"8801080224117");
    addBarcode(CLS_REUSE40_N,"8801080123205");
    addBarcode(CLS_REUSE40_N,"8801137510019");
    addBarcode(CLS_REUSE40_N,"8801137510026");
    addBarcode(CLS_REUSE40_N,"8801137520018");
    addBarcode(CLS_REUSE40_N,"8801137520025");
    addBarcode(CLS_REUSE40_N,"8801137520049");
    addBarcode(CLS_REUSE40_N,"8801137570013");
    addBarcode(CLS_REUSE40_N,"8801137570020");
    addBarcode(CLS_REUSE40_N,"8801137530017");
    addBarcode(CLS_REUSE40_N,"8801137530024");
    addBarcode(CLS_REUSE40_N,"8801137560014");
    addBarcode(CLS_REUSE40_N,"8801137560021");
    addBarcode(CLS_REUSE40_N,"8801137550015");
    addBarcode(CLS_REUSE40_N,"8801137550022");
    addBarcode(CLS_REUSE40_N,"8801137540016");
    addBarcode(CLS_REUSE40_N,"8801137540023");
    addBarcode(CLS_REUSE40_N,"8801137580012");
    addBarcode(CLS_REUSE40_N,"8801137580029");
    addBarcode(CLS_REUSE40_N,"8801460105227");
    addBarcode(CLS_REUSE40_N,"8801100129491");
    addBarcode(CLS_REUSE40_N,"8801100129484");
    addBarcode(CLS_REUSE40_N,"8801100129477");
    addBarcode(CLS_REUSE40_N,"8801100129378");
    addBarcode(CLS_REUSE40_N,"8801100129781");
    addBarcode(CLS_REUSE40_N,"8801100129798");
    addBarcode(CLS_REUSE40_N,"8801100129439");
    addBarcode(CLS_REUSE40_N,"8801100129620");
    addBarcode(CLS_REUSE40_N,"8801100129422");
    addBarcode(CLS_REUSE40_N,"8801100129446");
    addBarcode(CLS_REUSE40_N,"8801100129453");
    addBarcode(CLS_REUSE40_N,"8801100129460");
    addBarcode(CLS_REUSE40_N,"8801100129699");
    addBarcode(CLS_REUSE40_N,"8801100129774");
    addBarcode(CLS_REUSE40_N,"8801100129804");
    addBarcode(CLS_REUSE40_N,"8801147121731");
    addBarcode(CLS_REUSE40_N,"8801147121779");
    addBarcode(CLS_REUSE40_N,"8801147121809");
    addBarcode(CLS_REUSE40_N,"8801147121823");
    addBarcode(CLS_REUSE40_N,"8801147121847");
    addBarcode(CLS_REUSE40_N,"8801147121861");
    addBarcode(CLS_REUSE50_N,"8801858044688");
    addBarcode(CLS_REUSE50_N,"8801858044701");
    addBarcode(CLS_REUSE50_N,"8801858044725");
    addBarcode(CLS_REUSE40_N,"8801858044749");
    addBarcode(CLS_REUSE40_N,"8801858044756");
    addBarcode(CLS_REUSE40_N,"8801858044787");
    addBarcode(CLS_REUSE50_N,"8801021105017");
    addBarcode(CLS_REUSE50_N,"8801021105048");
    addBarcode(CLS_REUSE40_N,"8801021105062");
    addBarcode(CLS_REUSE40_N,"8801021083520");
    addBarcode(CLS_REUSE40_N,"8801021216485");
    addBarcode(CLS_REUSE40_N,"8809414660204");
    addBarcode(CLS_REUSE40_N,"8809414660198");
    addBarcode(CLS_REUSE40_N,"8801798000027");
    addBarcode(CLS_REUSE40_N,"8801798000065");
    addBarcode(CLS_REUSE50_N,"8801798000102");
    addBarcode(CLS_REUSE20_N,"8801798000126");
    addBarcode(CLS_REUSE20_N,"8801798000157");
    addBarcode(CLS_REUSE40_N,"8801119807243");
    addBarcode(CLS_REUSE40_N,"8801119807250");
    addBarcode(CLS_REUSE50_N,"8801119807281");
    addBarcode(CLS_REUSE50_N,"8801119807311");
    addBarcode(CLS_REUSE50_N,"8801119807373");
    addBarcode(CLS_REUSE50_N,"8801119807403");
    addBarcode(CLS_REUSE40_N,"8801119807458");
    addBarcode(CLS_REUSE50_N,"8801119807465");
    addBarcode(CLS_REUSE40_N,"8801119807540");
    addBarcode(CLS_REUSE50_N,"8801119807687");
    addBarcode(CLS_REUSE50_N,"8801119807670");
    addBarcode(CLS_REUSE40_N,"8801048951017");
    addBarcode(CLS_REUSE40_N,"8801048941025");
    addBarcode(CLS_REUSE40_N,"8801048941049");
    addBarcode(CLS_REUSE40_N,"8801048951031");
    addBarcode(CLS_REUSE40_N,"8801048911011");
    addBarcode(CLS_REUSE40_N,"8801048101016");
    addBarcode(CLS_REUSE40_N,"8801048921010");
    addBarcode(CLS_REUSE40_N,"8801048921119");
    addBarcode(CLS_REUSE40_N,"8802036000359");
    addBarcode(CLS_REUSE40_N,"8802036000380");
    addBarcode(CLS_REUSE40_N,"8802036000625");
    addBarcode(CLS_REUSE40_N,"8802036000427");
    addBarcode(CLS_REUSE40_N,"8802036000441");
    addBarcode(CLS_REUSE40_N,"8802036000465");
    addBarcode(CLS_REUSE40_N,"8802036000489");
    addBarcode(CLS_REUSE40_N,"8802036000502");
    addBarcode(CLS_REUSE40_N,"8802036000526");
    addBarcode(CLS_REUSE40_N,"8802036000564");


    //qLog(LOGLEVEL_VERY_HIGH, "Classifier", "Error", QString("load Barcode info :  %1 ").arg(lBarcode.size()));

}


int ClassifierApplication::checkBarcodeInfo(const QString &barcodeVal)
{
    for(int i=0; i < lBarcode.size(); i++)
    {

        barcodeInfo *bar = lBarcode.at(i);
        //qLog(LOGLEVEL_VERY_HIGH, "Classifier", "Error", QString("Barcode compare  %1 with %2").arg(barcodeVal).arg(bar->sBarcode));


        if(bar->sBarcode.compare(barcodeVal) ==0)
        {
            return bar->nCode;
        }
    }

    return CLS_ETC;
}


void ClassifierApplication::reportError(uint code)
{

}

void ClassifierApplication::barcodeArrived(const QString &barcode)
{
    qDebug() << "Barcode read : "  << barcode;
    qLog(LOGLEVEL_VERY_HIGH, "Classifier", "Error", QString("Barcode read :  %1 ").arg(barcode));

    if(barcodeValue.compare(barcode))
    {
        QByteArray cmdData;
        cmdData.clear();
        cmdData.append(QChar(REPORT_CLS));

        QByteArray data;
        data.clear();
        data.append(barcode.toLocal8Bit());
        reporter->sendMessage(cmdData, data);

        barcodeValue = barcode;

    }


}


bool ClassifierApplication::notify(QObject *obj, QEvent *event)
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


void ClassifierApplication::run()
{

    try
    {

        startCamera();
        worker = new Worker(*context, "Classifier","CLA", 1, this);
        connect(worker, SIGNAL(finished()), SLOT(quit()));
        connect(worker, SIGNAL(requestReceived(const QString& , const QString& )), SLOT(requestReceived(const QString& , const QString&)));
        //init model

        worker->start();

        reporter = new Publisher(*context,REPORT_IPC_CLS,"CLS" );
        reporter->start();


    }
    catch (std::exception& ex)
    {
        qWarning() << ex.what();
        exit(-1);
    }
}


void ClassifierApplication::requestReceived(const QString& sender, const QString& message)
{

    //qDebug() << "request Arrived : From " << sender;
    //qDebug() << "request Arrived : Message >> " << message;
    QString result;

    qLog(LOGLEVEL_VERY_HIGH, "Classifier", "Info", QString("Request Received :  %1 from  %2").arg(message).arg(sender));
    if(message.compare("InitCamera")== 0)
    {
        result = "OK";
        if(!bCameraOn && !bCameraInitializing)
        {
            setCamera();
            startCamera();
        }




    }
    else if (message.compare("Classifier")== 0)
    {
        bRequested = true;
        requestSender = sender;
#ifdef BARCODEUSE
        if(deviceType)
        {
            barcodeValue.clear();
            //reader->sendTrigger(false);
            reader->sendTrigger(true);
        }
#endif
        takeShot();
        return;
    }
    else if (message.compare("CloseCamera")== 0)
    {

        result = "OK";
        //stopCamera();
        //releaseCamera();
    }
    else if (message.compare("ReleaseCamera")== 0)
    {

        result = "CamOff";
        if(bCameraOn)
        {
            stopCamera();
            releaseCamera();
        }
    }
    else if (message.compare("SetupCamera")== 0)
    {

        result = "CamOn";
        if(!bCameraOn && !bCameraInitializing)
        {
            setCamera();
            startCamera();
        }
    }
    else
    {
        result = "UnKnown Command";

    }
    qLog(LOGLEVEL_VERY_HIGH, "Classifier", "Info", QString("Response send :  %1 to  %2").arg(result).arg(sender));

    worker->replyRequest(sender, result);

}


void ClassifierApplication::displayCameraError()
{
    //qDebug() << "Camera error : " << camera->errorString();
    qLog(LOGLEVEL_VERY_HIGH, "Classifier", "Info", QString("Camera error :  %1").arg(camera->errorString()));

}

void ClassifierApplication::initCamera()
{

    QProcess process;
    //*
    process.start("uvcdynctrl", QStringList() << "--load=/home/superbin/Nephron/resource/camset.txt");
    process.waitForFinished();

    qDebug() << "First Process End";
    process.start("uvcdynctrl", QStringList() << "--load=/home/superbin/Nephron/resource/camset2.txt");
    process.waitForFinished();
    qDebug() << "Second Process End";

}

void ClassifierApplication::setCamera()
{
    //*/
    qLog(LOGLEVEL_VERY_HIGH, "Classifier", "Info", QString("Camera setup start"));

    bCameraInitializing = true;
    camera = new QCamera(QCameraInfo::defaultCamera());

    //grabber = new CameraFrameGrabber();
     //   camera->setViewfinder(grabber);

    connect(camera, SIGNAL(error(QCamera::Error)), this, SLOT(displayCameraError()));


    imageCapture = new QCameraImageCapture(camera);

    QImageEncoderSettings settings = imageCapture->encodingSettings();
    settings.setCodec("image/jpeg");
    settings.setResolution(1920, 1080);
    settings.setQuality(QMultimedia::VeryHighQuality);


    imageCapture->setEncodingSettings(settings);
    camera->setCaptureMode(QCamera::CaptureStillImage);

    connect(imageCapture, SIGNAL(readyForCaptureChanged(bool)),  SLOT(readyForCapture(bool)));
    connect(imageCapture, SIGNAL(imageSaved(int,QString)), SLOT(imageSaved(int,QString)));
    connect(imageCapture, SIGNAL(error(int,QCameraImageCapture::Error,QString)),
            SLOT(displayCaptureError(int,QCameraImageCapture::Error,QString)));

    bCameraOn = true;
    bCameraInitializing = false;
    qLog(LOGLEVEL_VERY_HIGH, "Classifier", "Info", QString("Camera setup end"));

}
void ClassifierApplication::readyForCapture(bool ready)
{
    bReadyCamera = ready;
}

void ClassifierApplication::startCamera()
{
    camera->start();
}

void ClassifierApplication::stopCamera()
{
    if(camera)
    {
        camera->stop();
    }
}
void ClassifierApplication::releaseCamera()
{
    if(camera)
    {
        delete camera;
        camera = NULL;
    }
    if(imageCapture)
    {
        delete imageCapture;
        imageCapture = NULL;
    }

    bCameraOn = false;

}

void ClassifierApplication::takeShot()
{
    QString sPath, cPath;
    QDir sDir;
    QDateTime time = QDateTime::currentDateTime();

    if(bCamTest)
    {
        cPath = sDir.currentPath();
        sPath = time.toString("yyyy-MM-dd-hhmmss");
        switch(nTestMode)
        {
            case 1:
            if(sDir.exists("../CAN")== 0)
                sDir.mkdir("../CAN");
            sPath = sPath.prepend("/../CAN/Nephron_");
            break;
            case 2:
            if(sDir.exists("../PET")== 0)
                sDir.mkdir("../PET");
            sPath = sPath.prepend("/../PET/Nephron_");
            break;
            case 3:
            if(sDir.exists("../RECYCLE")== 0)
                sDir.mkdir("../RECYCLE");
            sPath = sPath.prepend("/../RECYCLE/Nephron_");
            break;
            case 4:
            if(sDir.exists("../REUSE20")== 0)
                sDir.mkdir("../REUSE20");
            sPath = sPath.prepend("/../REUSE20/Nephron_");
            break;
            case 5:
            if(sDir.exists("../REUSE40")== 0)
                sDir.mkdir("../REUSE40");
            sPath = sPath.prepend("/../REUSE40/Nephron_");
            break;
            case 6:
            if(sDir.exists("../REUSE50")== 0)
                sDir.mkdir("../REUSE50");
            sPath = sPath.prepend("/../REUSE50/Nephron_");
            break;
            case 7:
            if(sDir.exists("../REUSE100")== 0)
                sDir.mkdir("../REUSE100");
            sPath = sPath.prepend("/../REUSE100/Nephron_");
            break;
            case 8:
            if(sDir.exists("../REUSE20N")== 0)
                sDir.mkdir("../REUSE20N");
            sPath = sPath.prepend("/../REUSE20N/Nephron_");
            break;
            case 9:
            if(sDir.exists("../REUSE40N")== 0)
                sDir.mkdir("../REUSE40N");
            sPath = sPath.prepend("/../REUSE40N/Nephron_");
            break;
            case 10:
            if(sDir.exists("../REUSE50N")== 0)
                sDir.mkdir("../REUSE50N");
            sPath = sPath.prepend("/../REUSE50N/Nephron_");
            break;
            case 11:
            if(sDir.exists("../REUSE100N")== 0)
                sDir.mkdir("../REUSE100N");
            sPath = sPath.prepend("/../REUSE100N/Nephron_");
            break;
            case 12:
            if(sDir.exists("../ETC")== 0)
                sDir.mkdir("../ETC");
            sPath = sPath.prepend("/../ETC/Nephron_");
            break;
        }

        sPath = sPath.prepend(cPath);

    }
    else
    {
        if(sDir.exists("../IMAGES")== 0)
            sDir.mkdir("../IMAGES");
        cPath = sDir.currentPath();
    //    sPath = time.toString("Nephron_yyyy-MM-dd-hhmmss");
        sPath = time.toString("yyyy-MM-dd-hhmmss");
        sPath = sPath.prepend("/../IMAGES/Nephron_");
        sPath = sPath.prepend(cPath);
    }
    int nret =  imageCapture->capture(sPath);
    qDebug() << "Capture result : " << nret;

}

void ClassifierApplication::imageSaved(int id, const QString &fileName)
{
    //capture complete
    qDebug() << "Capture saved at : " << fileName;
    //stopCamera();
    if(bRequested)
    {
        //Classifier Here
        QString result, dstFile;
        int res0,res1,res2,res3;
        dstFile = fileName;
#ifdef BARCODEUSE
        if(deviceType)
        {
            res3 = checkBarcodeInfo(barcodeValue);
        }
        else
        {
            res3 =  CLS_ETC;
        }

#else
        res3 = CLS_ETC;
#endif


        if(res3 != CLS_ETC)
        {
            switch(res3)
            {
                case CLS_CAN:
                result = "CAN";
                dstFile.replace(".jpg", "_CAN.jpg");
                break;
                case CLS_PET:
                result = "PET";
                dstFile.replace(".jpg", "_PET.jpg");
                break;
                case CLS_RECYLE:
                result = "RECYCLE";
                dstFile.replace(".jpg", "_RECYCLE.jpg");
                break;
                case CLS_REUSE20:
                    result = "REUSE20";
                    dstFile.replace(".jpg", "_REUSE20.jpg");
                break;
                case CLS_REUSE40:
                    result = "REUSE40";
                    dstFile.replace(".jpg", "_REUSE40.jpg");
                break;
                case CLS_REUSE50:
                result = "REUSE50";
                dstFile.replace(".jpg", "_REUSE50.jpg");
                break;
                case CLS_REUSE100:
                result = "REUSE100";
                dstFile.replace(".jpg", "_REUSE100.jpg");
                break;
                case CLS_REUSE20_N:
                    result = "REUSE20_N";
                    dstFile.replace(".jpg", "_REUSE20_NEW.jpg");
                break;
                case CLS_REUSE40_N:
                    result = "REUSE40_N";
                    dstFile.replace(".jpg", "_REUSE40_NEW.jpg");
                break;
                case CLS_REUSE50_N:
                result = "REUSE50_N";
                dstFile.replace(".jpg", "_REUSE50_NEW.jpg");
                break;
                case CLS_REUSE100_N:
                result = "REUSE100_N";
                dstFile.replace(".jpg", "_REUSE100_NEW.jpg");
                break;
                case CLS_ETC:
                default:
                result = "ETC";
                dstFile.replace(".jpg", "_ETC.jpg");
                break;

            }
        }
        else
        {
#if 1

#endif


#if 1
#ifdef ANNOTATION
            res0 = ng0->DetectAndClassifyWithPath(fileName.toStdString());
            res1 = ng1->DetectAndClassifyWithPath(fileName.toStdString());
            //20180226
            res2 = ng2->DetectAndClassifyWithPath(fileName.toStdString());
#else //ANNOTATION

            res0 = ClassifyAugmentedWithPath(classifier0, fileName.toStdString());
            res1 = ClassifyAugmentedWithPath(classifier1, fileName.toStdString());



#endif //ANNOTATION

#else
            QString testedImage;
            testedImage = fileName;
            testedImage.replace(".jpg","withRect.jpg");
            cv::Mat img = cv::imread(fileName.toStdString());
            cv::Rect rect0	= detector0.DetectBbox(img);
            cv::Rect rect1 = detector1.DetectBbox(img);
            cv::Rect rect2 = detector2.DetectBbox(img);
            //cv::rectangle(img,rect0,cv::Scalar(0,255,0),1);
            //cv::rectangle(img,rect1,cv::Scalar(255,0,0),1);
            //cv::rectangle(img,rect2,cv::Scalar(0,0,255),1);

            cv::Rect rect;
            rect.x = 670;
            rect.y = 453;
            rect.width = 875;
            rect.height = 257;
            cv::rectangle(img,rect,cv::Scalar(0,133,133),2);

            cv::imwrite(testedImage.toStdString(),img);

            res0 = classifier0.ClassifyAugmentedWithPath(fileName.toStdString(),rect);
            res1 = classifier1.ClassifyAugmentedWithPath(fileName.toStdString(),rect);
            res2 = classifier2.ClassifyAugmentedWithPath(fileName.toStdString(),rect);


#endif
            qLog(LOGLEVEL_VERY_HIGH, "Classifier", "Error", QString("classify result :  res0(%1) res1(%2) res2(%3)").arg(res0).arg(res1).arg(res2));

            //20180226
            if (res0 != res1 || res0 != res2 || res1 != res2)
            {
               qLog(LOGLEVEL_VERY_HIGH, "Classifier", "Error", QString("ensenble mismatched "));
               res0 = CLS_ETC;
            }



            switch(res0)
            {
                case CLS_CAN:
                result = "CAN";
                dstFile.replace(".jpg", "_CAN.jpg");
                break;
                case CLS_PET:
                result = "PET";
                dstFile.replace(".jpg", "_PET.jpg");
                break;
                case CLS_RECYLE:
                result = "RECYCLE";
                dstFile.replace(".jpg", "_RECYCLE.jpg");
                break;
                case CLS_REUSE20:
                {
                    result = "REUSE20";
                    dstFile.replace(".jpg", "_REUSE20.jpg");
                }
                break;
                case CLS_REUSE40:
                {
                    result = "REUSE40";
                    dstFile.replace(".jpg", "_REUSE40.jpg");
                }
                break;
                case CLS_REUSE50:
                result = "REUSE50";
                dstFile.replace(".jpg", "_REUSE50.jpg");
                break;
                case CLS_REUSE100:
                result = "REUSE100";
                dstFile.replace(".jpg", "_REUSE100.jpg");
                break;
                case CLS_ETC:
                default:
                res0 = CLS_ETC;
                result = "ETC";
                dstFile.replace(".jpg", "_ETC.jpg");
                break;

            }

        }

        QFile::rename(fileName, dstFile);

        if(bCamTest)
        {
            result = "ETC";
            res0 = nTestMode -1;
        }

        if(nImageUploadMode == 0)
        {

            QByteArray cmdData;
            cmdData.clear();
            cmdData.append(QChar(REPORT_CLS));
            QList<QByteArray> data ;
            data.clear();
            QByteArray clsdata;
            clsdata.clear();
            clsdata.append(QString("%1").arg(res0, 2, 10, QChar('0')));
            data.append(clsdata);
            data.append(dstFile.toLocal8Bit());

            clsdata.clear();

            QDateTime currentDatetime = QDateTime::currentDateTime();
            clsdata.append(QString::number(currentDatetime.toTime_t()));
            data.append(clsdata);

            reporter->sendMessage(cmdData, data);
            qLog(LOGLEVEL_VERY_HIGH, "Classifier", "Info", QString("Image Upload Request :  %1(%2) ---> %3").arg(res0, 2, 10, QChar('0')).arg(QString(clsdata)).arg(dstFile));

        }



        worker->replyRequest(requestSender, result);
        bRequested = false;

    }

}
void ClassifierApplication::displayCaptureError(int id, const QCameraImageCapture::Error error, const QString &errorString)
{
    Q_UNUSED(id);
    Q_UNUSED(error);
    //QMessageBox::warning(this, tr("Image Capture Error"), errorString);
   //isCapturingImage = false;
}

