#include "modbusinterface.h"


#include <QModbusRtuSerialMaster>
#include <QTimer>
#include <QSerialPort>
#include <QVariant>
#include <QDebug>
#include <QSettings>

#include "global.hpp"





ModBusInterface::ModBusInterface(int deviceType, QObject *parent) : QObject(parent),m_coils(264, false), m_bFirst(true),m_StorageChangeValue(4,false),m_StorageChangeOldValue(4,false)
{

    m_nDeviceType = deviceType;

    loadPreviousSensorState();

    modbusDevice = new QModbusRtuSerialMaster(this);


    memset(m_StorageChangeEvent,0,sizeof(int)*4);

    connect(modbusDevice, SIGNAL(errorOccurred(QModbusDevice::Error)), SLOT(modbusError(QModbusDevice::Error)));

    connect(modbusDevice, SIGNAL(stateChanged(QModbusDevice::State)), SLOT(onModbusStateChanged(QModbusDevice::State)));


}

ModBusInterface::~ModBusInterface()
{

    //savePreviousSensorState();

}

void ModBusInterface::savePreviousSensorState(int index)
{
    QSettings settings("NTN", "Nephron");

    settings.beginGroup("SensorState");

    if(m_nDeviceType)
    {
        //Glass
        switch(index)
        {

        case GLASS_FULL:
            settings.setValue("GlassFull", m_coils.at(GLASS_FULL));
            break;

        case GLASS_DOOR_TOP_S:
            settings.setValue("GlassDoorTop", m_coils.at(GLASS_DOOR_TOP_S));
            break;
        case GLASS_DOOR_BOTTOM_S:
            settings.setValue("GlassDoorBottom", m_coils.at(GLASS_DOOR_BOTTOM_S));
        break;
        case GLASS_BIN_EMPTY:
            settings.setValue("GlassBinEmpty", m_coils.at(GLASS_BIN_EMPTY));
        break;
        case GLASS_GEARED_M_JAM:
            settings.setValue("GlassGearedMJam", m_coils.at(GLASS_GEARED_M_JAM));
        break;


        }







    }
    else
    {

        //CP
        switch(index)
        {

        case PET_FULL:
            settings.setValue("PetFull", m_coils.at(PET_FULL));
            break;
        case PET_80P:

            settings.setValue("Pet80", m_coils.at(PET_80P));
            break;
        case CAN_FULL:
             settings.setValue("CanFull", m_coils.at(CAN_FULL));
            break;
        case CAN_80P:
            settings.setValue("Can80", m_coils.at(CAN_80P));
            break;
        case DOOR_TOP_S:
            settings.setValue("CPDoorTop", m_coils.at(DOOR_TOP_S));
            break;
        case DOOR_BOTTOM_S:
            settings.setValue("CPDoorBottom", m_coils.at(DOOR_BOTTOM_S));
        break;
        case PET_BIN_EMPTY:
            settings.setValue("CanBinEmpty", m_coils.at(PET_BIN_EMPTY));
            break;
        case CAN_BIN_EMPTY:
            settings.setValue("PetBinEmpty", m_coils.at(CAN_BIN_EMPTY));
        break;

        case GEARED_M_JAM:
            settings.setValue("CPGearedMJam", m_coils.at(GEARED_M_JAM));
        break;

        }

    }
    settings.endGroup();

}


void ModBusInterface::loadPreviousSensorState()
{
    QSettings settings("NTN", "Nephron");

    settings.beginGroup("SensorState");

    if(m_nDeviceType)
    {
        //Glass

        m_coils.setBit(GLASS_FULL, settings.value("GlassFull").toBool());
        m_coils.setBit(GLASS_DOOR_TOP_S, settings.value("GlassDoorTop").toBool());
        m_coils.setBit(GLASS_DOOR_BOTTOM_S, settings.value("GlassDoorBottom").toBool());
        m_coils.setBit(GLASS_BIN_EMPTY, settings.value("GlassBinEmpty").toBool());
        m_coils.setBit(GLASS_GEARED_M_JAM, settings.value("GlassGearedMJam").toBool());

    }
    else
    {

        //CP
        m_coils.setBit(PET_FULL, settings.value("PetFull").toBool());
        m_coils.setBit(PET_80P, settings.value("Pet80").toBool());
        m_coils.setBit(CAN_FULL, settings.value("CanFull").toBool());
        m_coils.setBit(CAN_80P, settings.value("Can80").toBool());
        m_coils.setBit(DOOR_TOP_S, settings.value("CPDoorTop").toBool());
        m_coils.setBit(DOOR_BOTTOM_S, settings.value("CPDoorBottom").toBool());
        m_coils.setBit(PET_BIN_EMPTY, settings.value("CanBinEmpty").toBool());
        m_coils.setBit(CAN_BIN_EMPTY, settings.value("PetBinEmpty").toBool());
        m_coils.setBit(GEARED_M_JAM, settings.value("CPGearedMJam").toBool());
    }
    settings.endGroup();


}

