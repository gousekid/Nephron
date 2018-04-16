#include "plctask.h"
#include "global.hpp"
#include <QTimer>

//#define PLC_PORTNAME      "ttyUSB0"
#define PLC_PORTNAME      "ttyS0"
//#define PLC_BAUDRATE      QSerialPort::Baud57600
#define PLC_BAUDRATE      QSerialPort::Baud9600
#define PLC_DATABITS      QSerialPort::Data8
#define PLC_PARITY        QSerialPort::NoParity
#define PLC_STOPBITS      QSerialPort::OneStop
#define PLC_FLOWCONTROL   QSerialPort::NoFlowControl
//#define PLC_FLOWCONTROL   QSerialPort::HardwareControl

#define ENQ     5
#define ACK     6
#define NAK     15
#define EOT     4
#define ETX     3

#define READVAL         'R'
#define WRITEVAL        'W'
#define MONITORREG      'X'
#define MONITORVAL      'Y'

#define PLCCMD_MOVESTEP "MOVE_STEP"
#define PLCCMD_MOVINGSTEP "MOVING_STEP"
#define PLCCMD_SORTSTEP "SORT_STEP"
#define PLCCMD_COMPRESSSTEP "COMPRESS_STEP"
#define PLCCMD_STORAGESTEP "STORAGE_STEP"
#define PLCCMD_RETURNSTEP "RETURN_STEP"

#define PLCCMD_DOORSTATE "DOOR_STATE"
#define PLCCMD_BINSTATE "BIN_STATE"
#define PLCCMD_FULLSTATE "FULL_STATE"


#define PLCCMD_MOTORERROR "MOTOR_ERRROR"




#include <QSerialPortInfo>
#include <QDebug>

PLCTask::PLCTask(QObject *parent) : QObject(parent)
{
    plcData.resize(24);

    serial = new QSerialPort(this);

    for (QSerialPortInfo port : QSerialPortInfo::availablePorts())
    {
        //Their is some sorting to do for just list the port I want, with vendor Id & product Id
        qDebug() << port.portName() << port.vendorIdentifier() << port.productIdentifier()
                 << port.hasProductIdentifier() << port.hasVendorIdentifier() << port.isBusy();

    }

    connect(serial, SIGNAL(error(QSerialPort::SerialPortError)), SLOT(handleError(QSerialPort::SerialPortError)));
    //connect(serial, SIGNAL(readyRead), SLOT(handleRead));
    connect(serial, &QSerialPort::readyRead, this, &PLCTask::handleRead);
    connect(this, SIGNAL(registerAck(uint)), SLOT(registerCommandAcked(uint)));
}


void PLCTask::reconnect()
{
    bool connected = false;
    serial->setPortName(PLC_PORTNAME);
    serial->setBaudRate( PLC_BAUDRATE);
    serial->setDataBits(PLC_DATABITS);
    serial->setParity(PLC_PARITY);
    serial->setStopBits(PLC_STOPBITS);
    serial->setFlowControl(PLC_FLOWCONTROL);

    qDebug() << "PLC oPen try---->";
    if (serial->open(QIODevice::ReadWrite)) {
        qDebug() << "PLC oPen Success!!!!";
        connected = true;

        //registerMonitor(0,1,"%MB0");

        for(int i = 0 ; i < 8 ; i++)
        {
           //monitoringStart(i);
           //serial->waitForReadyRead(-1);
        }

    }
    else
    {
        qDebug() << "Printer oPen Failed!!!!";

    }
    emit portConnected(connected);

}

void PLCTask::registerCommandAcked(uint regNum )
{
    qDebug() << "registerCommandAcked : " << regNum;
    monitoringStart(0);
    /*
    switch(regNum)
    {
    case 0:
         registerMonitor(1,1,"%MX1.4");
        break;
    case 1:
        break;
        registerMonitor(2,1,"%MX1.5");
        break;
    case 2:
        registerMonitor(3,1,"%MX1.6");
        break;
    case 3:
        registerMonitor(4,1,"%MX1.7");
        break;
    case 4:
        registerMonitor(5,1,"%MX2.1");
        break;
    case 5:
        registerMonitor(6,1,"%MX2.4");
        break;
    case 6:
        registerMonitor(7,1,"%MX2.5");
        break;
    case 7:
      qDebug() << "End Register monitor";
        break;
    }

*/
}

