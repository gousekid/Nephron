#include "pcieio.h"
#include <QSettings>
#include <QTimer>
#include <QDebug>
#include <QtDebug>
#include "global.hpp"
#include <QThread>
#include <QElapsedTimer>


bool g_sorterClose;
int g_sorterNotCloseCount;
int g_crusherStopCount;     // (17.08.21) YC add to delay crusher stopping


pcieIO::pcieIO(int deviceType, QObject *parent) : QObject(parent),m_coils(264, false), m_bFirst(true),m_StorageChangeValue(4,false),m_StorageChangeOldValue(4,false)
{

    m_nDeviceType = deviceType;

    loadPreviousSensorState();

    memset(m_StorageChangeEvent,0,sizeof(int)*4);



//yc    connect(modbusDevice, SIGNAL(errorOccurred(QModbusDevice::Error)), SLOT(modbusError(QModbusDevice::Error)));

//yc    connect(modbusDevice, SIGNAL(stateChanged(QModbusDevice::State)), SLOT(onModbusStateChanged(QModbusDevice::State)));

    SorterAction[TARGET_LEFT][SORTER_SENSOR_STATE_UNKNOWN][SORTER_MOTOR_STATE_STOP] = SORTER_MOTOR_STATE_LEFT;
    SorterAction[TARGET_LEFT][SORTER_SENSOR_STATE_UNKNOWN][SORTER_MOTOR_STATE_STOP_WAIT] = SORTER_MOTOR_STATE_STOP_WAIT;
    SorterAction[TARGET_LEFT][SORTER_SENSOR_STATE_UNKNOWN][SORTER_MOTOR_STATE_LEFT] = SORTER_MOTOR_STATE_LEFT;
    SorterAction[TARGET_LEFT][SORTER_SENSOR_STATE_UNKNOWN][SORTER_MOTOR_STATE_RIGHT] = SORTER_MOTOR_STATE_STOP_WAIT;

    SorterAction[TARGET_LEFT][SORTER_SENSOR_STATE_LEFT][SORTER_MOTOR_STATE_STOP] = SORTER_MOTOR_STATE_STOP;
    SorterAction[TARGET_LEFT][SORTER_SENSOR_STATE_LEFT][SORTER_MOTOR_STATE_STOP_WAIT] = SORTER_MOTOR_STATE_STOP_WAIT;
    SorterAction[TARGET_LEFT][SORTER_SENSOR_STATE_LEFT][SORTER_MOTOR_STATE_LEFT] = SORTER_MOTOR_STATE_STOP_WAIT;
    SorterAction[TARGET_LEFT][SORTER_SENSOR_STATE_LEFT][SORTER_MOTOR_STATE_RIGHT] = SORTER_MOTOR_STATE_STOP_WAIT;

    SorterAction[TARGET_LEFT][SORTER_SENSOR_STATE_RIGHT][SORTER_MOTOR_STATE_STOP] = SORTER_MOTOR_STATE_LEFT;
    SorterAction[TARGET_LEFT][SORTER_SENSOR_STATE_RIGHT][SORTER_MOTOR_STATE_STOP_WAIT] = SORTER_MOTOR_STATE_STOP_WAIT;
    SorterAction[TARGET_LEFT][SORTER_SENSOR_STATE_RIGHT][SORTER_MOTOR_STATE_LEFT] = SORTER_MOTOR_STATE_LEFT;
    SorterAction[TARGET_LEFT][SORTER_SENSOR_STATE_RIGHT][SORTER_MOTOR_STATE_RIGHT] = SORTER_MOTOR_STATE_STOP_WAIT;

    SorterAction[TARGET_LEFT][SORTER_SENSOR_STATE_CENTER][SORTER_MOTOR_STATE_STOP] = SORTER_MOTOR_STATE_LEFT;
    SorterAction[TARGET_LEFT][SORTER_SENSOR_STATE_CENTER][SORTER_MOTOR_STATE_STOP_WAIT] = SORTER_MOTOR_STATE_STOP_WAIT;
    SorterAction[TARGET_LEFT][SORTER_SENSOR_STATE_CENTER][SORTER_MOTOR_STATE_LEFT] = SORTER_MOTOR_STATE_LEFT;
    SorterAction[TARGET_LEFT][SORTER_SENSOR_STATE_CENTER][SORTER_MOTOR_STATE_RIGHT] = SORTER_MOTOR_STATE_STOP_WAIT;


    SorterAction[TARGET_RIGHT][SORTER_SENSOR_STATE_UNKNOWN][SORTER_MOTOR_STATE_STOP] = SORTER_MOTOR_STATE_RIGHT;
    SorterAction[TARGET_RIGHT][SORTER_SENSOR_STATE_UNKNOWN][SORTER_MOTOR_STATE_STOP_WAIT] = SORTER_MOTOR_STATE_STOP_WAIT;
    SorterAction[TARGET_RIGHT][SORTER_SENSOR_STATE_UNKNOWN][SORTER_MOTOR_STATE_LEFT] = SORTER_MOTOR_STATE_STOP_WAIT;
    SorterAction[TARGET_RIGHT][SORTER_SENSOR_STATE_UNKNOWN][SORTER_MOTOR_STATE_RIGHT] = SORTER_MOTOR_STATE_RIGHT;

    SorterAction[TARGET_RIGHT][SORTER_SENSOR_STATE_LEFT][SORTER_MOTOR_STATE_STOP] = SORTER_MOTOR_STATE_RIGHT;
    SorterAction[TARGET_RIGHT][SORTER_SENSOR_STATE_LEFT][SORTER_MOTOR_STATE_STOP_WAIT] = SORTER_MOTOR_STATE_STOP_WAIT;
    SorterAction[TARGET_RIGHT][SORTER_SENSOR_STATE_LEFT][SORTER_MOTOR_STATE_LEFT] = SORTER_MOTOR_STATE_STOP_WAIT;
    SorterAction[TARGET_RIGHT][SORTER_SENSOR_STATE_LEFT][SORTER_MOTOR_STATE_RIGHT] = SORTER_MOTOR_STATE_RIGHT;

    SorterAction[TARGET_RIGHT][SORTER_SENSOR_STATE_RIGHT][SORTER_MOTOR_STATE_STOP] = SORTER_MOTOR_STATE_STOP;
    SorterAction[TARGET_RIGHT][SORTER_SENSOR_STATE_RIGHT][SORTER_MOTOR_STATE_STOP_WAIT] = SORTER_MOTOR_STATE_STOP_WAIT;
    SorterAction[TARGET_RIGHT][SORTER_SENSOR_STATE_RIGHT][SORTER_MOTOR_STATE_LEFT] = SORTER_MOTOR_STATE_STOP_WAIT;
    SorterAction[TARGET_RIGHT][SORTER_SENSOR_STATE_RIGHT][SORTER_MOTOR_STATE_RIGHT] = SORTER_MOTOR_STATE_STOP_WAIT;

    SorterAction[TARGET_RIGHT][SORTER_SENSOR_STATE_CENTER][SORTER_MOTOR_STATE_STOP] = SORTER_MOTOR_STATE_RIGHT;
    SorterAction[TARGET_RIGHT][SORTER_SENSOR_STATE_CENTER][SORTER_MOTOR_STATE_STOP_WAIT] = SORTER_MOTOR_STATE_STOP_WAIT;
    SorterAction[TARGET_RIGHT][SORTER_SENSOR_STATE_CENTER][SORTER_MOTOR_STATE_LEFT] = SORTER_MOTOR_STATE_STOP_WAIT;
    SorterAction[TARGET_RIGHT][SORTER_SENSOR_STATE_CENTER][SORTER_MOTOR_STATE_RIGHT] = SORTER_MOTOR_STATE_RIGHT;


    SorterAction[TARGET_CENTER][SORTER_SENSOR_STATE_UNKNOWN][SORTER_MOTOR_STATE_STOP] = SORTER_MOTOR_STATE_LEFT;
    SorterAction[TARGET_CENTER][SORTER_SENSOR_STATE_UNKNOWN][SORTER_MOTOR_STATE_STOP_WAIT] = SORTER_MOTOR_STATE_STOP_WAIT;
    SorterAction[TARGET_CENTER][SORTER_SENSOR_STATE_UNKNOWN][SORTER_MOTOR_STATE_LEFT] = SORTER_MOTOR_STATE_LEFT;
    SorterAction[TARGET_CENTER][SORTER_SENSOR_STATE_UNKNOWN][SORTER_MOTOR_STATE_RIGHT] = SORTER_MOTOR_STATE_RIGHT;

    SorterAction[TARGET_CENTER][SORTER_SENSOR_STATE_LEFT][SORTER_MOTOR_STATE_STOP] = SORTER_MOTOR_STATE_RIGHT;
    SorterAction[TARGET_CENTER][SORTER_SENSOR_STATE_LEFT][SORTER_MOTOR_STATE_STOP_WAIT] = SORTER_MOTOR_STATE_STOP_WAIT;
    SorterAction[TARGET_CENTER][SORTER_SENSOR_STATE_LEFT][SORTER_MOTOR_STATE_LEFT] = SORTER_MOTOR_STATE_STOP_WAIT;
    SorterAction[TARGET_CENTER][SORTER_SENSOR_STATE_LEFT][SORTER_MOTOR_STATE_RIGHT] = SORTER_MOTOR_STATE_RIGHT;

    SorterAction[TARGET_CENTER][SORTER_SENSOR_STATE_RIGHT][SORTER_MOTOR_STATE_STOP] = SORTER_MOTOR_STATE_LEFT;
    SorterAction[TARGET_CENTER][SORTER_SENSOR_STATE_RIGHT][SORTER_MOTOR_STATE_STOP_WAIT] = SORTER_MOTOR_STATE_STOP_WAIT;
    SorterAction[TARGET_CENTER][SORTER_SENSOR_STATE_RIGHT][SORTER_MOTOR_STATE_LEFT] = SORTER_MOTOR_STATE_LEFT;
    SorterAction[TARGET_CENTER][SORTER_SENSOR_STATE_RIGHT][SORTER_MOTOR_STATE_RIGHT] = SORTER_MOTOR_STATE_STOP_WAIT;

    SorterAction[TARGET_CENTER][SORTER_SENSOR_STATE_CENTER][SORTER_MOTOR_STATE_STOP] = SORTER_MOTOR_STATE_STOP;
    SorterAction[TARGET_CENTER][SORTER_SENSOR_STATE_CENTER][SORTER_MOTOR_STATE_STOP_WAIT] = SORTER_MOTOR_STATE_STOP_WAIT;
    SorterAction[TARGET_CENTER][SORTER_SENSOR_STATE_CENTER][SORTER_MOTOR_STATE_LEFT] = SORTER_MOTOR_STATE_STOP_WAIT;
    SorterAction[TARGET_CENTER][SORTER_SENSOR_STATE_CENTER][SORTER_MOTOR_STATE_RIGHT] = SORTER_MOTOR_STATE_STOP_WAIT;

    //pcieIO_init();

}