void ModBusInterface::connectToDevice()
{
    if (!modbusDevice)
        return;

    if (modbusDevice->state() != QModbusDevice::ConnectedState)
    {
        /*

        QVariant parity = QSerialPort::NoParity;
        QVariant baudrate = QSerialPort::Baud115200;
        QVariant databits = QSerialPort::Data8;
        QVariant stopbits = QSerialPort::OneStop;
*/
        QVariant portName = QVariant("ttyS0");
        //QVariant portName = QVariant("ttyUSB0");
        modbusDevice->setConnectionParameter(QModbusDevice::SerialPortNameParameter,portName);
        modbusDevice->setConnectionParameter(QModbusDevice::SerialParityParameter,QVariant::fromValue((int)QSerialPort::NoParity));
        modbusDevice->setConnectionParameter(QModbusDevice::SerialBaudRateParameter,qVariantFromValue((m_nDeviceType)?(QSerialPort::Baud9600):(QSerialPort::Baud115200)));
        modbusDevice->setConnectionParameter(QModbusDevice::SerialDataBitsParameter,qVariantFromValue(QSerialPort::Data8));
        modbusDevice->setConnectionParameter(QModbusDevice::SerialStopBitsParameter,qVariantFromValue(QSerialPort::OneStop));

        modbusDevice->setTimeout(1000);
        modbusDevice->setNumberOfRetries(0);
        m_bConnected = modbusDevice->connectDevice();

    }



}


void ModBusInterface::disConnectFromDevice()
{
    modbusDevice->disconnectDevice();
    m_bConnected = false;

}



void ModBusInterface::modbusError(QModbusDevice::Error error)
{
 //qDebug() << "OK receive";
    //qDebug() << "modbusError : " << modbusDevice->errorString() << "(" << error << ")";
    qLog(LOGLEVEL_VERY_HIGH, "PLC", "Error", QString("modbusError:  %1 (%2)").arg(modbusDevice->errorString()).arg(error));


    if(error > QModbusDevice::NoError)
    {

        //error
    }

}

void ModBusInterface::onModbusStateChanged(QModbusDevice::State state)
{

    qLog(LOGLEVEL_VERY_HIGH, "PLC", "Info", QString("onModbusStateChanged:  %1 (%2)").arg(modbusDevice->errorString()).arg(state));

    m_bConnected = (state != QModbusDevice::UnconnectedState);
    emit portConnected(m_bConnected);
    if(state == QModbusDevice::ConnectedState)
    {
        //qDebug() << "Start request >>>>>>>>>>>>>>>>>>>>";
        qLog(LOGLEVEL_VERY_HIGH, "PLC", "Info", QString("onModbusStateChanged: QModbusDevice::ConnectedState"));

        if(m_nDeviceType)
        {

            writeRequest(MOVING_STEP, 0);
            writeRequest(MOVE_STEP, 0);
            writeRequest(RETURN_STEP, 0);
            writeRequest(COMPRESS_STEP, 0);
            writeRequest(CANCEL_SESSIOIN, 0);
            writeRequest(PAC_START, 0);
        }
        else
        {

            writeRequest(PLC_STATUS_RESET, 0);
            writeRequest(PAC_START, 0);
        }

        QTimer::singleShot(0, this, SLOT(sendRequest()));
    }
}


void ModBusInterface::writeRequest(int index, bool value)
{
    qDebug() << "writeRequest(" << index <<", " << value << ")";
    qLog(LOGLEVEL_VERY_HIGH, "PLC", "Info", QString("PLC Register write request (%1) to (%2)").arg(index).arg(value));
    writeCmd *w = new writeCmd(index, value);

    if(w)
    {
        m_writecmd.push_back(w);
    }

    qDebug() << "write procedure stacked >> " << m_writecmd.count();
/*
    if(!m_bWaitForWrite)
    {
        m_nWriteAddress = index;
        m_bWriteValue = value;
        m_bWaitForWrite = true;
    }
    else
    {

        qDebug() << "write procedure busy!!!";
    }

*/
}

