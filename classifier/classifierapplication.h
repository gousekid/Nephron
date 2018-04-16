#ifndef CLASSIFIERAPPLICATION_H
#define CLASSIFIERAPPLICATION_H


#include <QObject>
#include <QCoreApplication>
#include <QCamera>
#include <QCameraImageCapture>

#include "worker.h"
#include "zmsg.h"
#include "publisher.h"

#include "neurogenie.hpp"

#include <iostream>

#include "cameraframegrabber.h"

#include "barcodereader.h"
#define ANNOTATION


struct barcodeInfo
{
    int nCode;
    QString sBarcode;
    barcodeInfo(int code, const QString & barcode) {
       nCode = code;
       sBarcode = barcode;
    }
};


class ClassifierApplication : public QCoreApplication
{

    Q_OBJECT

    typedef QCoreApplication super;

public:
    explicit ClassifierApplication(int& argc, char** argv);
    ~ClassifierApplication()
    {
        stopCamera();
        delete imageCapture;
        delete camera;
        if(reporter)
        {
            reporter->stop();
            delete reporter;
        }
#ifdef ANNOTATION
        if(ng0)
        {
            delete ng0;
            ng0 = NULL;
        }

        if(ng1)
        {
            delete ng1;
            ng1 = NULL;
        }

        if(ng2)
        {
            delete ng2;
            ng2 = NULL;
        }

#endif

    }

    bool notify(QObject *obj, QEvent *event);
    void initCamera();
    void setCamera();
    void startCamera();
    void stopCamera();
    void releaseCamera();
    void takeShot();


signals:
    void finished();
    void failure(const QString& what);



protected slots:
    void run();
    void requestReceived(const QString& sender, const QString& message);
    void displayCameraError();

    void readyForCapture(bool ready);
    void imageSaved(int id, const QString &fileName);

    void displayCaptureError(int, QCameraImageCapture::Error, const QString &errorString);

    void reportError(uint code);
    void barcodeArrived(const QString& barcode);

protected:


private:
    ZMQContext* context;
    Worker* worker;
    QCamera* camera;
    QCameraImageCapture *imageCapture;
    QImageEncoderSettings imageSettings;

    BarcodeReader *reader;

    Classifier classifier0;
    Classifier classifier1;
    Classifier classifier2;

#ifdef ANNOTATION
    Detector detector0;
    Detector detector1;
    Detector detector2;

    Neurogenie *ng0;
    Neurogenie *ng1;
    Neurogenie *ng2;
#endif

    //CameraFrameGrabber*  grabber;

    bool bReadyCamera;

    bool bCameraOn;
    bool bCameraInitializing;

    QString requestSender;
    bool bRequested;

    bool bCamTest;
    int nImageUploadMode;
    uint nTestMode;
    Publisher* reporter;

    QString barcodeValue;

    QList<barcodeInfo*> lBarcode={};

    void loadBarcodeValue();
    void addBarcode(int code, const QString & barcodeVal);
    int checkBarcodeInfo(const QString& barcodeVal);

    int deviceType;


};
#endif // CLASSIFIERAPPLICATION_H
