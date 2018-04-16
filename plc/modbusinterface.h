#ifndef MODBUSINTERFACE_H
#define MODBUSINTERFACE_H

#include <QObject>
#include <QModbusDevice>
#include <QModbusDataUnit>
#include <QModbusReply>
#include <QModbusClient>
#include <QBitArray>



struct writeCmd
{
    int m_nWriteAddress;
    bool m_bWriteValue;
    writeCmd(int command, bool value) {
       m_nWriteAddress = command;
       m_bWriteValue = value;
    }
};

class ModBusInterface : public QObject
{
    Q_OBJECT
public:
    explicit ModBusInterface(int deviceType, QObject *parent = 0);

    ~ModBusInterface();

    void connectToDevice();

    void writeRequest(int index, bool value);
    int getState();

private:

    void disConnectFromDevice();
    void incommingDataProcess(int index, bool value);
    void processDoorOpen();
    void processStorage();
    void processStorageEmpty();

    void processPLCSoftReset();

    void setStorageChangeEvent(uint nCate, bool value);

    void setPLCSoftResetEvent(bool bSet);

    void loadPreviousSensorState();
    void savePreviousSensorState(int index);

    int m_StorageChangeEvent[4];
    QBitArray m_StorageChangeValue;
    QBitArray m_StorageChangeOldValue;

    int m_PLCSoftResetEvent;


    QModbusReply* lastRequest;
    QModbusClient* modbusDevice;

    QBitArray m_coils;
    QBitArray m_input;



    bool m_bConnected;

    bool m_bFirst;

    int m_nDeviceType;


    //bool m_bWaitForWrite;

    QList<writeCmd*> m_writecmd={};

signals:

    void reportError(uint);

    void plcStateChage(uint);
    void portConnected(bool);
    void plcErrorDetect(uint, char);

    void registerAck(uint);

    void sensorStateChange(int addr, bool value);

public slots:

    void sendRequest();

    void readReady();
    void writeResult();

    void modbusError(QModbusDevice::Error error);
    void onModbusStateChanged(QModbusDevice::State state);
};

#endif // MODBUSINTERFACE_H
