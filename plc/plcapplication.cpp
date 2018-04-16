#include "plcapplication.h"
#include "plctask.h"

#include <QTimer>
#include <QSettings>

#include "global.hpp"

#include <thread>

#include <pthread.h>

#include <sys/reboot.h>
#include <unistd.h>

//int door_bottom_state = 0;
//int door_top_state = 0;

void *sensor_update_thread_function(void *pt)
{
    unsigned char rx0,rx1,rx2;
    unsigned char ret_l=0, ret_h=0;

//    static int door_bottom_already_open = 0;
//    static int door_bottom_already_closed = 0;
//    static int door_top_already_open = 0;
//    static int door_top_already_closed = 0;

    static int door_bottom_state = 0; //0: close , 1: open
    static int door_top_state = 0; //0: close , 1: open


    PLCApplication *task = (PLCApplication*)pt;
    for(;;)
    {
        SERIAL_READ_RESET:
        task->ATM.Serial_Read(&rx0,1);
        //qDebug() << "rx0" <<  rx0;

        if(task->ATM.Serial_Read_with_Counter(&rx1,1) == 0)
        {
            qDebug() << "SERIAL_READ_RESET1";
            goto SERIAL_READ_RESET;
        }
        //qDebug() << "rx1" <<  rx1;


        if(task->ATM.Serial_Read_with_Counter(&rx2,1) == 0)
        {
            qDebug() << "SERIAL_READ_RESET2";
            goto SERIAL_READ_RESET;
        }
        //qDebug() << "rx2" <<  rx2;


        if((rx0 == 'c') && (rx1 == 'l') && (rx2 == 's')) //classify command
        {
            qDebug() << "!!rohs : Cam classify cmd";
            //QThread::msleep(100);
            task->stateChanged(PS_MOVINGCLSPOINT);
            //task->ATM.Serial_SendByte('e'); //reject test

        }

        else if((rx0 == 'j') && (rx1 == 'a') && (rx2 == 'm')) //jam detected
        {
            qDebug() << "!!rohs : JAM detected";
            //send to ui for jam
            //task->ATM.CrusherMotor_Stop_Now();
            task->crusher_motor_stop_timer_flag = 0;
            task->crusher_motor_stop_timer_count = 0;
            //task->ATM.Conveyor_Stop();
            //task->ATM.Sorter_Stop();
            task->ATM.MasterReset();
            task->bErrorPLC = true;
            task->detectPlcFault(PS_JAM, 1);
        }

        else if((rx0 == 'd') && (rx1 == 'b') && (rx2 == 'o')) //door bottom open
        {
            door_bottom_state = 1;
            task->ATM.CrusherMotor_Stop_Now(0);
            task->crusher_motor_stop_timer_flag = 0;
            task->crusher_motor_stop_timer_count = 0;

            if(door_top_state == 0){    //bottom open, top close
                task->detectPlcFault(PS_DOOROPEN,2);
            }else{  //all open
                task->detectPlcFault(PS_DOOROPEN,3);
            }


        }
        else if((rx0 == 'd') && (rx1 == 'b') && (rx2 == 'c')) //door bottom close
        {
            door_bottom_state = 0;

            if(door_top_state == 0){    //all close
                task->detectPlcFault(PS_DOOROPEN,0);
            }else{  //bottom close, top open
                task->detectPlcFault(PS_DOOROPEN,1);
            }

        }
        else if((rx0 == 'd') && (rx1 == 't') && (rx2 == 'o')) //door top open
        {
            door_top_state = 1;

            task->ATM.CrusherMotor_Stop_Now(0);
            task->crusher_motor_stop_timer_flag = 0;
            task->crusher_motor_stop_timer_count = 0;

            if(door_bottom_state == 0){    //top open, bottom close
                task->detectPlcFault(PS_DOOROPEN,1);
            }else{  //all open
                task->detectPlcFault(PS_DOOROPEN,3);
            }

        }
        else if((rx0 == 'd') && (rx1 == 't') && (rx2 == 'c')) //door top close
        {
            door_top_state = 0;
            if(door_bottom_state == 0){    //all close
                task->detectPlcFault(PS_DOOROPEN,0);
            }else{  //bottom open, top close
                task->detectPlcFault(PS_DOOROPEN,2);
            }
        }



        else if(rx0 == 'u')
        {
            task->atm_sensor_state = 0;
            ret_l = rx1;
            ret_h = rx2;

//            int h_bit7 = (int)ret_h/128; ret_h %= 128;  // error correction
//            int h_bit6 = (int)ret_h/64;  ret_h %= 64;   // error correction
            int h_bit5 = (int)ret_h/32;  ret_h %= 32;   // conveyor IR_B
            int h_bit4 = (int)ret_h/16;  ret_h %= 16;   // conveyor IR_F
            int h_bit3 = (int)ret_h/8;   ret_h %= 8;    // hallsensor R
            int h_bit2 = (int)ret_h/4;   ret_h %= 4;    // hallsensor L
            int h_bit1 = (int)ret_h/2;   ret_h %= 2;    // CAN bin Empty
            int h_bit0 = (int)ret_h;                    // PET bin Empty

//            int l_bit7 = (int)ret_l/128; ret_l %= 128;  // error correction
//            int l_bit6 = (int)ret_l/64;  ret_l %= 64;   // error correction
            int l_bit5 = (int)ret_l/32;  ret_l %= 32;   // Door Bottom
            int l_bit4 = (int)ret_l/16;  ret_l %= 16;   // Door Top
            int l_bit3 = (int)ret_l/8;   ret_l %= 8;    // CAN 80%
            int l_bit2 = (int)ret_l/4;   ret_l %= 4;    // CAN 100%
            int l_bit1 = (int)ret_l/2;   ret_l %= 2;    // PET 80%
            int l_bit0 = (int)ret_l;                    // PET 100%

            //rohs edit : sensor value, change FORCED
//            l_bit5 = 0; // Door Bottom
//           l_bit4 = 1; // Door Top
//            l_bit3 = 0; // CAN 80%
//            l_bit2 = 0; // CAN 100%
//            l_bit1 = 0; // PET 80%
//            l_bit0 = 0; // PET 100%
            h_bit1 = 0;  // CAN bin Empty
            h_bit0 = 0;  // PET bin Empty

            task->door_bottom_state = l_bit5;
            task->door_top_state = l_bit4;

            task->hall_sensor_state_R = h_bit3;
            task->hall_sensor_state_L = h_bit2;

            qDebug() <<"sensor update:"<< l_bit5 << l_bit4 << l_bit3 << l_bit2 << l_bit1 << l_bit0 << h_bit5 << h_bit4 << h_bit3 << h_bit2 << h_bit1 << h_bit0;


            if(l_bit0 == 1) task->atm_sensor_state |= (1 << 0);
            if(l_bit1 == 1) task->atm_sensor_state |= (1 << 1);
            if(l_bit2 == 1) task->atm_sensor_state |= (1 << 2);
            if(l_bit3 == 1) task->atm_sensor_state |= (1 << 3);
            if(l_bit4 == 1) task->atm_sensor_state |= (1 << 4);
            if(l_bit5 == 1) task->atm_sensor_state |= (1 << 5);
            if(h_bit0 == 1) task->atm_sensor_state |= (1 << 6);
            if(h_bit1 == 1) task->atm_sensor_state |= (1 << 7);

            //qDebug() <<"atm_sensor_state" << atm_sensor_state;


            static char state_PLC_ENDIR = 0;
            if(state_PLC_ENDIR == 0)
            {
                if(h_bit5 == 1)
                {
                    task->plcSensorStateChange(PLC_ENDIR,1);
                    state_PLC_ENDIR = 1;
                }
            }
            else
            {
                if(h_bit5 == 0)
                {
                    task->plcSensorStateChange(PLC_ENDIR,0);
                    state_PLC_ENDIR = 0;
                }
            }

            static char state_PLC_STARTIR = 0;
            if(state_PLC_STARTIR == 0)
            {
                if(h_bit4 == 1)
                {
                    task->plcSensorStateChange(PLC_STARTIR,1);
                    state_PLC_STARTIR = 1;
                }
            }
            else
            {
                if(h_bit4 == 0)
                {
                    task->plcSensorStateChange(PLC_STARTIR,0);
                    state_PLC_STARTIR = 0;
                }
            }


            static char state_PLC_CAN_TRAY = 0;
            if(state_PLC_CAN_TRAY == 0)
            {
                if(h_bit1 == 1)
                {
                    task->plcSensorStateChange(PLC_CAN_TRAY_SENSOR,1);
                    state_PLC_CAN_TRAY = 1;
                }
            }
            else
            {
                if(h_bit1 == 0)
                {
                    task->plcSensorStateChange(PLC_CAN_TRAY_SENSOR,0);
                    state_PLC_CAN_TRAY = 0;
                }
            }

            static char state_PLC_PET_TRAY = 0;
            if(state_PLC_PET_TRAY == 0)
            {
                if(h_bit0 == 1)
                {
                    task->plcSensorStateChange(PLC_PET_TRAY_SENSOR,1);
                    state_PLC_PET_TRAY = 1;
                }
            }
            else
            {
                if(h_bit0 == 0)
                {
                    task->plcSensorStateChange(PLC_PET_TRAY_SENSOR,0);
                    state_PLC_PET_TRAY = 0;
                }
            }


            static char state_DOOR_BOTTOM = 0;
            if(state_DOOR_BOTTOM == 0)
            {
                if(l_bit5 == 1)
                {
                    task->plcSensorStateChange(DOOR_BOTTOM_S,1);
                    state_DOOR_BOTTOM = 1;
                }
            }
            else
            {
                if(l_bit5 == 0)
                {
                    task->plcSensorStateChange(DOOR_BOTTOM_S,0);
                    state_DOOR_BOTTOM = 0;
                }
            }



            static char state_DOOR_TOP = 0;
            if(state_DOOR_TOP == 0)
            {
                if(l_bit4 == 1)
                {
                    task->plcSensorStateChange(DOOR_TOP_S,1);
                    state_DOOR_TOP = 1;
                }
            }
            else
            {
                if(l_bit4 == 0)
                {
                    task->plcSensorStateChange(DOOR_TOP_S,0);
                    state_DOOR_TOP = 0;
                }
            }

            static char state_PLC_CAN_80 = 0;
            if(state_PLC_CAN_80 == 0)
            {
                if(l_bit3 == 1)
                {
                    task->plcSensorStateChange(PLC_CAN_80_SENSOR,1);
                    state_PLC_CAN_80 = 1;
                }
            }
            else
            {
                if(l_bit3 == 0)
                {
                    task->plcSensorStateChange(PLC_CAN_80_SENSOR,0);
                    state_PLC_CAN_80 = 0;
                }
            }


            static char state_PLC_CAN_FULL = 0;
            if(state_PLC_CAN_FULL == 0)
            {
                if(l_bit2 == 1)
                {
                    task->plcSensorStateChange(PLC_CAN_FULL_SENSOR,1);
                    state_PLC_CAN_FULL = 1;
                    //
                }
            }
            else
            {
                if(l_bit2 == 0)
                {
                    task->plcSensorStateChange(PLC_CAN_FULL_SENSOR,0);
                    state_PLC_CAN_FULL = 0;
                }
            }



            static char state_PLC_PET_80 = 0;
            if(state_PLC_PET_80 == 0)
            {
                if(l_bit1 == 1)
                {
                    task->plcSensorStateChange(PLC_PET_80_SENSOR,1);
                    state_PLC_PET_80 = 1;
                }
            }
            else
            {
                if(l_bit1 == 0)
                {
                    task->plcSensorStateChange(PLC_PET_80_SENSOR,0);
                    state_PLC_PET_80 = 0;
                }
            }


            static char state_PLC_PET_FULL = 0;
            if(state_PLC_PET_FULL == 0)
            {
                if(l_bit0 == 1)
                {
                    task->plcSensorStateChange(PLC_PET_FULL_SENSOR,1);
                    state_PLC_PET_FULL = 1;
                }
            }
            else
            {
                if(l_bit0 == 0)
                {
                    task->plcSensorStateChange(PLC_PET_FULL_SENSOR,0);
                    state_PLC_PET_FULL = 0;
                }
            }
        }

        //for message ack

        else if((rx0 == 'v') && (rx1 == '0') && (rx2 == '0')) task->ATM.conveyor_stop_ack=0;
        else if((rx0 == 'v') && (rx1 == '0') && (rx2 == '1')) task->ATM.conveyor_cw_ack=0;
        else if((rx0 == 'v') && (rx1 == '0') && (rx2 == '2')) task->ATM.conveyor_ccw_ack=0;
        else if((rx0 == 'v') && (rx1 == '0') && (rx2 == '3')) task->ATM.conveyor_task_on_ack=0;
        else if((rx0 == 'v') && (rx1 == '0') && (rx2 == '4')) task->ATM.conveyor_task_off_ack=0;
        else if((rx0 == 'v') && (rx1 == '0') && (rx2 == '5')) task->ATM.conveyor_task_rev_ack=0;
        else if((rx0 == 'v') && (rx1 == '0') && (rx2 == '6')) task->ATM.conveyor_task_cont_ack=0;

        else if((rx0 == 'c') && (rx1 == '0') && (rx2 == '0')) task->ATM.crusher_stop_ack=0;
        else if((rx0 == 'c') && (rx1 == '0') && (rx2 == '1')) task->ATM.crusher_fx_ack=0;
        else if((rx0 == 'c') && (rx1 == '0') && (rx2 == '2')) task->ATM.crusher_rx_ack=0;
        else if((rx0 == 'c') && (rx1 == '0') && (rx2 == '3')) task->ATM.crusher_emstop_ack=0;

        else if((rx0 == 's') && (rx1 == '0') && (rx2 == '0')) task->ATM.sorter_init_ack=0;
        else if((rx0 == 's') && (rx1 == '0') && (rx2 == '1')) task->ATM.sorter_left_ack=0;
        else if((rx0 == 's') && (rx1 == '0') && (rx2 == '2')) task->ATM.sorter_right_ack=0;
        else if((rx0 == 's') && (rx1 == '0') && (rx2 == '3')) task->ATM.sorter_center_ack=0;
        else if((rx0 == 's') && (rx1 == '0') && (rx2 == '4')) task->ATM.sorter_stop_ack=0;

        else if((rx0 == 'l') && (rx1 == '0') && (rx2 == '0')) task->ATM.LED_Off_ack = 0;
        else if((rx0 == 'l') && (rx1 == '0') && (rx2 == '1')) task->ATM.LED_On_ack = 0;

        else if((rx0 == 'a') && (rx1 == '0') && (rx2 == '0')) task->ATM.can_pet_action_on_ack = 0;
        else if((rx0 == 'a') && (rx1 == '0') && (rx2 == '1')) task->ATM.can_pet_action_off_ack = 0;

        //20180311 //long term test
//        else if((rx0 == 't') && (rx1 == '0') && (rx2 == '0')) task->ATM.long_term_test_conv_off_ack = 0;
//        else if((rx0 == 't') && (rx1 == '0') && (rx2 == '1')) task->ATM.long_term_test_conv_on_ack = 0;
//        else if((rx0 == 't') && (rx1 == '0') && (rx2 == '2')) task->ATM.long_term_test_crusher_off_ack = 0;
//        else if((rx0 == 't') && (rx1 == '0') && (rx2 == '3')) task->ATM.long_term_test_crusher_on_ack = 0;
//        else if((rx0 == 't') && (rx1 == '0') && (rx2 == '4')) task->ATM.long_term_test_sorter_off_ack = 0;
//        else if((rx0 == 't') && (rx1 == '0') && (rx2 == '5')) task->ATM.long_term_test_sorter_on_ack = 0;

    }

    return NULL;

}