void ModBusInterface::sendRequest()
{
    if (!m_bConnected)
        return;
    //qDebug() << "sendRequest : " <<  m_writecmd.count() ;

    if(!m_writecmd.isEmpty())
    {
        //m_bWaitForWrite = false;
        int nStartAddress ;
        int valueSize ;

        writeCmd * w = m_writecmd.takeFirst();
        if(PLC_STATUS_RESET ==  w->m_nWriteAddress)
        {
            nStartAddress = PLC_STATUS_BLOCK_ADDR;
            valueSize = 8;
        }
        else if (IR_STATUS_RESET ==  w->m_nWriteAddress)
        {
            nStartAddress = IR_STATUS_BLOCK_ADDR;
            valueSize = 8;
        }
        else
        {
            nStartAddress = w->m_nWriteAddress;
            valueSize = 1;
        }

        qDebug() << "send write Request : " <<  w->m_nWriteAddress << " >> " << w->m_bWriteValue;
        qLog(LOGLEVEL_VERY_HIGH, "PLC", "Info", QString("send write Request to PLC  (%1) : (%2)").arg(w->m_nWriteAddress).arg(w->m_bWriteValue));

        QModbusDataUnit writeUnit = QModbusDataUnit(QModbusDataUnit::Coils, nStartAddress, valueSize);

        if(PLC_STATUS_RESET ==  w->m_nWriteAddress || IR_STATUS_RESET ==  w->m_nWriteAddress)
        {
            for(int i =0; i < valueSize; i ++)
            {
                writeUnit.setValue(i, 0);
            }

        }
        else
        {
            writeUnit.setValue(0, w->m_bWriteValue);
        }


        delete w;

        if (auto *reply = modbusDevice->sendWriteRequest(writeUnit, 1)) {
            if (!reply->isFinished())
                connect(reply, SIGNAL(finished()), SLOT(writeResult()));
            else
                delete reply; // broadcast replies return immediately
        } else {
            qDebug() << tr("Write error: ") + modbusDevice->errorString();
        }

    }
    else
    {

        QModbusDataUnit readUnit = QModbusDataUnit(QModbusDataUnit::Coils , 0, 264);
        if (auto *reply = modbusDevice->sendReadRequest(readUnit, 1)) {
            if (!reply->isFinished())
            {
                connect(reply, SIGNAL(finished()), SLOT(readReady()));
            }
            else
            {
                delete reply; // broadcast replies return immediately
            }
        }
        else
        {
            qDebug() << tr("Read error: ") + modbusDevice->errorString();
        }

    }

}