pcieIO::~pcieIO()
{

    //savePreviousSensorState();

}

void pcieIO::pcieIO_init()
{
    instantDiCtrl = AdxInstantDiCtrlCreate();
    instantDoCtrl = AdxInstantDoCtrlCreate();

    ICollection<DeviceTreeNode> *DIsupportedDevices = instantDiCtrl -> getSupportedDevices();
    ICollection<DeviceTreeNode> *DOsupportedDevices = instantDoCtrl -> getSupportedDevices();

    // DI initialize//
    if (DIsupportedDevices->getCount() == 0)
    {
        qDebug() << "DI supported Device Error";
    }
    else
    {
        DeviceTreeNode const &node = DIsupportedDevices->getItem(0);
        std::wstring description = (QString::fromWCharArray(node.Description)).toStdWString();
        qDebug() << QString::fromWCharArray(node.Description);

        DeviceInformation selected(description.c_str());

        ErrorCode errorCode = instantDiCtrl->setSelectedDevice(selected);
        if (errorCode != Success)
        {
            QString str;
            QString des = QString::fromStdWString(description);
            str.sprintf("Error:the error code is 0x%x\nThe %s is busy or not exit in computer now.\nSelect other device please!", errorCode, des.toUtf8().data());
            qDebug() << "DI supported Device Error:";
            qDebug() << errorCode;
            return;
        }

        configure.deviceName = QString::fromWCharArray(node.Description);
    }

    // DO initialize//
    if (DOsupportedDevices->getCount() == 0)
    {
        qDebug() << "DO supported Device Error";
    }
    else
    {
        DeviceTreeNode const &node = DOsupportedDevices->getItem(0);
        std::wstring description = (QString::fromWCharArray(node.Description)).toStdWString();
        qDebug() << QString::fromWCharArray(node.Description);

        DeviceInformation selected(description.c_str());

        ErrorCode errorCode = instantDoCtrl->setSelectedDevice(selected);
        if (errorCode != Success)
        {
            QString str;
            QString des = QString::fromStdWString(description);
            str.sprintf("Error:the error code is 0x%x\nThe %s is busy or not exit in computer now.\nSelect other device please!", errorCode, des.toUtf8().data());
            qDebug() << "DI supported Device Error:";
            qDebug() << errorCode;
            return;
        }

        configure.deviceName = QString::fromWCharArray(node.Description);
    }



    m_selfTestItem = SELF_TEST_NONE;
    m_doorOpenState = DOOR_STATE_CLOSE;

    portCount = instantDiCtrl->getPortCount();

    m_Sorter_target = TARGET_CENTER;
    m_Sorter_sensor_state = SORTER_SENSOR_STATE_UNKNOWN;
    m_Sorter_motor_state = SORTER_MOTOR_STATE_STOP;



    m_RecyclingStart = false;
    portDO_States = new quint8[portCount];
    for(int i=0; i<portCount; i++)
        portDO_States[i] =0;

    m_crusherMotorOutput = DO0_CRUSHER_MOTOR_OFF+DO1_CRUSHER_MOTOR_FWD;
    m_conveyorMotorOutput = DO2_CONVEYOR_MOTOR_CW_PWM_L+DO3_CONVEYOR_MOTOR_CCW_PWM_L;
    m_sorterMotorOutput = DO4_SORTER_MOTOR_FWD_OFF + DO5_SORTER_MOTOR_REV_OFF;
    m_ledOutput = DO6_LED_LIGHT_OFF;

    portDO_States[0] = m_crusherMotorOutput + m_conveyorMotorOutput + m_sorterMotorOutput + m_ledOutput;

    timer_DI = new QTimer(this);
    connect(timer_DI, SIGNAL(timeout()), this, SLOT(updateSignal_in()));
    timer_DI->start(10);

    m_Sorter_target = TARGET_CENTER;
    m_Sorter_target_before = TARGET_CENTER;

    m_seq = SEQUENCE_INIT;
    Cam_Classify_wait = CAM_CLS_NO;
    Cam_Classified_object = CAM_CLS_UNKNOWN;

    ErrorCode errorCode = Success;
    errorCode = instantDoCtrl->Write(0, portCount, portDO_States);


    g_sorterClose = false;
    g_sorterNotCloseCount = 0;

    g_crusherStopCount=1000;     // (17.08.21) YC add to delay crusher stopping

    QTimer::singleShot(10, this, SLOT(updateSignal_out()));

}

