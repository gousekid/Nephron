#ifndef PLCAPPLICATION_H
#define PLCAPPLICATION_H

#include <QObject>
#include <QCoreApplication>
#include "worker.h"
#include "client.h"
#include "zmsg.h"
#include "plctask.h"
#include "publisher.h"
#include "subscriber.h"
//#include "modbusinterface.h"
//#include "pcieio.h"
#include <QTimer>

//rohs edit
#include "atmega128app.h"



class PLCApplication : public QCoreApplication
{
    Q_OBJECT

    typedef QCoreApplication super;

public:
    explicit PLCApplication(int& argc, char** argv);

    bool notify(QObject *obj, QEvent *event);
    ~PLCApplication();

    //rohs edit
    int Serial_Open(const char *port, int baud, int blocking);
    int Serial_SendByte(int fd, unsigned char byte);
    ATmega128app ATM;
    int atm_sensor_state;
    int door_bottom_state=0;
    int door_top_state=0;
    int crusher_motor_stop_timer_flag = 0;
    int crusher_motor_stop_timer_count=0;
    int sensor_update_timer_flag=0;
    int hall_sensor_state_L = 0;
    int hall_sensor_state_R = 0;

    //20180311 long term test
    int long_term_test_timer_conveyor_flag  = -1;
    int long_term_test_timer_crusher_flag   = -1;
    int long_term_test_timer_sorter_flag    = -1;

    int long_term_test_counter_conveyor = 0;
    int long_term_test_counterMAX_conveyor = 0;
    int long_term_test_counter_crusher  = 0;
    int long_term_test_counterMAX_crusher  = 0;
    int long_term_test_counter_sorter   = 0;
    int long_term_test_counterMAX_sorter   = 0;

    //20180411
    int full_check_sensor_counter = 0;

    bool    bErrorPLC;



//    void run();


signals:
    void finished();
    void failure(const QString& what);


public slots:
    void run();
    void crusher_motor_stop_timer();
    void sensor_update_timer();
    //long term test code
    void long_term_test_timer_conveyor();
    void long_term_test_timer_crusher();
    void long_term_test_timer_sorter();

    //100% sensor check
    void full_check_sensor_update();

    void requestReceived(const QString& sender, const QString& message);
    void onConnect(bool bConnected);
    void responseReceived(const QString& service, const QString& msgbody);
    void receivedMessage(const QList<QByteArray>& message);

    void stateChanged(uint nState);
    void reportError(uint nError);
    void detectPlcFault(uint nError, char cParam);

    void compressStepError();

    void messageSent(const QList<QByteArray>&  message);

    void plcSensorStateChange(int index, bool value);

    //==================temporary Code
    void input();
    void toEndPoint();
    void outputStart();
    void outputEnd();
    void gotoReadyMode();
    void tmpSlot();



protected:


public:
    ZMQContext* context;
    Worker* worker;
    Client * client;
    //PLCTask* PLCInterface;
    //pcieIO* PLCInterface;


    Publisher* reporter;
    Subscriber* listener;
    bool stopStart;

    bool bPLCOnline;

    QTimer* checkCompressStepTimer;

    uint    latestType;

    int     deviceType;

    QTimer* Check_cmd;


};

#endif // PLCAPPLICATION_H