PLCApplication::PLCApplication(int& argc, char** argv)
    : super(argc, argv)
{
    bPLCOnline = false;
    context = createDefaultContext(this);
    context->start();

    stopStart =false;
    bErrorPLC = false;

    QTimer::singleShot(0, this, SLOT(run()));

    QTimer::singleShot(1000, this, SLOT(crusher_motor_stop_timer()));

    QTimer::singleShot(500, this, SLOT(sensor_update_timer()));

    //20180311 test
    QTimer::singleShot(1000, this, SLOT(long_term_test_timer_conveyor()));
    QTimer::singleShot(1000, this, SLOT(long_term_test_timer_crusher()));
    QTimer::singleShot(1000, this, SLOT(long_term_test_timer_sorter()));

    latestType = ERR_VALUE;


    QSettings setting("NTN", "Nephron");
    deviceType = setting.value("DeviceType").toInt();

    //rohs edit
    if(ATM.Serial_Open("/dev/ttyS0",57600,0)==1)
    {
        //ATM.Serial_String("Port Open");
        qDebug() << "Successfully Serial Port open";
    }
    else
    {
        qDebug() << "Fail to Serial Port open";
    }

}


PLCApplication::~PLCApplication()
{
    if(reporter)
    {
        reporter->stop();
        delete reporter;
    }
}