void pcieIO::updateSignal_in()
{
    static int count=0;
    quint8 *portStates = new quint8[portCount];
    ErrorCode errorCode = ErrorFuncBusy;

    while(errorCode != Success)
        errorCode = instantDiCtrl->Read(0, portCount, portStates);



    m_Crusher_sensor = portStates[0] & DI0_CRUSHER_SENSOR;
    m_Conveyor_in_sensor = portStates[0] & DI2_CONVEYOR_IN_SENSOR;
    m_Conveyor_out_sensor = portStates[0] & DI3_CONVEYOR_OUT_SENSOR ;
    m_Sorter_R_sensor = portStates[0] & DI4_SORTER_R_SENSOR;
    m_Sorter_L_sensor = portStates[0] & DI5_SORTER_L_SENSOR;
    m_Door_top_sensor = portStates[0] & DI6_DOOR_TOP_SENSOR;
    m_Door_bottom_sensor = portStates[0] & DI7_DOOR_BOTTOM_SENSOR;

    m_Pet_full_sensor = portStates[1] & DI8_PET_BOX_FULL_SENSOR;
    m_Pet_80_sensor = portStates[1] & DI9_PET_BOX_80_SENSOR;
    m_Pet_no_sensor = portStates[1] & DI10_PET_BOX_NO_SENSOR;
    m_Can_full_sensor = portStates[1] & DI11_CAN_BOX_FULL_SENSOR;
    m_Can_80_sensor = portStates[1] & DI12_CAN_BOX_80_SENSOR;
    m_Can_no_sensor = portStates[1] & DI13_CAN_BOX_NO_SENSOR;

    if(m_Sorter_R_sensor)
    {
        if(m_Sorter_L_sensor)
            m_Sorter_sensor_state = SORTER_SENSOR_STATE_CENTER;
        else
            m_Sorter_sensor_state = SORTER_SENSOR_STATE_RIGHT;
    }
    else
    {
        if(m_Sorter_L_sensor)
            m_Sorter_sensor_state = SORTER_SENSOR_STATE_LEFT;
        else
            m_Sorter_sensor_state = SORTER_SENSOR_STATE_UNKNOWN;
    }

    if(m_doorOpenState==DOOR_STATE_CLOSE)
    {
        if((m_Door_top_sensor==1)||(m_Door_bottom_sensor==1))
        {
           char state=0;
            if(m_Door_top_sensor==1)
            {
                qDebug()<<"top open";
                state+=1;
            }
            if(m_Door_bottom_sensor==1)
            {
                qDebug()<<"bottom open";
                state+=2;
            }
            emit plcErrorDetect(PS_DOOROPEN, state);
            m_doorOpenState = DOOR_STATE_OPEN;
        }
    }
    else if(m_doorOpenState == DOOR_STATE_OPEN)
    {
        if((m_Door_top_sensor==0)&&(m_Door_bottom_sensor==0))
        {
            m_doorOpenState = DOOR_STATE_CLOSE;
            emit plcErrorDetect(PS_DOOROPEN, 0);
        }
    }
}

void pcieIO::sorterCtrl()
{
    static int sorterCount=0;
    static int sorterMotorPwmCount=0;
    static int sorterMotorDefaultRunTime=0;



    if(m_Sorter_motor_state == SORTER_MOTOR_STATE_STOP_WAIT)
    {
        QThread::msleep(1000);
        m_Sorter_motor_state = SORTER_MOTOR_STATE_STOP;
    }

    if((m_Sorter_motor_state == SORTER_MOTOR_STATE_LEFT) || (m_Sorter_motor_state == SORTER_MOTOR_STATE_RIGHT))
        m_Sorter_motor_state = SorterAction[m_Sorter_target][m_Sorter_sensor_state][m_Sorter_motor_state];
    else
        m_Sorter_motor_state = SorterAction[m_Sorter_target][m_Sorter_sensor_state][m_Sorter_motor_state];
}