void PLCTask::closePort()
{
    if (serial->isOpen())
        serial->close();

}

void PLCTask::readRequest(uint length, const QString &variableName)
{
    QByteArray data;
    QString digitStr = "%1";

    data += ENQ; //head
    data += digitStr.arg(0, 2, 10, QChar('0')); //
    data += READVAL;
    data += "SS";
    data += digitStr.arg(1, 2, 10, QChar('0'));
    data += digitStr.arg(variableName.length(), 2, 10, QChar('0'));
    data += variableName;
    data += EOT;

    qDebug() << "Message to send --> " << data;

    serial->write(data, data.length());

   // qDebug() << "Message to send --> " << data;
   /*
    for(int i = 0 ; i < data.length() ; i++)
    {
       // if(i == 1)continue;
       serial->putChar(data.at(i));
       serial->flush();
       qDebug() << "Message sented() --> "<< i <<"///   " << QChar(data[i]);

    }
    //*/
    //QTimer::singleShot(1000, this, SLOT(testTick()));
}

void PLCTask::writeRequest(uint length,  const QString &variableName, uint value)
{
    QByteArray data;
    QString digitStr = "%1";

    data += ENQ; //head
    data += digitStr.arg(0, 2, 10, QChar('0')); //
    data += WRITEVAL;
    data += "SS";
    data += digitStr.arg(1, 2, 10, QChar('0'));
    data += digitStr.arg(variableName.length(), 2, 10, QChar('0'));
    data += variableName;
    data += digitStr.arg(value, 2, 10, QChar('0'));
    data += EOT;
    qDebug() << "writeRequest >> " << data;
    //waitForSerailIdle();
    //bBusy = true;
    serial->write(data, data.length());
}

void PLCTask::writeRequest(const QString &variableName)
{
    QByteArray data;
    data += ENQ; //head
    data += variableName;
    data += EOT;
    qDebug() << "writeRequest >> " << data;
    //waitForSerailIdle();
    //bBusy = true;
    serial->write(data, data.length());
}


void PLCTask::registerMonitor()
{
    QByteArray data;
    QString digitStr = "%1";

    data.append(ENQ); //head
    data.append(digitStr.arg(0,2,10,QChar('0'))); //
    data.append(MONITORREG);
    data.append(digitStr.arg(0,2,10,QChar('0')));//QString("0%1").arg(regNum);//digitStr.arg(regNum);
    data.append("RSB");

    data.append(digitStr.arg(4,2,10,QChar('0')));
    data.append("%MB0");
    data.append(digitStr.arg(3,2,10,QChar('0')));

    data.append(EOT);


    qDebug() << "register data :: " << data;

    serial->write(data, data.length());
}


void PLCTask::registerMonitor(uint regNum, uint length,  const QString &variableName  )
{

    QByteArray data;
    QString digitStr = "%1";

    data.append(ENQ); //head
    data.append(digitStr.arg(0,2,10,QChar('0'))); //
    data.append(MONITORREG);
    data.append(digitStr.arg(regNum,2,10,QChar('0')));//QString("0%1").arg(regNum);//digitStr.arg(regNum);
    data.append("RSS");
    data.append(digitStr.arg(length,2,10,QChar('0')));
    data.append(digitStr.arg(variableName.length(),2,10,QChar('0')));
    data.append(variableName);
    data += EOT;

    qDebug() << "register data :: " << data;

    serial->write(data, data.length());
    PLCVariable *variable = new PLCVariable(variableName,length);
    regVariable.insert(regNum, variable);

}

void PLCTask::monitoringStart(uint regNum)
{
    QByteArray data;
    QString digitStr = "%1";

    data.append(ENQ); //head
    data.append(digitStr.arg(0,2,10,QChar('0'))); //
    data.append(MONITORVAL);
    data.append(digitStr.arg(regNum,2,10,QChar('0')));
    data.append(EOT);
    qDebug() << "monitoringStart (" << regNum << ") : " <<  data;

    serial->write(data, data.length());

}