void ModBusInterface::incommingDataProcess(int index, bool value)
{


    if(m_coils.at(index) == value)
    {

        return;
    }
    //qDebug() << index << "'th value changed : " << m_coils.at(index) << " --> " << value;
    qLog(LOGLEVEL_VERY_HIGH, "PLC", "Info", QString("%1'th value changed : %2 --> %3").arg(index).arg(m_coils.at(index)).arg(value));

    m_coils.setBit(index,value);
    if(m_nDeviceType)
    {
        switch(index)
        {
        case MOVING_STEP:
            if(value)
            {
			
				//g version only
               //writeRequest(MOVE_STEP, 0);
                emit plcStateChage(PS_GETREADY);
            }

        break;

        case MOVE_STEP:
            if(value)
            {
                //setPLCSoftResetEvent(false);
                emit plcStateChage(PS_MOVINGCLSPOINT);
            }
            /*
            else
            {
                emit plcStateChage(PS_RECYCLINGEND);

            }
            */
        break;
        case COMPRESS_STEP:
            if(value)
            {
                //setPLCSoftResetEvent(false);
                emit plcStateChage(PS_RECYCLINGEND);
            }
            break;
        case CANCEL_SESSIOIN:
            if(value)
            {
                //setPLCSoftResetEvent(false);
                emit plcStateChage(PS_CANCEL_SESSION);
                writeRequest(CANCEL_SESSIOIN, 0);
            }
        break;
        case RETURN_STEP:
            if(value)
            {
                //setPLCSoftResetEvent(false);
                emit plcStateChage(PS_RETURNOBJECT);
            }
        break;
        case RETURN_STEP2:
            if(value)
            {
                //setPLCSoftResetEvent(false);
                emit plcStateChage(PS_RETURNOBJECT2);
            }
        break;
        case GLASS_FULL:
            emit plcErrorDetect(PS_STORAGE_FULL, 1);
            break;

        case GLASS_DOOR_TOP_S:
        case GLASS_DOOR_BOTTOM_S:
            processDoorOpen();
        break;
        case GLASS_BIN_EMPTY:
            emit plcErrorDetect(PS_STORAGE_EMPTY, value);
        break;
        case GLASS_GEARED_M_JAM:
            emit plcErrorDetect(PS_JAM, value);
        break;


        }
        switch(index)
        {
        case GLASS_FULL:
        case GLASS_DOOR_TOP_S:
        case GLASS_DOOR_BOTTOM_S:
        case GLASS_BIN_EMPTY:
        case PLC_STARTIR:
        case PLC_ENDIR:            
        case PLC_G_SORTER_TOP_SENSOR:
        case PLC_G_SORTER_BOTTOM_SENSOR:
        case PLC_G_SLING_TOP_SENSOR:
        case PLC_G_SLING_BOTTOM_SENSOR:
        case PLC_G_TRASH_TOP_SENSOR:
        case PLC_G_TRASH_BOTTOM_SENSOR:
        case PLC_G_TRAY_SENSOR:
            emit sensorStateChange(index, value);
            break;
        }
    }
    else
    {
        switch(index)
        {
        case MOVING_STEP:
            if(value)
            {

                //g version only
               //writeRequest(MOVE_STEP, 0);
                emit plcStateChage(PS_GETREADY);
            }

        break;

        case MOVE_STEP:
            if(value)
            {
                //setPLCSoftResetEvent(false);
                emit plcStateChage(PS_MOVINGCLSPOINT);
            }
            /*
            else
            {
                emit plcStateChage(PS_RECYCLINGEND);

            }
            */
        break;
        case COMPRESS_STEP:
            if(value)
            {
                //setPLCSoftResetEvent(false);
                emit plcStateChage(PS_RECYCLINGEND);
            }
            break;
        case CANCEL_SESSIOIN:
            if(value)
            {
                //setPLCSoftResetEvent(false);
                emit plcStateChage(PS_CANCEL_SESSION);
                writeRequest(CANCEL_SESSIOIN, 0);
            }
        break;
        case RETURN_STEP:
            if(value)
            {
                //setPLCSoftResetEvent(false);
                emit plcStateChage(PS_RETURNOBJECT);
            }
        break;
        case RETURN_STEP2:
            if(value)
            {
                //setPLCSoftResetEvent(false);
                emit plcStateChage(PS_RETURNOBJECT2);
            }
        break;

        case PET_FULL:
            setStorageChangeEvent(0, value);
            savePreviousSensorState(PET_FULL);
            break;
        case PET_80P:
            setStorageChangeEvent(1, value);
            savePreviousSensorState(PET_80P);
            break;
        case CAN_FULL:
            setStorageChangeEvent(2, value);
            savePreviousSensorState(CAN_FULL);
            break;
        case CAN_80P:
            setStorageChangeEvent(3, value);
            savePreviousSensorState(CAN_80P);
            break;

        case DOOR_TOP_S:
            processDoorOpen();
            savePreviousSensorState(DOOR_TOP_S);
            break;
        case DOOR_BOTTOM_S:
            processDoorOpen();            
            savePreviousSensorState(DOOR_BOTTOM_S);
            break;
        break;
        case PET_BIN_EMPTY:
            processStorageEmpty();
            savePreviousSensorState(PET_BIN_EMPTY);
            break;
        case CAN_BIN_EMPTY:
            processStorageEmpty();
            savePreviousSensorState(CAN_BIN_EMPTY);
        break;

        case GEARED_M_JAM:
            savePreviousSensorState(GEARED_M_JAM);
            emit plcErrorDetect(PS_JAM, value);
        break;

        }

        switch(index)
        {
        case PLC_CAN_FULL_SENSOR:
        case PLC_CAN_80_SENSOR:
        case PLC_PET_FULL_SENSOR:
        case PLC_PET_80_SENSOR:
        case DOOR_TOP_S:
        case DOOR_BOTTOM_S:
        case PLC_CAN_TRAY_SENSOR:
        case PLC_PET_TRAY_SENSOR:
        case PLC_STARTIR:
        case PLC_ENDIR:
        case PLC_JAMSENSOR:
        case PLC_SORTER_IDLE:
        case PLC_SORTER_CENTER:
        case PLC_SORTER_LEFT:
        case PLC_SORTER_RIGHT:
            emit sensorStateChange(index, value);
            break;
        }

    }

    //sm_coils.setBit(index,value);

}