void pcieIO::SorterOpen()
{
    ErrorCode errorCode = Success;
    qint64 nanoSec;
    qint64 loop;

    qDebug("Soter Open");
    if(m_Sorter_target == TARGET_RIGHT)
    {
        m_sorterMotorOutput = DO4_SORTER_MOTOR_FWD_ON + DO5_SORTER_MOTOR_REV_OFF;
        portDO_States[0] = m_crusherMotorOutput + m_conveyorMotorOutput + m_sorterMotorOutput + m_ledOutput;
        errorCode = instantDoCtrl->Write(0, portCount, portDO_States);

        for(loop=0; loop<4000; loop++)
        {
            for(int k=0;k<15;k++)
            {
                for(int j=0;j<5000;j++)
                    nanoSec = k*j;
            }
        }

        for(loop=0; loop<60000; loop++)
        {
            if(m_Sorter_sensor_state == SORTER_SENSOR_STATE_RIGHT)
                break;
            else
            {
                m_sorterMotorOutput = DO4_SORTER_MOTOR_FWD_OFF + DO5_SORTER_MOTOR_REV_OFF;
                portDO_States[0] = m_crusherMotorOutput + m_conveyorMotorOutput + m_sorterMotorOutput + m_ledOutput;
                errorCode = instantDoCtrl->Write(0, portCount, portDO_States);

                for(int k=0;k<90;k++)
                {
                    for(int j=0;j<5000;j++)
                        nanoSec = k*j;
                }

                m_sorterMotorOutput = DO4_SORTER_MOTOR_FWD_ON + DO5_SORTER_MOTOR_REV_OFF;
                portDO_States[0] = m_crusherMotorOutput + m_conveyorMotorOutput + m_sorterMotorOutput + m_ledOutput;
                errorCode = instantDoCtrl->Write(0, portCount, portDO_States);

                for(int k=0;k<20;k++)
                {
                    for(int j=0;j<5000;j++)
                        nanoSec = k*j;
                }
                updateSignal_in();
            }
        }
    }

    else if(m_Sorter_target == TARGET_LEFT)
    {
        m_sorterMotorOutput = DO4_SORTER_MOTOR_FWD_OFF + DO5_SORTER_MOTOR_REV_ON;
        portDO_States[0] = m_crusherMotorOutput + m_conveyorMotorOutput + m_sorterMotorOutput + m_ledOutput;
        errorCode = instantDoCtrl->Write(0, portCount, portDO_States);

        for(loop=0; loop<4000; loop++)
        {
            for(int k=0;k<15;k++)
            {
                for(int j=0;j<5000;j++)
                    nanoSec = k*j;
            }
        }

        for(loop=0; loop<60000; loop++)
        {
            if(m_Sorter_sensor_state == SORTER_SENSOR_STATE_LEFT)
                break;
            else
            {
                m_sorterMotorOutput = DO4_SORTER_MOTOR_FWD_OFF + DO5_SORTER_MOTOR_REV_OFF;
                portDO_States[0] = m_crusherMotorOutput + m_conveyorMotorOutput + m_sorterMotorOutput + m_ledOutput;
                errorCode = instantDoCtrl->Write(0, portCount, portDO_States);

                for(int k=0;k<90;k++)
                {
                    for(int j=0;j<5000;j++)
                        nanoSec = k*j;
                }

                m_sorterMotorOutput = DO4_SORTER_MOTOR_FWD_OFF + DO5_SORTER_MOTOR_REV_ON;
                portDO_States[0] = m_crusherMotorOutput + m_conveyorMotorOutput + m_sorterMotorOutput + m_ledOutput;
                errorCode = instantDoCtrl->Write(0, portCount, portDO_States);

                for(int k=0;k<20;k++)
                {
                    for(int j=0;j<5000;j++)
                        nanoSec = k*j;
                }
                updateSignal_in();
            }
        }

    }

    if(loop>20000)
        m_seq = SEQUENCE_SORTER_JAM;

    m_sorterMotorOutput = DO4_SORTER_MOTOR_FWD_OFF + DO5_SORTER_MOTOR_REV_OFF;
    portDO_States[0] = m_crusherMotorOutput + m_conveyorMotorOutput + m_sorterMotorOutput + m_ledOutput;
    errorCode = instantDoCtrl->Write(0, portCount, portDO_States);
}

void pcieIO::SorterClose()
{
    ErrorCode errorCode = Success;
    qint64 nanoSec;
    uint64 loop;
    long acc=45000;
    bool returnVal;

    m_Sorter_target = TARGET_CENTER;
    qDebug("sorter close");
    for(loop=0; loop<120; loop++)
    {
        sorterCtrl();
        if(acc>1000)
            acc-=4000;
        else if(acc>-350)
            acc-=20;
        else if(acc>-45000)
            acc-=4000;

        if(m_Sorter_sensor_state == SORTER_SENSOR_STATE_CENTER)
        {
            g_sorterClose=true;
            g_sorterNotCloseCount=0;
            break;
        }
        else
        {
            if(m_Sorter_motor_state == SORTER_MOTOR_STATE_LEFT)
            {
                m_sorterMotorOutput = DO4_SORTER_MOTOR_FWD_OFF + DO5_SORTER_MOTOR_REV_ON;
                portDO_States[0] = m_crusherMotorOutput + m_conveyorMotorOutput + m_sorterMotorOutput + m_ledOutput;
                errorCode = ErrorFuncBusy;
                while(errorCode != Success)
                    errorCode = instantDoCtrl->Write(0, portCount, portDO_States);
                QThread::usleep(2100);

                m_sorterMotorOutput = DO4_SORTER_MOTOR_FWD_OFF + DO5_SORTER_MOTOR_REV_OFF;
                portDO_States[0] = m_crusherMotorOutput + m_conveyorMotorOutput + m_sorterMotorOutput + m_ledOutput;
                errorCode = ErrorFuncBusy;
                while(errorCode != Success)
                    errorCode = instantDoCtrl->Write(0, portCount, portDO_States);
                QThread::usleep(labs(acc)+2000);
            }
            else if(m_Sorter_motor_state == SORTER_MOTOR_STATE_RIGHT)
            {
                m_sorterMotorOutput = DO4_SORTER_MOTOR_FWD_ON + DO5_SORTER_MOTOR_REV_OFF;
                portDO_States[0] = m_crusherMotorOutput + m_conveyorMotorOutput + m_sorterMotorOutput + m_ledOutput;
                errorCode = ErrorFuncBusy;
                while(errorCode != Success)
                    errorCode = instantDoCtrl->Write(0, portCount, portDO_States);
                QThread::usleep(2000);

                m_sorterMotorOutput = DO4_SORTER_MOTOR_FWD_OFF + DO5_SORTER_MOTOR_REV_OFF;
                portDO_States[0] = m_crusherMotorOutput + m_conveyorMotorOutput + m_sorterMotorOutput + m_ledOutput;
                errorCode = ErrorFuncBusy;
                while(errorCode != Success)
                    errorCode = instantDoCtrl->Write(0, portCount, portDO_States);
                QThread::usleep(labs(acc)+2000);
            }
            g_sorterClose=false;
            if(g_sorterNotCloseCount<3)
                g_sorterNotCloseCount++;
        }
        updateSignal_in();
    }
    qDebug("for end");
    m_sorterMotorOutput = DO4_SORTER_MOTOR_FWD_OFF + DO5_SORTER_MOTOR_REV_OFF;
    portDO_States[0] = m_crusherMotorOutput + m_conveyorMotorOutput + m_sorterMotorOutput + m_ledOutput;
    errorCode = ErrorFuncBusy;
    while(errorCode != Success)
        errorCode = instantDoCtrl->Write(0, portCount, portDO_States);
}