void PLCTask::processIncommingMessage(const QByteArray& message)
{

    qDebug() << "Incomming Message From PLC  : " << message;
    QByteArray msg;

    for(int i = 0; i < message.length(); i++)
    {
        if(message.at(i) == ACK || message.at(i)==ENQ)
        {
            msg = message.mid(i);
            break;
        }
    }

    if(msg.length() == 0)
    {
        qDebug() << "Message Format Error  : " << message;
        return;
    }
    qDebug() << "Filtered Message   : " << msg;

    char result =  msg[0];
    //char Command = msg[3];

    //qDebug() << message;
    //head
    if(result == ACK)
    {
        /*
        switch(msg)
        {
            case MONITORVAL:
            {
                uint regNum = msg[5] - 48;
                uint count = msg[7] - 48;



            }

            break;

        case MONITORREG:
            {
                uint regNum = msg[5] - 48;
                qDebug() << "Register OK :" << regNum;
                emit registerAck(regNum);
            }
            break;
        case WRITEVAL:

            break;
        case READVAL:

            break;
        }
        if(Command == MONITORVAL)
        {


        }

        */
    }
    else if (result == ENQ)
    {

        QString sCmd = msg.mid(1,msg.length()-3);
        char cParam = msg.at(msg.length() - 2);
        qDebug() << "From PLC ("<< sCmd<< ")  ";

        if(sCmd.compare(PLCCMD_MOVESTEP)==0)
        {
            emit plcStateChage(PS_MOVINGCLSPOINT);
        }
        else if(sCmd.compare(PLCCMD_MOVINGSTEP)==0)
        {
            emit plcStateChage(PS_GETREADY);
        }
        else if(sCmd.compare(PLCCMD_SORTSTEP)==0)
        {
            emit plcStateChage(PS_SORTERMOVE);
        }
        else if(sCmd.compare(PLCCMD_COMPRESSSTEP)==0)
        {
            //emit plcStateChage(PS_RECYCLINGPROCEED);
            emit plcStateChage(PS_RECYCLINGEND);

        }
        else if(sCmd.compare(PLCCMD_STORAGESTEP)==0)
        {
            emit plcStateChage(PS_CHECKSTORAGE);
        }
        else if(sCmd.compare(PLCCMD_RETURNSTEP)==0)
        {

            emit plcStateChage(PS_RETURNOBJECT);
        }
        else if(sCmd.compare(PLCCMD_DOORSTATE)==0)
        {
            emit plcErrorDetect(PS_DOOROPEN, cParam);
        }
        else if(sCmd.compare(PLCCMD_BINSTATE)==0)
        {
            emit plcErrorDetect(PS_STORAGE_EMPTY, cParam);
        }
        else if(sCmd.compare(PLCCMD_FULLSTATE)==0)
        {
            //emit plcErrorDetect(PS_STORAGE_FULL, cParam);
            emit plcStateChage(PS_RECYCLINGEND);
        }
        else if(sCmd.compare(PLCCMD_MOTORERROR)==0)
        {

            emit plcErrorDetect(PS_JAM, cParam);
        }
        else
        {

            //unKnown
        }



    }



}


void PLCTask::handleRead()
{
    QByteArray data = serial->readAll();
    qDebug() << "Incomming Message From PLC data" << data;

    for(int i =0; i < data.length(); i++)
    {
        if(data[i] == EOT || data[i] == ETX)
        {
            tempData += data[i];
            processIncommingMessage(tempData);

            tempData.clear();
            //readRequest(1,"dd");
        }
        else
        {
            tempData += data[i];
        }

    }


//treat printer report
}

void PLCTask::byteswritten(qint64 )
{

}

void PLCTask::writeData(const QByteArray &data)
{
    serial->write(data);
}

void PLCTask::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        //QMessageBox::critical(this, tr("Critical Error"), serial->errorString());
        closePort();
    }
    else
    {
        emit reportError(error);
    }
}
