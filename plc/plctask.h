#ifndef PLCTASK_H
#define PLCTASK_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QMap>
#include <QBitArray>
#include <QThread>
#define IO_START_PROC   "%MX0"

#define IO_PET_DETECTED   "%MX8"
#define IO_CAN_DETECTED   "%MX9"
#define IO_ETC_DETECTED   "%MX10"

#define IO_PET_STOREFULL   "%MX11"
#define IO_CAN_STOREFULL   "%MX12"
#define IO_PET_STORE80   "%MX13"
#define IO_CAN_STORE80   "%MX14"

#define IO_MOTOR_FAIL   "%MX16"
#define IO_STORAGE_IN   "%MX20"
#define IO_DOOR_OPEN    "%MX21"






struct PLCVariable
{
    QString         m_variableName;   //  Address of worker
    QByteArray      m_value;      //  Owning service, if known
    uint            m_length;         //  Expires at unless heartbeat

    PLCVariable(const QString &variableName, uint length) {
       m_variableName = variableName;
       m_length = length;
       m_value.resize(length);
       m_value = 0;
    }
};



class PLCTask : public QObject
{
    Q_OBJECT
public:
    explicit PLCTask(QObject *parent = 0);
    ~PLCTask()
    {


    }
    void closePort();
    void reconnect();
    void writeData(const QByteArray &data);
    void readRequest(uint length, const QString &variableName);
    void writeRequest(uint length,  const QString &variableName, uint value);
    void writeRequest(const QString &variableName);
    void registerMonitor(uint regNum, uint length,  const QString &variableName  );
    void monitoringStart(uint regNum);
    void registerMonitor();

private:
    QSerialPort *serial;
    QMap<int, PLCVariable*> regVariable;    //  Hash of known workers
    QByteArray tempData;

    void processIncommingMessage(const QByteArray& message);


    bool bInitMonitor;
    bool nCurMonitorIdx;



    QBitArray plcData;




protected:
    static void waitForSerailIdle();

signals:

    void reportError(uint);

    void plcStateChage(uint);
    void portConnected(bool);
    void plcErrorDetect(uint, char);

    void registerAck(uint);


public slots:

    void handleRead();
    void byteswritten(qint64 );
    void handleError(QSerialPort::SerialPortError error);
    void registerCommandAcked(uint regNum);
};

#endif // PLCTASK_H