void ModBusInterface::processStorageEmpty()
{
    char openMode = 0;
    if(m_coils.at(PET_BIN_EMPTY))
    {
        openMode |= (1 << 0);
    }

    if(m_coils.at(CAN_BIN_EMPTY))
    {
        openMode |= (1 << 1);
    }


    qLog(LOGLEVEL_VERY_HIGH, "PLC", "Info", QString("processStorageEmpty : (%1)").arg(int(openMode)));
    emit plcErrorDetect(PS_STORAGE_EMPTY, openMode);

}
void ModBusInterface::processDoorOpen()
{
    char openMode = 0;
    if(m_nDeviceType)
    {
        if(m_coils.at(GLASS_DOOR_TOP_S))
        {
            openMode |= (1 << 0);
        }

        if(m_coils.at(GLASS_DOOR_BOTTOM_S))
        {
            openMode |= (1 << 1);
        }
    }
    else
    {
        if(m_coils.at(DOOR_TOP_S))
        {
            openMode |= (1 << 0);
        }

        if(m_coils.at(DOOR_BOTTOM_S))
        {
            openMode |= (1 << 1);
        }
    }
    qLog(LOGLEVEL_VERY_HIGH, "PLC", "Info", QString("processDoorOpen : (%1)").arg(int(openMode)));
    emit plcErrorDetect(PS_DOOROPEN, openMode);

}
void ModBusInterface::setStorageChangeEvent(uint nCate, bool value)
{
    if(nCate>=4)
        return;
    qLog(LOGLEVEL_VERY_HIGH, "PLC", "Info", QString("setStorageChangeEvent(%1, %2)").arg(nCate).arg(value));
    m_StorageChangeEvent[nCate] = 100;
    m_StorageChangeValue[nCate] = value;
}


void ModBusInterface::setPLCSoftResetEvent(bool bSet)
{
    if(bSet)
    {
        qLog(LOGLEVEL_VERY_HIGH, "PLC", "Error", QString("PLC Soft Reset register"));
        m_PLCSoftResetEvent = 300;
    }
    else
    {
        qLog(LOGLEVEL_VERY_HIGH, "PLC", "Error", QString("PLC Soft Reset Unregister"));
        m_PLCSoftResetEvent = 0;
    }
}

void ModBusInterface::processPLCSoftReset()
{
    bool reset = false;
    if(m_PLCSoftResetEvent == 1)
    {
        reset = true;
        m_PLCSoftResetEvent = 0;
    }
    else if(m_PLCSoftResetEvent > 1)
    {

        --m_PLCSoftResetEvent;
    }

    if(reset)
    {
        qLog(LOGLEVEL_VERY_HIGH, "PLC", "Error", QString("PLC Soft Reset===="));
        writeRequest(PLC_STATUS_RESET, 0);
        writeRequest(IR_STATUS_RESET, 0);
    }
}

void ModBusInterface::processStorage()
{
    QBitArray apply(4);
    bool noti= false;
    apply.fill(false);
    for(int i = 0 ; i < apply.size(); i++)
    {
        if(m_StorageChangeEvent[i] == 1)
        {
            if(m_StorageChangeValue.at(i) != m_StorageChangeOldValue.at(i))
            {
                apply.setBit(i,true);
                noti = true;
            }
            m_StorageChangeEvent[i] = 0;
        }
        else if(m_StorageChangeEvent[i] > 1)
        {

            --m_StorageChangeEvent[i];
        }

    }


    if(noti)
    {
        char openMode = 0;

        for(int i = 0 ; i < apply.size(); i++)
        {
            if(apply.at(i))
            {
                if(m_StorageChangeValue.at(i))
                {
                    openMode |= (1 << i);
                }
                m_StorageChangeOldValue.setBit(i,m_StorageChangeValue.at(i));
            }
            else
            {
                if(m_StorageChangeOldValue.at(i))
                {
                    openMode |= (1 << i);
                }
            }
        }

        qLog(LOGLEVEL_VERY_HIGH, "PLC", "Info", QString("process Storage : (%1)").arg(int(openMode)));
        emit plcErrorDetect(PS_STORAGE_FULL, openMode);

    }



}