void pcieIO::ConveyorRun(int clock)
{

    ErrorCode errorCode = Success;
    qint64 nanoSec;

    for(int loop=0; loop<clock; loop++)
    {
        if((loop%2)==0)
            m_conveyorMotorOutput = DO2_CONVEYOR_MOTOR_CW_PWM_H + DO3_CONVEYOR_MOTOR_CCW_PWM_L;
        else
            m_conveyorMotorOutput = DO2_CONVEYOR_MOTOR_CW_PWM_L + DO3_CONVEYOR_MOTOR_CCW_PWM_L;
        portDO_States[0] = m_crusherMotorOutput + m_conveyorMotorOutput + m_sorterMotorOutput + m_ledOutput;
        errorCode = instantDoCtrl->Write(0, portCount, portDO_States);

        for(int k=0;k<15;k++)
        {
            for(int j=0;j<5000;j++)
                nanoSec = k*j;
        }
    }
}

void pcieIO::ConvIn()
{

    ErrorCode errorCode = Success;
    QThread qThread;
    qint64 nanoSec;

    m_seq = SEQUENCE_INPUT_WAIT;

    for(int loop=0; loop<15000; loop++)
    {

        if(m_Conveyor_out_sensor==false)
        {
            m_conveyorMotorOutput = DO2_CONVEYOR_MOTOR_CW_PWM_L + DO3_CONVEYOR_MOTOR_CCW_PWM_L;
            portDO_States[0] = m_crusherMotorOutput + m_conveyorMotorOutput + m_sorterMotorOutput + m_ledOutput;
            errorCode = instantDoCtrl->Write(0, portCount, portDO_States);
            qDebug()<<"out";
            m_seq = SEQUENCE_CLASSIFY;

            emit plcStateChage(PS_MOVINGCLSPOINT);
            break;
        }
        else
        {
            if((loop%2)==0)
                m_conveyorMotorOutput = DO2_CONVEYOR_MOTOR_CW_PWM_H + DO3_CONVEYOR_MOTOR_CCW_PWM_L;
            else
                m_conveyorMotorOutput = DO2_CONVEYOR_MOTOR_CW_PWM_L + DO3_CONVEYOR_MOTOR_CCW_PWM_L;
            portDO_States[0] = m_crusherMotorOutput + m_conveyorMotorOutput + m_sorterMotorOutput + m_ledOutput;
            errorCode = instantDoCtrl->Write(0, portCount, portDO_States);
        }
        for(int k=0;k<15;k++)
        {
            for(int j=0;j<5000;j++)
                nanoSec = k*j;
        }

        if(loop%50==0)
            updateSignal_in();
    }
    Cam_Classified_object = CAM_CLS_UNKNOWN;
    Cam_Classify_wait = CAM_CLS_REQ;
    m_conveyorMotorOutput = DO2_CONVEYOR_MOTOR_CW_PWM_L + DO3_CONVEYOR_MOTOR_CCW_PWM_L;
}

void pcieIO::ConvInTest()
{
    ErrorCode errorCode = Success;
    QThread qThread;
    qint64 nanoSec;

    for(int loop=0; loop<15000; loop++)
    {

        if(m_Conveyor_out_sensor==false)
        {
            m_conveyorMotorOutput = DO2_CONVEYOR_MOTOR_CW_PWM_L + DO3_CONVEYOR_MOTOR_CCW_PWM_L;
            portDO_States[0] = m_crusherMotorOutput + m_conveyorMotorOutput + m_sorterMotorOutput + m_ledOutput;
            errorCode = instantDoCtrl->Write(0, portCount, portDO_States);
            qDebug()<<"out";
            m_seq = SEQUENCE_CLASSIFY;
            emit plcStateChage(PS_MOVINGCLSPOINT);
            break;
        }
        else
        {
            if((loop%2)==0)
                m_conveyorMotorOutput = DO2_CONVEYOR_MOTOR_CW_PWM_H + DO3_CONVEYOR_MOTOR_CCW_PWM_L;
            else
                m_conveyorMotorOutput = DO2_CONVEYOR_MOTOR_CW_PWM_L + DO3_CONVEYOR_MOTOR_CCW_PWM_L;
            portDO_States[0] = m_crusherMotorOutput + m_conveyorMotorOutput + m_sorterMotorOutput + m_ledOutput;
            errorCode = instantDoCtrl->Write(0, portCount, portDO_States);
        }
        for(int k=0;k<15;k++)
        {
            for(int j=0;j<5000;j++)
                nanoSec = k*j;
        }

        if(loop%50==0)
            updateSignal_in();
    }
    Cam_Classified_object = CAM_CLS_UNKNOWN;
    Cam_Classify_wait = CAM_CLS_REQ;
    m_conveyorMotorOutput = DO2_CONVEYOR_MOTOR_CW_PWM_L + DO3_CONVEYOR_MOTOR_CCW_PWM_L;
}

void pcieIO::ConvOut()
{
    ErrorCode errorCode = Success;
    QThread qThread;
    qint64 nanoSec;

    m_seq = SEQUENCE_INPUT_WAIT;
    for(int loop=0; loop<15000; loop++)
    {

        if((loop%2)==0)
            m_conveyorMotorOutput = DO2_CONVEYOR_MOTOR_CW_PWM_L + DO3_CONVEYOR_MOTOR_CCW_PWM_L;
        else
            m_conveyorMotorOutput = DO2_CONVEYOR_MOTOR_CW_PWM_L + DO3_CONVEYOR_MOTOR_CCW_PWM_H;
        portDO_States[0] = m_crusherMotorOutput + m_conveyorMotorOutput + m_sorterMotorOutput + m_ledOutput;
        errorCode = instantDoCtrl->Write(0, portCount, portDO_States);
        for(int k=0;k<15;k++)
        {
            for(int j=0;j<5000;j++)
                nanoSec = k*j;
        }
    }
    m_conveyorMotorOutput = DO2_CONVEYOR_MOTOR_CW_PWM_L + DO3_CONVEYOR_MOTOR_CCW_PWM_L;
}

