#ifndef PCIEIO_H
#define PCIEIO_H

#include <QObject>
#include <QBitArray>
#include <QTimer>
#include "../../../superbin/Downloads/linux_driver_source_3.2.10.0_64bit/inc/bdaqctrl.h"


#define DI0_CRUSHER_SENSOR       0x01
#define DI1_RESERVED             0x02
#define DI2_CONVEYOR_IN_SENSOR   0x04
#define DI3_CONVEYOR_OUT_SENSOR  0x08
#define DI4_SORTER_R_SENSOR      0x10
#define DI5_SORTER_L_SENSOR      0x20
#define DI6_DOOR_TOP_SENSOR      0x80
#define DI7_DOOR_BOTTOM_SENSOR   0x40

#define DI8_PET_BOX_FULL_SENSOR  0x01
#define DI9_PET_BOX_80_SENSOR    0x02
#define DI10_PET_BOX_NO_SENSOR   0x04
#define DI11_CAN_BOX_FULL_SENSOR 0x08
#define DI12_CAN_BOX_80_SENSOR   0x10
#define DI13_CAN_BOX_NO_SENSOR   0x20

#define DO0_CRUSHER_MOTOR_ON            0x01
#define DO0_CRUSHER_MOTOR_OFF           0x00
#define DO1_CRUSHER_MOTOR_REV           0x02
#define DO1_CRUSHER_MOTOR_FWD           0x00
#define DO2_CONVEYOR_MOTOR_CW_PWM_H     0x04
#define DO2_CONVEYOR_MOTOR_CW_PWM_L     0x00
#define DO3_CONVEYOR_MOTOR_CCW_PWM_H    0x08
#define DO3_CONVEYOR_MOTOR_CCW_PWM_L    0x00
#define DO4_SORTER_MOTOR_FWD_ON         0x10
#define DO4_SORTER_MOTOR_FWD_OFF        0x00
#define DO5_SORTER_MOTOR_REV_ON         0x20
#define DO5_SORTER_MOTOR_REV_OFF        0x00
#define DO6_LED_LIGHT_ON                0x40
#define DO6_LED_LIGHT_OFF               0x00

#define TARGET_LEFT          0
#define TARGET_CENTER        1
#define TARGET_RIGHT         2

#define SORTER_SENSOR_STATE_LEFT    0
#define SORTER_SENSOR_STATE_CENTER  1
#define SORTER_SENSOR_STATE_RIGHT   2
#define SORTER_SENSOR_STATE_UNKNOWN 3

#define SORTER_MOTOR_STATE_STOP         0
#define SORTER_MOTOR_STATE_STOP_WAIT    1
#define SORTER_MOTOR_STATE_LEFT         2
#define SORTER_MOTOR_STATE_RIGHT        3
#define SORTER_MOTOR_STATE_JAM          4

#define SORTER_MOTOR_FWD_PWM_TOTAL_PERIOD 50
#define SORTER_MOTOR_FWD_PWM_LOW_PERIOD 48

#define SORTER_MOTOR_REV_PWM_TOTAL_PERIOD 50
#define SORTER_MOTOR_REV_PWM_LOW_PERIOD 48
#define SORTER_MOTOR_DEFAULT_RUN_TIME 100
#define SORTER_MOTOR_TOTAL_RUN_TIME 2000

#define CONVEYOR_DRIVE_IN   0
#define CONVEYOR_DRIVE_OUT  1

#define SEQUENCE_INIT           0
#define SEQUENCE_INPUT_WAIT     1
#define SEQUENCE_INPUT          2
#define SEQUENCE_CLASSIFY       3
#define SEQUENCE_SORTER_OPEN    4
#define SEQUENCE_INPUT_CRUSHER  5
#define SEQUENCE_SORTER_CLOSE   6
#define SEQUENCE_CRUSHER_JAM    7
#define SEQUENCE_SORTER_JAM     8
#define SEQUENCE_SELFTEST       9
#define SEQUENCE_SORTER_JAM_REQ_WAIT 100


