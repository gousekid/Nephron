#ifndef ATMEGA128APP_H
#define ATMEGA128APP_H

#include <QObject>
#include <QString>


class ATmega128app : public QObject
{

public:
    ATmega128app();
    ~ATmega128app();

    //init
    int Serial_Open(const char *port, unsigned int baud, int blocking); // return 0: error, return 1: success

    //base function
    void Serial_SendByte(unsigned char byte);
    //int  Serial_Read(unsigned char *buf, int size);
    void Serial_Read(unsigned char *buf, int size);
    int  Serial_Read_with_Counter(unsigned char *buf, int size);
    void Serial_String(char *);
    void Serial_String(const QString& );

    //cmd to devices
    void CrusherMotor_Forward(void);
    void CrusherMotor_Reverse(void);
    void CrusherMotor_Stop(void);
    void CrusherMotor_Stop_Now(int);

    void Sorter_Left(void);
    void Sorter_Right(void);
    void Sorter_Center(void);
    void Sorter_Init(void);
    void Sorter_Stop(void);

    void Conveyor_Stop(void);
    void Conveyor_Front(void);
    void Conveyor_Back(void);
    void Conveyor_Task_ON(void);
//    void Conveyor_Task_Off(void);
    void Conveyor_Task_Rev(void);
    void Conveyor_Continue_Task(void);

    void LED_On(void);
    void LED_Off(void);

    void CAN_PET_Action(void);
    void CAN_PET_Action_off(void);

    void LongTermTest_Conv_Off(void);
    void LongTermTest_Conv_On(void);
    void LongTermTest_Crusher_Off(void);
    void LongTermTest_Crusher_On(void);
    void LongTermTest_Sorter_Off(void);
    void LongTermTest_Sorter_On(void);

    //sensor state reading
    void Get_Sensor_Update(void);
//    void Get_Sensor_State(void);

    void MasterReset(void);

    //void Wait_for_Obj_Delivery(void);

    int fd_;
    int hall_l, hall_r;

    int conveyor_stop_ack=0;
    int conveyor_cw_ack=0;
    int conveyor_ccw_ack=0;
    int conveyor_task_on_ack=0;
    int conveyor_task_off_ack=0;
    int conveyor_task_rev_ack=0;
    int conveyor_task_cont_ack=0;

    int crusher_stop_ack=0;
    int crusher_fx_ack=0;
    int crusher_rx_ack=0;
    int crusher_emstop_ack=0;

    int sorter_init_ack=0;
    int sorter_left_ack=0;
    int sorter_right_ack=0;
    int sorter_center_ack=0;
    int sorter_stop_ack=0;

    int LED_On_ack=0;
    int LED_Off_ack=0;

    int can_pet_action_on_ack=0;
    int can_pet_action_off_ack=0;

    //20180310 long term test
    int long_term_test_conv_on_ack=0;
    int long_term_test_conv_off_ack=0;

    int long_term_test_crusher_on_ack=0;
    int long_term_test_crusher_off_ack=0;

    int long_term_test_sorter_on_ack=0;
    int long_term_test_sorter_off_ack=0;

};

#endif // ATMEGA128APP_H