void pcieIO::ConvOutTest()
{
    ErrorCode errorCode = Success;
    QThread qThread;
    qint64 nanoSec;

    for(int loop=0; loop<15000; loop++)
    {

        if((loop%2)==0)
            m_conveyorMotorOutput = DO2_CONVEYOR_MOTOR_CW_PWM_L + DO3_CONVEYOR_MOTOR_CCW_PWM_L;
        else
            m_conveyorMotorOutput = DO2_CONVEYOR_MOTOR_CW_PWM_L + DO3_CONVEYOR_MOTOR_CCW_PWM_H;
        portDO_States[0] = m_crusherMotorOutput + m_conveyorMotorOutput + m_sorterMotorOutput + m_ledOutput;
        errorCode = instantDoCtrl->Write(0, portCount, portDO_States);
        for(int k=0;k<15;k++)
        {
            for(int j=0;j<5000;j++)
                nanoSec = k*j;
        }
    }
    m_conveyorMotorOutput = DO2_CONVEYOR_MOTOR_CW_PWM_L + DO3_CONVEYOR_MOTOR_CCW_PWM_L;
}

bool pcieIO::WaitForClassfy()
{
    if(Cam_Classify_wait == CAM_CLS_DONE)
    {
        if(Cam_Classified_object == CAM_CLS_PET)
        {
            m_Sorter_target = TARGET_RIGHT;
            m_Sorter_target_before = TARGET_RIGHT;
        }
        else if(Cam_Classified_object == CAM_CLS_CAN)
        {
            m_Sorter_target = TARGET_LEFT;
            m_Sorter_target_before = TARGET_LEFT;
        }
        else
        {
            m_Sorter_target = TARGET_CENTER;
            m_Sorter_target_before = TARGET_CENTER;
            emit plcStateChage(PS_RETURNOBJECT);
        }
        return true;
    }
    return false;
}

void pcieIO::CrusherIn()
{

    ErrorCode errorCode = Success;
    QThread qThread;
    qint64 nanoSec;


    m_crusherMotorOutput = DO0_CRUSHER_MOTOR_ON+DO1_CRUSHER_MOTOR_FWD;

    for(int loop=0; loop<2000; loop++)
    {
        if((loop%2)==0)
            m_conveyorMotorOutput = DO2_CONVEYOR_MOTOR_CW_PWM_H + DO3_CONVEYOR_MOTOR_CCW_PWM_L;
        else
            m_conveyorMotorOutput = DO2_CONVEYOR_MOTOR_CW_PWM_L + DO3_CONVEYOR_MOTOR_CCW_PWM_L;
        portDO_States[0] = m_crusherMotorOutput + m_conveyorMotorOutput + m_sorterMotorOutput + m_ledOutput;
        errorCode = instantDoCtrl->Write(0, portCount, portDO_States);

        for(int k=0;k<15;k++)
        {
            for(int j=0;j<5000;j++)
                nanoSec = k*j;
        }

        if(loop%50==0)
            updateSignal_in();
    }

    for(int loop=0; loop<15000; loop++)
    {

        if(m_Conveyor_out_sensor==true)
        {
            m_conveyorMotorOutput = DO2_CONVEYOR_MOTOR_CW_PWM_L + DO3_CONVEYOR_MOTOR_CCW_PWM_L;
            portDO_States[0] = m_crusherMotorOutput + m_conveyorMotorOutput + m_sorterMotorOutput + m_ledOutput;
            errorCode = instantDoCtrl->Write(0, portCount, portDO_States);
            qDebug()<<"out";
            m_seq = SEQUENCE_SORTER_CLOSE;
            break;
        }
        else
        {
            if((loop%2)==0)
                m_conveyorMotorOutput = DO2_CONVEYOR_MOTOR_CW_PWM_H + DO3_CONVEYOR_MOTOR_CCW_PWM_L;
            else
                m_conveyorMotorOutput = DO2_CONVEYOR_MOTOR_CW_PWM_L + DO3_CONVEYOR_MOTOR_CCW_PWM_L;
            portDO_States[0] = m_crusherMotorOutput + m_conveyorMotorOutput + m_sorterMotorOutput + m_ledOutput;
            errorCode = instantDoCtrl->Write(0, portCount, portDO_States);
        }
        for(int k=0;k<15;k++)
        {
            for(int j=0;j<5000;j++)
                nanoSec = k*j;
        }

        if(loop%50==0)
            updateSignal_in();
    }
    m_crusherMotorOutput = DO0_CRUSHER_MOTOR_ON+DO1_CRUSHER_MOTOR_FWD;
    m_conveyorMotorOutput = DO2_CONVEYOR_MOTOR_CW_PWM_L + DO3_CONVEYOR_MOTOR_CCW_PWM_L;
    portDO_States[0] = m_crusherMotorOutput + m_conveyorMotorOutput + m_sorterMotorOutput + m_ledOutput;
    errorCode = instantDoCtrl->Write(0, portCount, portDO_States);
}

void pcieIO::SensorCheck()
{
    if(m_bConveyor_in_sensor!=m_Conveyor_in_sensor)
    {
        m_bConveyor_in_sensor=m_Conveyor_in_sensor;
        emit sensorStateChange(PLC_STARTIR, !m_bConveyor_in_sensor);
    }

    if(m_bConveyor_out_sensor!=m_Conveyor_out_sensor)
    {
        m_bConveyor_out_sensor=m_Conveyor_out_sensor;
        emit sensorStateChange(PLC_ENDIR, !m_bConveyor_out_sensor);
    }

    if(m_bDoor_top_sensor!=m_Door_top_sensor)
    {
        m_bDoor_top_sensor=m_Door_top_sensor;
        emit sensorStateChange(DOOR_TOP_S, !m_Door_top_sensor);
    }

    if(m_bDoor_bottom_sensor!=m_Door_bottom_sensor)
    {
        m_bDoor_bottom_sensor=m_Door_bottom_sensor;
        emit sensorStateChange(DOOR_BOTTOM_S, !m_Door_bottom_sensor);
    }

    if(m_bPet_full_sensor!=m_Pet_full_sensor)
    {
        m_bPet_full_sensor=m_Pet_full_sensor;
        emit sensorStateChange(PLC_PET_FULL_SENSOR, !m_Pet_full_sensor);
    }

    if(m_bPet_80_sensor!=m_Pet_80_sensor)
    {
        m_bPet_80_sensor=m_Pet_80_sensor;
        emit sensorStateChange(PLC_PET_80_SENSOR, !m_Pet_80_sensor);
    }

    if(m_bPet_no_sensor!=m_Pet_no_sensor)
    {
        m_bPet_no_sensor=m_Pet_no_sensor;
        emit sensorStateChange(PLC_PET_TRAY_SENSOR, !m_Pet_no_sensor);
    }

    if(m_bCan_full_sensor!=m_Can_full_sensor)
    {
        m_bCan_full_sensor=m_Can_full_sensor;
        emit sensorStateChange(PLC_CAN_FULL_SENSOR, !m_Can_full_sensor);
    }

    if(m_bCan_80_sensor!=m_Can_80_sensor)
    {
        m_bCan_80_sensor=m_Can_80_sensor;
        emit sensorStateChange(PLC_CAN_80_SENSOR, !m_Can_80_sensor);
    }

    if(m_bCan_no_sensor!=m_Can_no_sensor)
    {
        m_bCan_no_sensor=m_Can_no_sensor;
        emit sensorStateChange(PLC_CAN_TRAY_SENSOR, !m_Can_no_sensor);
    }

    if(m_Sorter_sensor_state==SORTER_SENSOR_STATE_CENTER)
        emit sensorStateChange(PLC_SORTER_CENTER, 1);
    else if(m_Sorter_sensor_state==SORTER_SENSOR_STATE_LEFT)
        emit sensorStateChange(PLC_SORTER_LEFT, 1);
    else if(m_Sorter_sensor_state==SORTER_SENSOR_STATE_RIGHT)
        emit sensorStateChange(PLC_SORTER_RIGHT, 1);
}

