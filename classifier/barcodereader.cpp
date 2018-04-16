#include "barcodereader.h"
#include <QDebug>
#include "global.hpp"
#include <QTimer>

#define TRIGGER_ON "||>trigger on"
#define TRIGGER_OFF "||>trigger off"


BarcodeReader::BarcodeReader(QObject *parent) : QObject(parent)
{
    serial = new QSerialPort(this);



    connect(serial, SIGNAL(error(QSerialPort::SerialPortError)), SLOT(handleError(QSerialPort::SerialPortError)));
    connect(serial,SIGNAL(readyRead()), this, SLOT(handleRead()));

    QTimer::singleShot(0, this, SLOT(reconnect()));
}

void BarcodeReader::reconnect()
{

    serial->setPortName("/dev/ttyACM0");
    serial->setBaudRate( QSerialPort::Baud9600);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);

    if (serial->open(QIODevice::ReadWrite)) {

        qLog(LOGLEVEL_VERY_HIGH, "Classifier", "Info", QString("barcode reader oPen succes"));

        //sendTrigger();

    }
    else
    {
        qLog(LOGLEVEL_VERY_HIGH, "Classifier", "Error", QString("barcode reader oPen Failed!!!!"));
        QTimer::singleShot(1000, this, SLOT(reconnect()));
    }

}

void BarcodeReader::handleRead()
{
    QByteArray data = serial->readAll();

    qLog(LOGLEVEL_VERY_HIGH, "Classifier", "Error", QString("Incomming Message From Barcode reader :  %1 ").arg(QString(data.toHex())));

    emit coderead(QString(data));

}

void BarcodeReader::handleError(QSerialPort::SerialPortError error)
{
    qLog(LOGLEVEL_VERY_HIGH, "Classifier", "Error", QString("Barcode handleError :  %1 ").arg(error));

}

void BarcodeReader::sendTrigger(bool bOn)
{
    if(bOn)
    {
        qLog(LOGLEVEL_VERY_HIGH, "Classifier", "Error", QString("Barcode sendTrigger :  ON "));

        sendString(TRIGGER_ON);
    }
    else
    {
        qLog(LOGLEVEL_VERY_HIGH, "Classifier", "Error", QString("Barcode sendTrigger :  OFF "));

        sendString(TRIGGER_OFF);
    }
}

void BarcodeReader::sendString(const char *str)
{

    QByteArray data;
    QString sTemp;



    data.clear();
    data.append(str);
    data.append(0x0d);
    data.append(0x0a);
    sTemp = data.toHex();

    qLog(LOGLEVEL_VERY_HIGH, "Classifier", "Error", QString("Barcode command :  %1 ").arg(sTemp));

    serial->write(data);
    serial->waitForBytesWritten(-1);

}