#define DOOR_STATE_CLOSE    0
#define DOOR_STATE_OPEN     1

#define CAM_CLS_NO       0
#define CAM_CLS_REQ      1
#define CAM_CLS_WAIT     2
#define CAM_CLS_DONE     3

#define CAM_CLS_UNKNOWN      0
#define CAM_CLS_PET          1
#define CAM_CLS_CAN          2

#define SORTER_CLOSE_REPEAT 10

using namespace Automation::BDaq;

struct ConfigureParameter
{
    QString deviceName;
};

struct writeCmd
{
    int m_nWriteAddress;
    bool m_bWriteValue;
    writeCmd(int command, bool value) {
       m_nWriteAddress = command;
       m_bWriteValue = value;
    }
};

class pcieIO : public QObject
{
    Q_OBJECT
public:
    explicit pcieIO(int deviceType, QObject *parent = 0);

    ~pcieIO();

    void connectToDevice();

    void writeRequest(int index, bool value);
    int getState();
    void pcieIO_init();
    int serial_open(const char *port, int baud, int blocking);
    int Serial_SendByte(int fd, unsigned char byte);

    ConfigureParameter configure;
    QTimer* timer_DI;
    int portCount;
    int Cam_Classify_wait;
    int Cam_Classified_object;
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
    void sorterCtrl();
    void SorterClose();
    void SorterOpen();
    void ConveyorRun(int clock);
    void ConvIn();
    void ConvInTest();
    void ConvOut();
    void ConvOutTest();
    bool WaitForClassfy();
    void CrusherIn();
    void SensorCheck();

    int m_StorageChangeEvent[4];
    QBitArray m_StorageChangeValue;
    QBitArray m_StorageChangeOldValue;

    int m_PLCSoftResetEvent;

    QBitArray m_coils;
    QBitArray m_input;

    InstantDiCtrl * instantDiCtrl;
    InstantDoCtrl * instantDoCtrl;

    int m_seq;
    int m_selfTestItem;
    uint8 m_crusherMotorOutput;
    uint8 m_conveyorMotorOutput;
    uint8 m_sorterMotorOutput;
    uint8 m_ledOutput;




    bool m_bConnected;

    bool m_bFirst;

    int m_nDeviceType;

    bool m_Crusher_sensor;
    bool m_Conveyor_in_sensor;
    bool m_Conveyor_out_sensor;
    bool m_Sorter_R_sensor;
    bool m_Sorter_L_sensor;
    bool m_Door_top_sensor;
    bool m_Door_bottom_sensor;
    bool m_Pet_full_sensor;
    bool m_Pet_80_sensor;
    bool m_Pet_no_sensor;
    bool m_Can_full_sensor;
    bool m_Can_80_sensor;
    bool m_Can_no_sensor;

    bool m_bCrusher_sensor;
    bool m_bConveyor_in_sensor;
    bool m_bConveyor_out_sensor;
    bool m_bSorter_R_sensor;
    bool m_bSorter_L_sensor;
    bool m_bDoor_top_sensor;
    bool m_bDoor_bottom_sensor;
    bool m_bPet_full_sensor;
    bool m_bPet_80_sensor;
    bool m_bPet_no_sensor;
    bool m_bCan_full_sensor;
    bool m_bCan_80_sensor;
    bool m_bCan_no_sensor;

    int m_Sorter_target;
    int m_Sorter_target_before;
    int m_Sorter_sensor_state;
    int m_Sorter_motor_state;
    int m_conveyor_rotation_dir;

    int SorterAction[3][4][4];

    unsigned char *portDO_States;
    bool m_RecyclingStart;
    bool m_doorOpenState;
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

    void pcieIOError(int error);
    void pcieIOStateChanged(int state);
    void updateSignal_in();
    void updateSignal_out();
};

#endif // MODBUSINTERFACE_H