void pcieIO::updateSignal_out()
{

    ErrorCode errorCode = Success;
    int repeat=0;
    qint64 nanoSec;

    updateSignal_in();
    if((m_RecyclingStart==false)&&(m_seq!=SEQUENCE_SELFTEST))
        m_seq = SEQUENCE_INIT;

    switch(m_seq)
    {
    case SEQUENCE_INIT:
        m_ledOutput = DO6_LED_LIGHT_OFF;

        //-------- (17.08.21) YC add to delay crusher stopping -------------------//
        if(g_crusherStopCount<1000)
        {
            m_crusherMotorOutput = DO0_CRUSHER_MOTOR_ON+DO1_CRUSHER_MOTOR_FWD;
            g_crusherStopCount++;
            QThread::msleep(10); // 10ms * 1000 = 10s
        }
        else
            m_crusherMotorOutput = DO0_CRUSHER_MOTOR_OFF+DO1_CRUSHER_MOTOR_FWD;
        //------------------------------------------------------------------------//

        portDO_States[0] = m_crusherMotorOutput + m_conveyorMotorOutput + m_sorterMotorOutput + m_ledOutput;
        errorCode = instantDoCtrl->Write(0, portCount, portDO_States);

        if((g_sorterClose==false)&&(g_sorterNotCloseCount<3))
            SorterClose();

        if(m_RecyclingStart)
            m_seq = SEQUENCE_INPUT_WAIT;
        break;
    case SEQUENCE_INPUT_WAIT:

        m_ledOutput = DO6_LED_LIGHT_ON;
        portDO_States[0] = m_crusherMotorOutput + m_conveyorMotorOutput + m_sorterMotorOutput + m_ledOutput;
        errorCode = instantDoCtrl->Write(0, portCount, portDO_States);

        if(m_Conveyor_in_sensor==false)
            m_seq = SEQUENCE_INPUT;
        break;
    case SEQUENCE_INPUT:
        ConvIn();
        break;
    case SEQUENCE_CLASSIFY:
        if(WaitForClassfy())
        {
            if((Cam_Classified_object == CAM_CLS_PET)||(Cam_Classified_object==CAM_CLS_CAN))
                m_seq = SEQUENCE_SORTER_OPEN;
            else
            {
                ConvOut();
                m_seq = SEQUENCE_INPUT_WAIT;
            }
        }
        break;
    case SEQUENCE_SORTER_OPEN:
        SorterOpen();
        emit plcStateChage(PS_GETREADY);
        m_seq = SEQUENCE_INPUT_CRUSHER;
        break;
    case SEQUENCE_INPUT_CRUSHER:
        CrusherIn();
         m_seq = SEQUENCE_SORTER_CLOSE;
        break;
    case SEQUENCE_SORTER_CLOSE:
        for(repeat=0; repeat<SORTER_CLOSE_REPEAT; repeat++)
        {
            SorterClose();
            updateSignal_in();
            if(m_Sorter_sensor_state == SORTER_SENSOR_STATE_CENTER)
                break;
            else
            {
                m_Sorter_target = m_Sorter_target_before;
                SorterOpen();
                ConveyorRun(7000);
            }
        }
        if(repeat==SORTER_CLOSE_REPEAT)
        {
            m_seq = SEQUENCE_SORTER_JAM;
        }
        else
        {
        //   m_crusherMotorOutput = DO0_CRUSHER_MOTOR_OFF+DO1_CRUSHER_MOTOR_FWD;
        //   errorCode = instantDoCtrl->Write(0, portCount, portDO_States);
             m_seq=SEQUENCE_INPUT;
             g_crusherStopCount = 0; // (17.08.21) YC add to delay crusher stopping
             emit plcStateChage(PS_RECYCLINGEND);
        }
        break;
    case SEQUENCE_SORTER_JAM:
        m_crusherMotorOutput = DO0_CRUSHER_MOTOR_OFF+DO1_CRUSHER_MOTOR_FWD;
        m_conveyorMotorOutput = DO2_CONVEYOR_MOTOR_CW_PWM_L+DO3_CONVEYOR_MOTOR_CCW_PWM_L;
        m_sorterMotorOutput = DO4_SORTER_MOTOR_FWD_OFF + DO5_SORTER_MOTOR_REV_OFF;
        m_ledOutput = DO6_LED_LIGHT_ON;

        portDO_States[0] = m_crusherMotorOutput + m_conveyorMotorOutput + m_sorterMotorOutput + m_ledOutput;
        errorCode = instantDoCtrl->Write(0, portCount, portDO_States);
        m_seq = SEQUENCE_SORTER_JAM_REQ_WAIT;
        emit plcErrorDetect(PS_JAM, 1);
        break;
    case SEQUENCE_SORTER_JAM_REQ_WAIT:
        break;
    case SEQUENCE_SELFTEST:
        SensorCheck();
        qDebug()<<"test";
        switch(m_selfTestItem)
        {
        case SELF_TEST_STEPPING_F:
            ConvInTest();
            m_selfTestItem = SELF_TEST_NONE;
            break;
        case SELF_TEST_STEPPING_R:
            ConvOutTest();
            m_selfTestItem = SELF_TEST_NONE;
            break;
        case SELF_TEST_SORTER_L:
            m_Sorter_target = TARGET_LEFT;
            SorterOpen();
            m_selfTestItem = SELF_TEST_NONE;
            break;
        case SELF_TEST_SORTER_R:
            m_Sorter_target = TARGET_RIGHT;
            SorterOpen();
            m_selfTestItem = SELF_TEST_NONE;
            break;
        case SELF_TEST_SORTER_C:
            SorterClose();
            m_selfTestItem = SELF_TEST_NONE;
            break;
        case SELF_TEST_GEARED_F:
            m_crusherMotorOutput = DO0_CRUSHER_MOTOR_ON+DO1_CRUSHER_MOTOR_FWD;
            portDO_States[0] = m_crusherMotorOutput + m_conveyorMotorOutput + m_sorterMotorOutput + m_ledOutput;
            errorCode = instantDoCtrl->Write(0, portCount, portDO_States);
         //   for(int k=0;k<80000;k++)
         //   {
         //       for(int j=0;j<40000;j++)
         //           nanoSec = k*j;
         //   }
         //   m_crusherMotorOutput = DO0_CRUSHER_MOTOR_OFF+DO1_CRUSHER_MOTOR_FWD;
         //   portDO_States[0] = m_crusherMotorOutput + m_conveyorMotorOutput + m_sorterMotorOutput + m_ledOutput;
         //   errorCode = instantDoCtrl->Write(0, portCount, portDO_States);
            m_selfTestItem = SELF_TEST_NONE;
            break;
        case SELF_TEST_GEARED_R:
            m_crusherMotorOutput = DO0_CRUSHER_MOTOR_ON+DO1_CRUSHER_MOTOR_REV;
            portDO_States[0] = m_crusherMotorOutput + m_conveyorMotorOutput + m_sorterMotorOutput + m_ledOutput;
            errorCode = instantDoCtrl->Write(0, portCount, portDO_States);
         //   for(int k=0;k<80000;k++)
         //   {
         //       for(int j=0;j<40000;j++)
         //           nanoSec = k*j;
         //   }
         //   m_crusherMotorOutput = DO0_CRUSHER_MOTOR_OFF+DO1_CRUSHER_MOTOR_REV;
         //   portDO_States[0] = m_crusherMotorOutput + m_conveyorMotorOutput + m_sorterMotorOutput + m_ledOutput;
         //  errorCode = instantDoCtrl->Write(0, portCount, portDO_States);
            m_selfTestItem = SELF_TEST_NONE;
            break;
        case SELF_TEST_GEARED_S:
            m_crusherMotorOutput = DO0_CRUSHER_MOTOR_OFF+DO1_CRUSHER_MOTOR_FWD;
            portDO_States[0] = m_crusherMotorOutput + m_conveyorMotorOutput + m_sorterMotorOutput + m_ledOutput;
            errorCode = instantDoCtrl->Write(0, portCount, portDO_States);
            m_selfTestItem = SELF_TEST_NONE;
            break;
        }


    }
    QTimer::singleShot(10, this, SLOT(updateSignal_out()));
}