bool PLCApplication::notify(QObject *obj, QEvent *event){
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


void PLCApplication::crusher_motor_stop_timer()
{

    if(crusher_motor_stop_timer_flag == 1)
    {
        qDebug() << "crusher_motor_stop_timer_count : " << crusher_motor_stop_timer_count;
        crusher_motor_stop_timer_count++;
        if(crusher_motor_stop_timer_count == 20)
        {
            qDebug() << "STOP NOW!!";
            ATM.CrusherMotor_Stop_Now(1);
            crusher_motor_stop_timer_flag = 0;
            crusher_motor_stop_timer_count = 0;
        }
    }
    QTimer::singleShot(2000, this, SLOT(crusher_motor_stop_timer()));
}


void PLCApplication::sensor_update_timer()
{
    if(sensor_update_timer_flag == 1)
    {
        ATM.Get_Sensor_Update();
    }
    QTimer::singleShot(500, this, SLOT(sensor_update_timer()));
}


//20180311 long term test

void PLCApplication::long_term_test_timer_conveyor()
{
    if(long_term_test_timer_conveyor_flag==1)
    {
        qDebug() << "LTT : Conveyor : " << long_term_test_counter_conveyor;
        if(long_term_test_counter_conveyor == 0)   ATM.Conveyor_Front();
        if(long_term_test_counter_conveyor ==10)   ATM.Conveyor_Stop();
        if(long_term_test_counter_conveyor ==12)   ATM.Conveyor_Back();
        if(long_term_test_counter_conveyor ==22)   ATM.Conveyor_Stop();
        if(long_term_test_counter_conveyor ==23)   long_term_test_counter_conveyor = -1;
        long_term_test_counter_conveyor++;
        long_term_test_counterMAX_conveyor++;
        if(long_term_test_counterMAX_conveyor == 60*120)
        {
            qDebug() << "LTT : Conveyor : Counter MAX reached";
            long_term_test_timer_conveyor_flag = 0;
            long_term_test_counterMAX_conveyor = 0;
        }
    }
    if(long_term_test_timer_conveyor_flag==0)
    {
        ATM.Conveyor_Stop();
        long_term_test_counter_conveyor = 0;
        qDebug() << "LTT : Conveyor : " << long_term_test_counter_conveyor;
        long_term_test_timer_conveyor_flag = -1;
    }
    QTimer::singleShot(1000, this, SLOT(long_term_test_timer_conveyor()));
}


void PLCApplication::long_term_test_timer_crusher()
{
    if(long_term_test_timer_crusher_flag==1)
    {
        qDebug() << "LTT : Crusher : " << long_term_test_counter_crusher;
        if(long_term_test_counter_crusher == 0)   ATM.CrusherMotor_Forward();
        if(long_term_test_counter_crusher ==10)   ATM.CrusherMotor_Stop_Now(0);
        if(long_term_test_counter_crusher ==12)   ATM.CrusherMotor_Reverse();
        if(long_term_test_counter_crusher ==22)   ATM.CrusherMotor_Stop_Now(0);
        if(long_term_test_counter_crusher ==23)   long_term_test_counter_crusher = -1;
        long_term_test_counter_crusher++;
        long_term_test_counterMAX_crusher++;
        if(long_term_test_counterMAX_crusher == 60*120)
        {
            qDebug() << "LTT : Crusher : Counter MAX reached";
            long_term_test_timer_crusher_flag = 0;
            long_term_test_counterMAX_crusher = 0;
        }
    }
    if(long_term_test_timer_crusher_flag==0)
    {
        ATM.CrusherMotor_Stop_Now(0);
        long_term_test_counter_crusher = 0;
        qDebug() << "LTT : Crusher : " << long_term_test_counter_crusher;
        long_term_test_timer_crusher_flag = -1;
    }
    QTimer::singleShot(1000, this, SLOT(long_term_test_timer_crusher()));
}

void PLCApplication::long_term_test_timer_sorter()
{
    if(long_term_test_timer_sorter_flag==1)
    {
        qDebug() << "LTT : Sorter : " << long_term_test_counter_sorter;
        if(long_term_test_counter_sorter == 0)   ATM.Sorter_Right();
        if(long_term_test_counter_sorter == 5)   ATM.Sorter_Center();
        if(long_term_test_counter_sorter ==10)   ATM.Sorter_Left();
        if(long_term_test_counter_sorter ==15)   ATM.Sorter_Center();
        if(long_term_test_counter_sorter ==19)   long_term_test_counter_sorter = -1;
        long_term_test_counter_sorter++;
        long_term_test_counterMAX_sorter++;
        if(long_term_test_counterMAX_sorter == 60*120)
        {
            qDebug() << "LTT : Crusher : Counter MAX reached";
            long_term_test_timer_sorter_flag = 0;
            long_term_test_counterMAX_sorter = 0;
        }
    }
    if(long_term_test_timer_sorter_flag==0)
    {
        ATM.Sorter_Center();
        long_term_test_counter_sorter = 0;
        qDebug() << "LTT : Crusher : " << long_term_test_counter_sorter;
        long_term_test_timer_sorter_flag = -1;
    }
    QTimer::singleShot(1000, this, SLOT(long_term_test_timer_sorter()));
}

void PLCApplication::run()
{

    try
    {
        /*
        PLCInterface = new PLCTask(this);
        connect(PLCInterface, SIGNAL(portConnected(bool)),SLOT(onConnect(bool)));

        connect(PLCInterface, SIGNAL(plcStateChage(uint)), SLOT(stateChanged(uint)));
        connect(PLCInterface, SIGNAL(plcErrorDetect(uint,char)), SLOT(detectPlcFault(uint,char)));

        connect(PLCInterface, SIGNAL(reportError(uint)), SLOT(reportError(uint)));

        PLCInterface->reconnect();
*/



        //using PCI card
//        PLCInterface = new pcieIO(deviceType,this);
//        connect(PLCInterface, SIGNAL(portConnected(bool)),SLOT(onConnect(bool))); //not necessary
//        connect(PLCInterface, SIGNAL(plcStateChage(uint)), SLOT(stateChanged(uint))); //important
//        connect(PLCInterface, SIGNAL(plcErrorDetect(uint,char)), SLOT(detectPlcFault(uint,char)));
//        connect(PLCInterface, SIGNAL(reportError(uint)), SLOT(reportError(uint))); // null
//        connect(PLCInterface, SIGNAL(sensorStateChange(int, bool)), SLOT(plcSensorStateChange(int,bool))); //not necessary
//        PLCInterface->connectToDevice();



        worker = new Worker(*context, "PLCInterface","PLC", 0, this);
        connect(worker, SIGNAL(finished()), SLOT(quit()));
        connect(worker, SIGNAL(requestReceived(const QString& , const QString& )), SLOT(requestReceived(const QString& , const QString&)));
        worker->start();

        client = new Client(*context,"PLCC",0,this);
        connect(client, SIGNAL(responseReceived(const QString&, const QString&)), SLOT(responseReceived(const QString&,const QString&)));
        client->start();

        reporter = new Publisher(*context,REPORT_IPC_PLC,"PLC" );
        reporter->start();
        connect(reporter, SIGNAL(messageSent(const QList<QByteArray>& )), SLOT(messageSent(const QList<QByteArray>& )));

        listener = new Subscriber(*context, BROADCAST_IPC,"PLC");
        //reporter->setSubscribe("");
        connect(listener,SIGNAL(receivedMessage(const QList<QByteArray>&)), SLOT(receivedMessage(const QList<QByteArray>&)) );
        listener->start();
        if(deviceType == 0)
        {
            checkCompressStepTimer = new QTimer(this);
            checkCompressStepTimer->setSingleShot(true);

            checkCompressStepTimer->setInterval(10000);

            connect(checkCompressStepTimer, SIGNAL(timeout()), SLOT(compressStepError()));
        }

        QTimer* tmpTimer = new QTimer(this);
        connect(tmpTimer, SIGNAL(timeout()), this, SLOT(tmpSlot()));

        qDebug() << "ATmega128 app run";

        pthread_t sensor_update_thread;
        if(pthread_create(&sensor_update_thread, NULL, sensor_update_thread_function, this)) {
            //fprintf(stderr, "Error creating thread\n");
            qDebug() << "Error creating thread";
            exit(1);
        }
        else
        {
            qDebug() << "Sensor Thread on";
        }

        QThread::msleep(200);
        ATM.MasterReset();
        QThread::msleep(200);
        ATM.Get_Sensor_Update();

        //dev mode
        //rohs : 20180205 sorter door init
        if(true)
        {
            int sorter_jam_detect = 0;
            ATM.Sorter_Left();
            QThread::msleep(3000);
            ATM.Sorter_Left();
            QThread::msleep(3000);

            ATM.Get_Sensor_Update();
            QThread::msleep(100);
            qDebug() << "hall sensor" << hall_sensor_state_L << hall_sensor_state_R;
            if((hall_sensor_state_L == 1) && (hall_sensor_state_R == 0))
            {
                sorter_jam_detect = 0;
            }
            else
            {
                ATM.Sorter_Stop();
                sorter_jam_detect = 1;
                qDebug() << "Sorter JAM!!!";
                bErrorPLC = true;
                detectPlcFault(PS_JAM, 1);
            }

            if(sorter_jam_detect == 0)
            {
                ATM.Sorter_Right();
                QThread::msleep(3000);
                ATM.Sorter_Right();
                QThread::msleep(3000);

                ATM.Get_Sensor_Update();
                QThread::msleep(100);
                qDebug() << "hall sensor" << hall_sensor_state_L << hall_sensor_state_R;
                if((hall_sensor_state_L == 0) && (hall_sensor_state_R == 1))
                {
                    sorter_jam_detect = 0;
                }
                else
                {
                    ATM.Sorter_Stop();
                    sorter_jam_detect = 1;
                    qDebug() << "Sorter JAM!!!";
                    bErrorPLC = true;
                    detectPlcFault(PS_JAM, 1);
                }
            }
            if(sorter_jam_detect == 0)
            {
                ATM.Sorter_Center();
                QThread::msleep(3000);
                qDebug() << "Sorter JAM solved";
                bErrorPLC = false;
                detectPlcFault(PS_JAM, 0);
            }
        }

        //20180409 test



    }

    catch (std::exception& ex)
    {
        qWarning() << ex.what();
        exit(-1);
    }
}

void PLCApplication::tmpSlot()
{
    QByteArray cmdData;
    cmdData.clear();
    cmdData.append(QChar(REPORT_ERROR));
    QByteArray data ;
    char doorOpenState;
    doorOpenState = 2;
    data.clear();
    data.append(QChar(PS_DOOROPEN));
    data.append(doorOpenState);
    reporter->sendMessage(cmdData, data);
}


void PLCApplication::plcSensorStateChange(int index, bool value)
{
    QByteArray cmdData;
    cmdData.clear();
    cmdData.append(QChar(REPORT_SENSORSTATE));
    QByteArray data ;
    data.clear();
    data.append(uchar(index));
    data.append(QChar(value));

    reporter->sendMessage(cmdData, data);
}


void PLCApplication::full_check_sensor_update(void)
{
    ATM.Get_Sensor_Update();
    QThread::msleep(100);
    int atm_sensor_state_filter = atm_sensor_state & 0x0000000f;
    qDebug() << "full check sensor update :" <<atm_sensor_state_filter;

    //full state
    if((atm_sensor_state_filter == 3) || (atm_sensor_state_filter == 12) || (atm_sensor_state_filter == 15))
    {
        full_check_sensor_counter++;
        qDebug() << "count : full check sensor :" << full_check_sensor_counter;
    }
    else
    {
        full_check_sensor_counter = 0;
    }
    if(full_check_sensor_counter >= 5)
    {
        qDebug() << "maximum count : full check sensor ";
        detectPlcFault(PS_STORAGE_FULL,atm_sensor_state_filter);
    }
}


void PLCApplication::messageSent(const QList<QByteArray>& message)
{
    QString sLog ;
    //for(int i = 0 ; i < message.count(); i++)
   // {
 //       sLog.append(QString("\n%1 : ").arg(i));
  //      sLog.append(QString(message[i]));
  //  }
    //qLog(LOGLEVEL_VERY_HIGH, "PLC", "Info", QString("Publish message sented :%1").arg(sLog));

}

void PLCApplication::receivedMessage(const QList<QByteArray>& message)
{

    qDebug() << "SUB >> ";
    for(int i = 0 ; i < message.count(); i++)
    {
        qDebug() << i << ": " << message[i];
    }

}

int previous_direction;
//rohs edit
void PLCApplication::responseReceived(const QString& service, const QString& msgbody)
{

    int plcRegIdx =0;
    qDebug() << "!!!responseReceived:  " << msgbody;
    qLog(LOGLEVEL_VERY_HIGH, "PLC", "Info", QString("responseReceived :%1").arg(msgbody));


    //PLCInterface->Cam_Classified_object = CAM_CLS_UNKNOWN; //rohs edit
    if(msgbody.compare("CAN")==0)
    {
        qDebug() << "PLC CAN D etected!!! ";
        latestType = CLS_CAN;
        stateChanged(PS_GETREADY);
        stateChanged(PS_RECYCLINGPROCEED);

        QThread::msleep(100);
        ATM.Serial_SendByte('c'); //PC > microcontroller

        //20180411 100% sensor check
        QThread::msleep(200);
        full_check_sensor_update();
        QThread::msleep(2300);

        stateChanged(PS_RECYCLINGEND);
//      full_check_sensor_update();

    }
    else if(msgbody.compare("PET")==0)
    {
        qDebug() << "PLC PET Detected!!! ";
        latestType = CLS_PET;
        stateChanged(PS_GETREADY);
        stateChanged(PS_RECYCLINGPROCEED);

        QThread::msleep(100);
        ATM.Serial_SendByte('p'); //PC > microcontroller

        //20180411 100% sensor check
        QThread::msleep(200);
        full_check_sensor_update();
        QThread::msleep(2300);

        stateChanged(PS_RECYCLINGEND);
        //full_check_sensor_update();
    }
    else if(msgbody.compare("RECYCLE")==0)
    {
        qDebug() << "RECYCLE GLASS Detected!!! ";
        latestType = CLS_RECYLE;

        stateChanged(PS_GETREADY);
        stateChanged(PS_RETURNOBJECT);
        QThread::msleep(500);
        ATM.Serial_SendByte('e');
        QThread::msleep(2500);

        stateChanged(PS_RECYCLINGEND);
    }
    else if(msgbody.compare("REUSE20")==0)
    {
        qDebug() << "REUSE20 Detected!!! ";
        latestType = CLS_REUSE20;

        stateChanged(PS_GETREADY);
        stateChanged(PS_RETURNOBJECT);
        QThread::msleep(500);
        ATM.Serial_SendByte('e');
        QThread::msleep(2500);

        stateChanged(PS_RECYCLINGEND);
    }
    else if(msgbody.compare("REUSE40")==0)
    {
        qDebug() << "REUSE40 GLASS Detected!!! ";
        latestType = CLS_REUSE40;

        stateChanged(PS_GETREADY);
        stateChanged(PS_RETURNOBJECT);
        QThread::msleep(500);
        ATM.Serial_SendByte('e');
        QThread::msleep(2500);

        stateChanged(PS_RECYCLINGEND);
    }
    else if(msgbody.compare("REUSE50")==0)
    {
        qDebug() << "REUSE50 GLASS Detected!!! ";
        latestType = CLS_REUSE50;

        stateChanged(PS_GETREADY);
        stateChanged(PS_RETURNOBJECT);
        QThread::msleep(500);
        ATM.Serial_SendByte('e');
        QThread::msleep(2500);

        stateChanged(PS_RECYCLINGEND);
    }
    else if(msgbody.compare("REUSE100")==0)
    {
        qDebug() << "REUSE100 GLASS Detected!!! ";
        latestType = CLS_REUSE100;

        stateChanged(PS_GETREADY);
        stateChanged(PS_RETURNOBJECT);
        QThread::msleep(500);
        ATM.Serial_SendByte('e');
        QThread::msleep(2500);

        stateChanged(PS_RECYCLINGEND);
    }
    else if(msgbody.compare("REUSE20_N")==0)
    {
        qDebug() << "REUSE20_N Detected!!! ";
        latestType = CLS_REUSE20_N;

        stateChanged(PS_GETREADY);
        stateChanged(PS_RETURNOBJECT);
        QThread::msleep(500);
        ATM.Serial_SendByte('e');
        QThread::msleep(2500);

        stateChanged(PS_RECYCLINGEND);
    }
    else if(msgbody.compare("REUSE40_N")==0)
    {
        qDebug() << "REUSE40_N GLASS Detected!!! ";
        latestType = CLS_REUSE40_N;

        stateChanged(PS_GETREADY);
        stateChanged(PS_RETURNOBJECT);
        QThread::msleep(500);
        ATM.Serial_SendByte('e');
        QThread::msleep(2500);

        stateChanged(PS_RECYCLINGEND);
    }
    else if(msgbody.compare("REUSE50_N")==0)
    {
        qDebug() << "REUSE50_N GLASS NEW Detected!!! ";
        latestType = CLS_REUSE50_N;

        stateChanged(PS_GETREADY);
        stateChanged(PS_RETURNOBJECT);
        QThread::msleep(500);
        ATM.Serial_SendByte('e');
        QThread::msleep(2500);

//        stateChanged(PS_RECYCLINGEND);
    }
    else if(msgbody.compare("REUSE100_N")==0)
    {
        qDebug() << "REUSE100_N GLASS Detected!!! ";
        latestType = CLS_REUSE100_N;

        stateChanged(PS_GETREADY);
        stateChanged(PS_RETURNOBJECT);
        QThread::msleep(500);
        ATM.Serial_SendByte('e');
        QThread::msleep(2500);

//        stateChanged(PS_RECYCLINGEND);
    }
    else if(msgbody.compare("ETC")==0)
    {
        qDebug() << "PLC ETC Detected!!! ";
        //stateChanged(PS_RETURNOBJECT);
        stateChanged(PS_GETREADY);
        latestType = CLS_ETC;

        QThread::msleep(100);
        ATM.Serial_SendByte('e');

        //20180411 100% sensor check
        QThread::msleep(200);
        full_check_sensor_update();
        QThread::msleep(2300);

        stateChanged(PS_RETURNOBJECT);
        //full_check_sensor_update();


    }
    else if(msgbody.compare("OK")==0)
    {
        qDebug() << "OK receive";
        return;
    }
    else
    {
        qDebug() << "Classifier error! ";
//        PLCInterface->Cam_Classify_wait = CAM_CLS_DONE;
//        PLCInterface->Cam_Classified_object = CAM_CLS_UNKNOWN;
        return;
    }

    if(deviceType == 0)
    {

        if(plcRegIdx != ERR_VALUE)
        {
            //checkCompressStepTimer->start();
        }
    }

    //PLCInterface->writeRequest(plcRegIdx, 1);

}

void PLCApplication::compressStepError()
{
    //send plc to error
    qLog(LOGLEVEL_VERY_HIGH, "PLC", "Error", QString("compressStepError :  "));
    //PLCInterface->writeRequest(ERR_VALUE, 1);
    bErrorPLC = true;
    //send to ui for jam
    detectPlcFault(PS_JAM, 1);
}



//rohs edit
void PLCApplication::requestReceived(const QString& sender, const QString& message)
{
    QString result = "OK";
    qDebug() << "request Arrived : From " << sender;
    qDebug() << "request Arrived : Message >> " << message;

    //PLC Command ^^
    qLog(LOGLEVEL_VERY_HIGH, "PLC", "Info", QString("Request Received :  %1 from  %2").arg(message).arg(sender));

    if(message.compare(PLCMD_START)==0)
    {
        qDebug() << "rohs : PLCMD_START ";
        //PLCInterface->writeRequest(PAC_START, 1); //rohs edit
        client->sendRequest("Classifier", "InitCamera");

        ATM.CrusherMotor_Forward();
        ATM.LED_On();
        ATM.CAN_PET_Action();
        //responseReceived("test0","CAN"); force input test

        crusher_motor_stop_timer_flag = 0;
        crusher_motor_stop_timer_count = 0;

        qDebug() << "flag on";


    }
    else if(message.compare(PLCMD_STOP)==0)
    {
        qDebug() << "rohs : PLCMD_STOP ";
        //stopStart = true;
        //gotoReadyMode();
        //PLCInterface->writeRequest(PAC_START, 0);
        client->sendRequest("Classifier", "CloseCamera");
        result = "SOK";

        ATM.LED_Off();
        ATM.Conveyor_Stop();
        //ATM.Sorter_Center();
        ATM.CAN_PET_Action_off();
        //ATM.CrusherMotor_Stop(); //bug fixed
        crusher_motor_stop_timer_flag = 1;

        //rohs edit 20180225
        ATM.Get_Sensor_Update();
        QThread::msleep(200);

        int atm_sensor_state_filter = atm_sensor_state & 0x0000000f;
        detectPlcFault(PS_STORAGE_FULL,atm_sensor_state_filter);
        qDebug() << "pet/can full state check : " << atm_sensor_state_filter;


        qDebug() << "hall sensor" << hall_sensor_state_L << hall_sensor_state_R;
        if((hall_sensor_state_L == 1) && (hall_sensor_state_R == 1))
        {
            qDebug() << "Sorter JAM solved";
            bErrorPLC = false;
            detectPlcFault(PS_JAM, 0);
        }
        else
        {
            qDebug() << "Sorter JAM!!!";
            bErrorPLC = true;
            detectPlcFault(PS_JAM, 1);
        }


    }
    else if(message.compare(PLCMD_CHECKSTART)==0)
    {
       //PLCInterface->writeRequest((deviceType)?SELF_GTEST_START:SELF_TEST_START, 1);
        ATM.MasterReset();
        QThread::msleep(200);
        ATM.LED_On();
        sensor_update_timer_flag = 1;
        crusher_motor_stop_timer_flag = 0;
        crusher_motor_stop_timer_count = 0;

    }
    else if(message.compare(PLCMD_CHECKSTOP)==0)
    {
        bErrorPLC = false;
        ATM.LED_Off();
        sensor_update_timer_flag = 0;
        crusher_motor_stop_timer_flag = 0;
        crusher_motor_stop_timer_count = 0;
        ATM.MasterReset();
        QThread::msleep(300);
        ATM.Sorter_Center();
        QThread::msleep(3000);

        ATM.Get_Sensor_Update();
        QThread::msleep(20);
        qDebug() << "hall sensor" << hall_sensor_state_L << hall_sensor_state_R;
        if((hall_sensor_state_L == 1) && (hall_sensor_state_R == 1))
        {
            qDebug() << "Sorter JAM solved";
            bErrorPLC = false;
            detectPlcFault(PS_JAM, 0);
        }


        //rohs edit
        ATM.Get_Sensor_Update();
        QThread::msleep(100);
        qDebug() << "rohs : PLCMD_CHECKSTOP";

        //rohs edit 20180225, storage state signal tx@CHECKSTOP
        int atm_sensor_state_filter = atm_sensor_state & 0x0000000f;
        detectPlcFault(PS_STORAGE_FULL,atm_sensor_state_filter);

        result = QString::number(atm_sensor_state);
    }

    //rohs edit : 20180204
    else if(message.compare(PLCMD_CONVEYOR_LONG_ON)==0)
    {
        qDebug() << "rohs : Long-term test for conveyor part ON";
        //ATM.LongTermTest_Conv_On();
        long_term_test_timer_conveyor_flag = 1;

    }
    else if(message.compare(PLCMD_CONVEYOR_LONG_OFF)==0)
    {
        qDebug() << "rohs : Long-term test for conveyor part OFF";
        //ATM.LongTermTest_Conv_Off();
        long_term_test_timer_conveyor_flag = 0;
    }
    else if(message.compare(PLCMD_CRUSHER_LONG_ON)==0)
    {
        qDebug() << "rohs : Long-term test for crusher motor ON";
        //ATM.LongTermTest_Crusher_On();
        long_term_test_timer_crusher_flag = 1;
    }
    else if(message.compare(PLCMD_CRUSHER_LONG_OFF)==0)
    {
        qDebug() << "rohs : Long-term test for crusher motor OFF";
        //ATM.LongTermTest_Crusher_Off();
        long_term_test_timer_crusher_flag = 0;
    }
    else if(message.compare(PLCMD_SORTER_LONG_ON)==0)
    {
        qDebug() << "rohs : Long-term test for sorter part ON";
        //ATM.LongTermTest_Sorter_On();
        long_term_test_timer_sorter_flag = 1;
    }
    else if(message.compare(PLCMD_SORTER_LONG_OFF)==0)
    {
        qDebug() << "rohs : Long-term test for sorter part OFF";
        //ATM.LongTermTest_Sorter_Off();
        long_term_test_timer_sorter_flag = 0;
    }


    else if(message.compare(PLCMD_STEPPING_F)==0)
    {
        //PLCInterface->writeRequest((deviceType)? SELF_GTEST_STEPPING_F:SELF_TEST_STEPPING_F, 1);
        ATM.Conveyor_Front();
        QThread::msleep(3000);
        ATM.Conveyor_Stop();
    }
    else if(message.compare(PLCMD_STEPPING_R)==0)
    {
        //PLCInterface->writeRequest((deviceType)?SELF_GTEST_STEPPING_R:SELF_TEST_STEPPING_R, 1);
        ATM.Conveyor_Back();
        QThread::msleep(3000);
        ATM.Conveyor_Stop();
    }
    else if(message.compare(PLCMD_SORTER_LEFT)==0)
    {
        //PLCInterface->writeRequest((deviceType)?SELF_GTEST_SORTER_UD:SELF_TEST_SORTER_L, 1);
        ATM.Sorter_Left();
    }
    else if(message.compare(PLCMD_SORTER_RIGHT)==0)
    {
        //PLCInterface->writeRequest(SELF_TEST_SORTER_R, 1);
        ATM.Sorter_Right();
    }
    else if(message.compare(PLCMD_SORTER_CENTER)==0)
    {
        //PLCInterface->writeRequest(SELF_TEST_SORTER_C, 1);
        ATM.Sorter_Center();
     }
    else if(message.compare(PLCMD_GEARED_F)==0)
    {
        //PLCInterface->writeRequest((deviceType)?SELF_GTEST_GEARED_F:SELF_TEST_GEARED_F, 1);
        ATM.CrusherMotor_Forward();
        QThread::msleep(5000);
        ATM.CrusherMotor_Stop_Now(1);

    }
    else if(message.compare(PLCMD_GEARED_R)==0)
    {
        //PLCInterface->writeRequest((deviceType)?SELF_GTEST_GEARED_R:SELF_TEST_GEARED_R, 1);
        ATM.CrusherMotor_Reverse();
        QThread::msleep(5000);
        ATM.CrusherMotor_Stop_Now(1);
    }
    else if(message.compare(PLCMD_GEARED_S)==0)
    {
        qLog(LOGLEVEL_VERY_HIGH, "PLC", "Info", QString("request Geared motor stop"));
        if(deviceType==0)
        {
            //PLCInterface->writeRequest(GEARED_MOTOR_START, 0);
            //ATM.CrusherMotor_Stop_Now();
            crusher_motor_stop_timer_flag = 1;
        }
    }
    else if(message.compare(PLCMD_QUERY_STATE)==0)
    {
        qDebug() << "rohs : PLCMD_QUERY_STATE ";
        //result = QString::number(PLCInterface->getState());

        ATM.Get_Sensor_Update();
        QThread::msleep(100);
        result = QString::number(atm_sensor_state);
        qDebug() << "sensor state" << result << "number : " << atm_sensor_state;
        //ATM.Sorter_Init();

//        //here
//        int atm_sensor_state_filter = atm_sensor_state & 0x0000000f;
//        detectPlcFault(PS_STORAGE_FULL,atm_sensor_state_filter);

//        qDebug() << "hall sensor" << hall_sensor_state_L << hall_sensor_state_R;
//        if((hall_sensor_state_L == 1) && (hall_sensor_state_R == 1))
//        {
//            qDebug() << "Sorter JAM solved";
//            bErrorPLC = false;
//            detectPlcFault(PS_JAM, 0);
//        }
//        else
//        {
//            qDebug() << "Sorter JAM!!!";
//            bErrorPLC = true;
//            detectPlcFault(PS_JAM, 1);
//        }



    }

    //qLog(LOGLEVEL_VERY_HIGH, "PLC", "Info", QString("Response send :  %1 to  %2").arg(result).arg(sender));

    worker->replyRequest(sender, result);

}

void PLCApplication::onConnect(bool bConnected)
{
//    qDebug() << "PLC Connected -->  " << bConnected;
//    bPLCOnline = bConnected;
//    if(bPLCOnline)
//    {

//    }


}




void PLCApplication::stateChanged(uint nState)
{
    QByteArray cmdData;
    cmdData.clear();
    cmdData.append(QChar(REPORT_PLCSTATE));
    QByteArray data ;
    data.clear();
    data.append(QChar(nState));


    if(bErrorPLC)
    {
        //qLog(LOGLEVEL_VERY_HIGH, "PLC", "Info", QString("Publish aborted because an error :  (%1) ").arg(nState));
        //return;
    }

    reporter->sendMessage(cmdData, data);

    qDebug() << "state changed : " << nState;


    if(nState == PS_MOVINGCLSPOINT)
    {
        qDebug() << "send classify message to classifier";
        qLog(LOGLEVEL_VERY_HIGH,"PLC", "Info", "send classify message to classifier");

        client->sendRequest("Classifier", "Classifier"); //rohs : cmd to camera classifier
    }
    else if(nState == PS_GETREADY)
    {

        //qDebug() << "Send Init camera to classifier";
        qLog(LOGLEVEL_VERY_HIGH,"PLC", "Info", "Send Init camera to classifier");
        //client->sendRequest("Classifier", "InitCamera");
        //*
        if(latestType != ERR_VALUE)
        {
            QByteArray cmdData,data;
            data.clear();
            cmdData.clear();
            cmdData.append(QChar(REPORT_PLCSTATE));
            data.append(PS_RECEIVECLS);
            data.append(QChar(latestType));
            latestType = ERR_VALUE;
            reporter->sendMessage(cmdData, data);
        }
        //*/
    }
    else if(nState == PS_RECYCLINGEND)
    {
        if(deviceType == 0)
        {
            checkCompressStepTimer->stop();
        }
        if(latestType != ERR_VALUE)
        {
            QByteArray cmdData,data;
            data.clear();
            cmdData.clear();
            cmdData.append(QChar(REPORT_PLCSTATE));
            data.append(PS_RECEIVECLS);
            data.append(QChar(latestType));
            latestType = ERR_VALUE;

            reporter->sendMessage(cmdData, data);
        }

    }

}

void PLCApplication::detectPlcFault(uint nError, char cParam)
{
    QByteArray cmdData;
    QByteArray data;
    cmdData.clear();
    cmdData.append(QChar(REPORT_ERROR));
    data.clear();
    data.append(QChar(nError));
    data.append(cParam);
    qLog(LOGLEVEL_VERY_HIGH, "PLC", "ERROR", QString("Publish send :  (%1) : %2 with param (%3)").arg(cmdData[0]).arg(nError).arg(int(cParam)));

    reporter->sendMessage(cmdData, data);
}


void PLCApplication::reportError(uint nError)
{



}





void PLCApplication::input()
{
    qDebug() << "PLC Can Iputed  --> move tray  ";
    if(stopStart)
    {
        stopStart =false;
        return;
    }

    QByteArray cmdData;
    cmdData.append(QChar(REPORT_PLCSTATE));
    QByteArray data ;
    data.append(QChar(PS_GETREADY));
    reporter->sendMessage(cmdData, data);

    QTimer::singleShot(1000, this, SLOT(toEndPoint()));

}

bool gCan = false;
uint uGlass = CLS_REUSE20;

void PLCApplication::toEndPoint()
{
    qDebug() << "PLC End point request Classify";

    QByteArray cmdData;
    QByteArray data ;
    data += PS_RECEIVECLS;
 #if 0
    if(gCan)
    {
        data += QChar(CLS_CAN);
        gCan = false;
    }
    else
    {
        data += CLS_PET;
        gCan = true;
    }
 #else
    if(uGlass > CLS_REUSE100_N)
    {
        uGlass = CLS_REUSE20;
    }
    data += uGlass;
    uGlass++;
 #endif
    cmdData.append(QChar(REPORT_PLCSTATE));
    reporter->sendMessage(cmdData, data);
    QTimer::singleShot(1000, this, SLOT(outputEnd()));
    //reporter->sendMessage(REREUSE40PORT_PLCSTATE, "RC");
    //client->sendRequest("Classifier", "Classifier");
}

void PLCApplication::outputStart()
{
    //publish output state
    //reporter->sendMessage(REPORT_PLCSTATE, "OS");
     qDebug() << "PLC Output Start";
     QByteArray cmdData;
     cmdData.append(QChar(REPORT_PLCSTATE));
     QByteArray data ;
     //data.append(PSuint length,_SORTERMOVE);
     reporter->sendMessage(cmdData, data);

    QTimer::singleShot(1000, this, SLOT(outputEnd()));
}

void PLCApplication::outputEnd()
{
    //publish output state
    //reporter->sendMessage(REPORT_PLCSTATE, "OE");
    qDebug() << "PLC Output End";
    QByteArray cmdData;
    cmdData.append(QChar(REPORT_PLCSTATE));
    QByteArray data ;
    data.append(PS_RECYCLINGEND);
    reporter->sendMessage(cmdData, data);
    //QTimer::singleShot(1000, this, SLOT(gotoReadyMode()));
}


void PLCApplication::gotoReadyMode()
{
    qDebug() << "PLC Now in ready mode";
   // reporter->sendMessage(REPORT_PLCSTATE, "RM");
    //publish ready

}