int  ModBusInterface::getState()
{
    int nRet = 0;
    if(m_nDeviceType)
    {
        if(m_coils.at(GLASS_FULL))
        {
            nRet |= (1 << 0);
        }
        if(m_coils.at(GLASS_DOOR_TOP_S))
        {
            nRet |= (1 << 1);
        }
        if(m_coils.at(GLASS_DOOR_BOTTOM_S))
        {
            nRet |= (1 << 2);
        }
        if(m_coils.at(GLASS_BIN_EMPTY))
        {
            nRet |= (1 << 3);
        }
        if(m_coils.at(GLASS_GEARED_M_JAM))
        {
            nRet |= (1 << 4);
        }


    }
    else
    {
        if(m_coils.at(PET_FULL))
        {
            nRet |= (1 << 0);
        }
        if(m_coils.at(PET_80P))
        {
            nRet |= (1 << 1);
        }
        if(m_coils.at(CAN_FULL))
        {
            nRet |= (1 << 2);
        }
        if(m_coils.at(CAN_80P))
        {
            nRet |= (1 << 3);
        }
        if(m_coils.at(DOOR_TOP_S))
        {
            nRet |= (1 << 4);
        }
        if(m_coils.at(DOOR_BOTTOM_S))
        {
            nRet |= (1 << 5);
        }
        if(m_coils.at(PET_BIN_EMPTY))
        {
            nRet |= (1 << 6);
        }
        if(m_coils.at(CAN_BIN_EMPTY))
        {
            nRet |= (1 << 7);
        }
        if(m_coils.at(GEARED_M_JAM))
        {
            nRet |= (1 << 8);
        }

    }


    return nRet;

}

void ModBusInterface::writeResult()
{
    QModbusReply *reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    if (reply->error() == QModbusDevice::NoError) {

        const QModbusDataUnit unit = reply->result();
        qDebug() << "Write OK!! : at " << unit.startAddress() ;
        qLog(LOGLEVEL_VERY_HIGH, "PLC", "Info", QString("PLC Register Write OK!! : at  :%1 (%2)").arg(unit.startAddress()).arg(int(unit.value(0))));

    } else if (reply->error() == QModbusDevice::ProtocolError) {
        qDebug() << tr("Read response error: %1 (Mobus exception: 0x%2)").
                                    arg(reply->errorString()).
                                    arg(reply->rawResult().exceptionCode(), -1, 16);
    } else {
        qDebug() << tr("Read response error: %1 (code: 0x%2)").
                                    arg(reply->errorString()).
                                    arg(reply->error(), -1, 16);
    }

    reply->deleteLater();
    QTimer::singleShot(0, this, SLOT(sendRequest()));
}

void ModBusInterface::readReady()
{
    QModbusReply *reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        for (uint i = 0; i < unit.valueCount(); i++) {
            //qDebug() <<  tr("Address: %1, Value: %2").arg(unit.startAddress())
            //                         .arg(QString::number(unit.value(i),
            //                              unit.registerType() <= QModbusDataUnit::Coils ? 10 : 16));
            //ui->readValue->addItem(entry);
            incommingDataProcess(i,unit.value(i));
        }

        processStorage();
        processPLCSoftReset();
    } else if (reply->error() == QModbusDevice::ProtocolError) {
        qDebug() << tr("Read response error: %1 (Mobus exception: 0x%2)").
                                    arg(reply->errorString()).
                                    arg(reply->rawResult().exceptionCode(), -1, 16);
    } else {
        qDebug() << tr("Read response error: %1 (code: 0x%2)").
                                    arg(reply->errorString()).
                                    arg(reply->error(), -1, 16);
    }

    reply->deleteLater();
    if(m_bFirst)
    {
        m_bFirst= false;
    }
    QTimer::singleShot(0, this, SLOT(sendRequest()));
}