void pcieIO::savePreviousSensorState(int index)
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

void pcieIO::loadPreviousSensorState()
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

void pcieIO::connectToDevice()
{
}


void pcieIO::disConnectFromDevice()
{
//yc    modbusDevice->disconnectDevice();
    m_bConnected = false;
}



void pcieIO::pcieIOError(int error)
{
//yc    qLog(LOGLEVEL_VERY_HIGH, "PLC", "Error", QString("modbusError:  %1 (%2)").arg(modbusDevice->errorString()).arg(error));
}

void pcieIO::pcieIOStateChanged(int state)
{

    emit portConnected(m_bConnected);

    {
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


void pcieIO::writeRequest(int index, bool value)
{
     ErrorCode errorCode = Success;
    switch(index)
    {
    case PAC_START:
        m_RecyclingStart = value;
        if(value==false)
            m_seq = SEQUENCE_INIT;
        break;
    case SELF_TEST_START:
        if(value==true)
        {
            m_ledOutput = DO6_LED_LIGHT_ON;
            portDO_States[0] = m_crusherMotorOutput + m_conveyorMotorOutput + m_sorterMotorOutput + m_ledOutput;
            errorCode = instantDoCtrl->Write(0, portCount, portDO_States);
            m_seq = SEQUENCE_SELFTEST;
        }
        else
        {
            m_ledOutput = DO6_LED_LIGHT_OFF;
            portDO_States[0] = m_crusherMotorOutput + m_conveyorMotorOutput + m_sorterMotorOutput + m_ledOutput;
            errorCode = instantDoCtrl->Write(0, portCount, portDO_States);
            m_seq = SEQUENCE_INIT;
        }
        break;
    case SELF_TEST_STEPPING_F:
        m_selfTestItem = index;
        break;
    case SELF_TEST_STEPPING_R:
        m_selfTestItem = index;
        break;
    case SELF_TEST_SORTER_L:
        m_selfTestItem = index;
        break;
    case SELF_TEST_SORTER_R:
        m_selfTestItem = index;
        break;
    case SELF_TEST_SORTER_C:
        m_selfTestItem = index;
        break;
    case SELF_TEST_GEARED_F:
        m_selfTestItem = index;
        break;
    case SELF_TEST_GEARED_R:
        m_selfTestItem = index;
        break;
    case GEARED_MOTOR_START:
        if(value == 0)
            m_selfTestItem = SELF_TEST_GEARED_S;
        break;
    }

}

void pcieIO::sendRequest()
{
/*    if (!m_bConnected)
        return;

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
yc*/
}

void pcieIO::incommingDataProcess(int index, bool value)
{


    if(m_coils.at(index) == value)
    {

        return;
    }


    //qDebug() << index << "'th value changed : " << gg.at(index) << " --> " << value;
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


void pcieIO::processStorageEmpty()
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
void pcieIO::processDoorOpen()
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
void pcieIO::setStorageChangeEvent(uint nCate, bool value)
{
    if(nCate>=4)
        return;
    qLog(LOGLEVEL_VERY_HIGH, "PLC", "Info", QString("setStorageChangeEvent(%1, %2)").arg(nCate).arg(value));
    m_StorageChangeEvent[nCate] = 100;
    m_StorageChangeValue[nCate] = value;
}


void pcieIO::setPLCSoftResetEvent(bool bSet)
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

void pcieIO::processPLCSoftReset()
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

void pcieIO::processStorage()
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

int  pcieIO::getState()
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

    qDebug() << "rohs : get sensor state - " << nRet;

    return nRet;

}

void pcieIO::writeResult()
{
/*yc    QModbusReply *reply = qobject_cast<QModbusReply *>(sender());
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
    yc*/
}

void pcieIO::readReady()
{
/*yc    QModbusReply *reply = qobject_cast<QModbusReply *>(sender());
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
yc*/
}


