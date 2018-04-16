#include "atmega128app.h"
#include "global.hpp"
#include <QThread>
#include <QTimer>
#include <QDebug>

#include <sys/time.h>

//for serial comm
#include <linux/serial.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/file.h>

#define ACK_MSG_TIMEOUT 5000


ATmega128app::ATmega128app()
{

    fd_ = -1;

}


ATmega128app::~ATmega128app()
{

}


int ATmega128app::Serial_Open(const char *port, unsigned int baud, int blocking)
{
    int fd=-1;
    struct termios opts;
    int flags = O_RDWR | O_NOCTTY;
    if (!blocking)
        flags |= O_NONBLOCK;

    fd=open(port, flags, 0);

    if (fd==-1)
        return 0; //-1

    if (tcgetattr(fd, &opts))
    {
        //printf("*** %i\n",fd);
        //perror("tcgetattr");
        return 0; //-1
    }
    cfsetispeed(&opts, B57600);
    cfsetospeed(&opts, B57600);
    cfmakeraw(&opts);
//    opts.c_cflag &= ~CSTOPB;



    opts.c_cflag = (opts.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
    opts.c_iflag &= ~IGNBRK;         // disable break processing
    opts.c_lflag = 0;                // no signaling chars, no echo, // no canonical processing
    opts.c_oflag = 0;                // no remapping, no delays
    opts.c_cc[VMIN]  = 0;            // read doesn't block
    opts.c_cc[VTIME] = 5;            // 0.5 seconds read timeout
    opts.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl
    opts.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
    opts.c_cflag &= ~(PARENB | PARODD);      // shut off parity
    opts.c_cflag |= 0;
    opts.c_cflag &= ~CSTOPB;
    opts.c_cflag &= ~CRTSCTS;

    if (tcsetattr(fd,TCSANOW,&opts))
    {
        //perror("tcsetattr");
        return 0; //-1
    }

    tcflush(fd, TCIOFLUSH);
    //return fd;
    fd_ = fd;


    return 1;
}




void ATmega128app::Serial_SendByte(unsigned char byte)
{
  int n = write(fd_, &byte, 1);
  if(n < 0)
  {
    if(errno == EAGAIN)
    {
      //return 0;
        return;
    }
    else
    {
      //return 1;
        return;
    }
  }
  QThread::msleep(1);
  return;
}

void ATmega128app::Serial_Read(unsigned char *buf, int size)
{
  int n=0;

  for(;;)
  {
      QThread::usleep(1);
      n = read(fd_, buf, size);
      if(n!=-1) return;
  }
}

int ATmega128app::Serial_Read_with_Counter(unsigned char *buf, int size)
{
  int n=0;
  int counter=0;
  for(;;)
  {
      QThread::usleep(1);
      n = read(fd_, buf, size);
      if(n!=-1) return -1;
      counter++;

      if(counter > 500) return 0;
  }
}


void ATmega128app::Serial_String(char *_str)
{
    int index=0;
    Serial_SendByte(0x18); //LCD clear
    while(_str[index] != 0)
    {
        Serial_SendByte(_str[index]);
        QThread::msleep(1);
        index++;
    }
    Serial_SendByte(0x0a);
}


void ATmega128app::Serial_String(const QString& _str)
{
    QByteArray ba = _str.toLatin1();
    char* c_str = ba.data();
    Serial_String(c_str);
}



void ATmega128app::CrusherMotor_Stop(void) //20sec
{

    RE_TRANSMISSION_CMS:
    long counter=0;
    crusher_stop_ack = 1;

    Serial_SendByte('c');
    Serial_SendByte('0');
    Serial_SendByte('0');

    //ack message check
    for(;;)
    {
        QThread::usleep(1);
        counter++;
        if(counter > ACK_MSG_TIMEOUT)
        {
            qDebug() << "CMS ack message timeout";
            goto RE_TRANSMISSION_CMS;
        }
        if(crusher_stop_ack == 0)
        {
            qDebug() << "CMS ack message received @" << counter;
            break;
        }
    }

}

void ATmega128app::CrusherMotor_Forward(void)
{

    RE_TRANSMISSION_CMF:
    long counter=0;
    crusher_fx_ack = 1;

    Serial_SendByte('c');
    Serial_SendByte('0');
    Serial_SendByte('1');

    //ack message check
    for(;;)
    {
        QThread::usleep(1);
        counter++;
        if(counter > ACK_MSG_TIMEOUT)
        {
            qDebug() << "CMF ack message timeout";
            goto RE_TRANSMISSION_CMF;
        }
        if(crusher_fx_ack == 0)
        {
            qDebug() << "CMR ack message received @" << counter;
            break;
        }
    }
}

void ATmega128app::CrusherMotor_Reverse(void)
{

    RE_TRANSMISSION_CMR:
    long counter=0;
    crusher_rx_ack = 1;

    Serial_SendByte('c');
    Serial_SendByte('0');
    Serial_SendByte('2');

    //ack message check
    for(;;)
    {
        QThread::usleep(1);
        counter++;
        if(counter > ACK_MSG_TIMEOUT)
        {
            qDebug() << "CMR ack message timeout";
            goto RE_TRANSMISSION_CMR;
        }
        if(crusher_rx_ack == 0)
        {
            qDebug() << "CMR ack message received @" << counter;
            break;
        }
    }

}


void ATmega128app::CrusherMotor_Stop_Now(int loop)
{
    RE_TRANSMISSION_CMSN:
    long counter=0;
    crusher_emstop_ack = 1;

    Serial_SendByte('c');
    Serial_SendByte('0');
    Serial_SendByte('3');

    //ack message check
    while(loop)
    {
        QThread::usleep(1);
        counter++;
        if(counter > ACK_MSG_TIMEOUT)
        {
            qDebug() << "CMSN ack message timeout";
            goto RE_TRANSMISSION_CMSN;
        }
        if(crusher_emstop_ack == 0)
        {
            qDebug() << "CMSN ack message received @" << counter;
            break;
        }
    }

}



void ATmega128app::Sorter_Init(void)
{

    RE_TRANSMISSION_SOI:
    long counter=0;
    sorter_init_ack = 1;

    Serial_SendByte('s');
    Serial_SendByte('0');
    Serial_SendByte('0');

    //ack message check
    for(;;)
    {
        QThread::usleep(1);
        counter++;
        if(counter > ACK_MSG_TIMEOUT)
        {
            qDebug() << "SOI ack message timeout";
            goto RE_TRANSMISSION_SOI;
        }
        if(sorter_init_ack == 0)
        {
            qDebug() << "SOI ack message received @" << counter;
            break;
        }
    }

}

void ATmega128app::Sorter_Left(void)
{

    RE_TRANSMISSION_SOL:
    long counter=0;
    sorter_left_ack = 1;

    Serial_SendByte('s');
    Serial_SendByte('0');
    Serial_SendByte('1');

    //ack message check
    for(;;)
    {
        QThread::usleep(1);
        counter++;
        if(counter > ACK_MSG_TIMEOUT)
        {
            qDebug() << "SOL ack message timeout";
            goto RE_TRANSMISSION_SOL;
        }
        if(sorter_left_ack == 0)
        {
            qDebug() << "SOL ack message received @" << counter;
            break;
        }
    }

}

void ATmega128app::Sorter_Right(void)
{
    RE_TRANSMISSION_SOR:
    long counter=0;
    sorter_right_ack = 1;

    Serial_SendByte('s');
    Serial_SendByte('0');
    Serial_SendByte('2');

    //ack message check
    for(;;)
    {
        QThread::usleep(1);
        counter++;
        if(counter > ACK_MSG_TIMEOUT)
        {
            qDebug() << "SOR ack message timeout";
            goto RE_TRANSMISSION_SOR;
        }
        if(sorter_right_ack == 0)
        {
            qDebug() << "SOR ack message received @" << counter;
            break;
        }
    }

}

void ATmega128app::Sorter_Center(void)
{
    RE_TRANSMISSION_SOC:
    long counter=0;
    sorter_center_ack = 1;


    Serial_SendByte('s');
    Serial_SendByte('0');
    Serial_SendByte('3');

    //ack message check
    for(;;)
    {
        QThread::usleep(1);
        counter++;
        if(counter > ACK_MSG_TIMEOUT)
        {
            qDebug() << "SOC ack message timeout";
            goto RE_TRANSMISSION_SOC;
        }
        if(sorter_center_ack == 0)
        {
            qDebug() << "SOC ack message received @" << counter;
            break;
        }
    }
}

void ATmega128app::Sorter_Stop(void)
{
    RE_TRANSMISSION_SOS:
    long counter=0;
    sorter_stop_ack = 1;

    Serial_SendByte('s');
    Serial_SendByte('0');
    Serial_SendByte('4');

    //ack message check
    for(;;)
    {
        QThread::usleep(1);
        counter++;
        if(counter > ACK_MSG_TIMEOUT)
        {
            qDebug() << "SOS ack message timeout";
            goto RE_TRANSMISSION_SOS;
        }
        if(sorter_stop_ack == 0)
        {
            qDebug() << "SOS ack message received @" << counter;
            break;
        }
    }

}


void ATmega128app::Conveyor_Stop(void)
{
    RE_TRANSMISSION_CVS:
    long counter=0;
    conveyor_stop_ack = 1;

    Serial_SendByte('v');
    Serial_SendByte('0');
    Serial_SendByte('0');

    //ack message check
    for(;;)
    {
        QThread::usleep(1);
        counter++;
        if(counter > ACK_MSG_TIMEOUT)
        {
            qDebug() << "CVS ack message timeout";
            goto RE_TRANSMISSION_CVS;
        }
        if(conveyor_stop_ack == 0)
        {
            qDebug() << "CVS ack message received @" << counter;
            break;
        }
    }

}

void ATmega128app::Conveyor_Front(void)
{
    RE_TRANSMISSION_CVF:
    long counter=0;
    conveyor_cw_ack = 1;

    Serial_SendByte('v');
    Serial_SendByte('0');
    Serial_SendByte('1');

    //ack message check
    for(;;)
    {
        QThread::usleep(1);
        counter++;
        if(counter > ACK_MSG_TIMEOUT)
        {
            qDebug() << "CVF ack message timeout";
            goto RE_TRANSMISSION_CVF;
        }
        if(conveyor_cw_ack == 0)
        {
            qDebug() << "CVF ack message received @" << counter;
            break;
        }
    }
}

void ATmega128app::Conveyor_Back(void)
{
    RE_TRANSMISSION_CVB:
    long counter=0;
    conveyor_ccw_ack = 1;

    Serial_SendByte('v');
    Serial_SendByte('0');
    Serial_SendByte('2');

    //ack message check
    for(;;)
    {
        QThread::usleep(1);
        counter++;
        if(counter > ACK_MSG_TIMEOUT)
        {
            qDebug() << "CVB ack message timeout";
            goto RE_TRANSMISSION_CVB;
        }
        if(conveyor_ccw_ack == 0)
        {
            qDebug() << "CVB ack message received @" << counter;
            break;
        }
    }

}

void ATmega128app::Conveyor_Task_ON(void)
{
    RE_TRANSMISSION_CTO:
    long counter=0;
    conveyor_task_on_ack = 1;

    Serial_SendByte('v');
    Serial_SendByte('0');
    Serial_SendByte('3');

    //ack message check
    for(;;)
    {
        QThread::usleep(1);
        counter++;
        if(counter > ACK_MSG_TIMEOUT)
        {
            qDebug() << "CTO ack message timeout";
            goto RE_TRANSMISSION_CTO;
        }
        if(conveyor_task_on_ack == 0)
        {
            qDebug() << "CTO ack message received @" << counter;
            break;
        }
    }

}



void ATmega128app::Conveyor_Task_Rev(void)
{
    RE_TRANSMISSION_CTR:
    long counter=0;
    conveyor_task_rev_ack = 1;

    Serial_SendByte('v');
    Serial_SendByte('0');
    Serial_SendByte('5');

    //ack message check
    for(;;)
    {
        QThread::usleep(1);
        counter++;
        if(counter > ACK_MSG_TIMEOUT)
        {
            qDebug() << "CTR ack message timeout";
            goto RE_TRANSMISSION_CTR;
        }
        if(conveyor_task_rev_ack == 0)
        {
            qDebug() << "CTR ack message received @" << counter;
            break;
        }
    }
}


void ATmega128app::Conveyor_Continue_Task(void)
{
    RE_TRANSMISSION_CCT:
    long counter=0;
    conveyor_task_cont_ack = 1;

    Serial_SendByte('v');
    Serial_SendByte('0');
    Serial_SendByte('6');

    //ack message check
    for(;;)
    {
        QThread::usleep(1);
        counter++;
        if(counter > ACK_MSG_TIMEOUT)
        {
            qDebug() << "CCT ack message timeout";
            goto RE_TRANSMISSION_CCT;
        }
        if(conveyor_task_cont_ack == 0)
        {
            qDebug() << "CCT ack message received @" << counter;
            break;
        }
    }

}



void ATmega128app::LED_On(void)
{
    RE_TRANSMISSION_LED_ON:
    long counter=0;
    LED_On_ack = 1;

    Serial_SendByte('l');
    Serial_SendByte('0');
    Serial_SendByte('1');

    //ack message check
    for(;;)
    {
        QThread::usleep(1);
        counter++;
        if(counter > ACK_MSG_TIMEOUT)
        {
            qDebug() << "LON ack message timeout";
            goto RE_TRANSMISSION_LED_ON;
        }
        if(LED_On_ack == 0)
        {
            qDebug() << "LON ack message received @" << counter;
            break;
        }
    }
}


void ATmega128app::LED_Off(void)
{
    RE_TRANSMISSION_LED_OFF:
    int counter=0;
    LED_Off_ack = 1;

    Serial_SendByte('l');
    Serial_SendByte('0');
    Serial_SendByte('0');

    //ack message check
    for(;;)
    {
        QThread::usleep(1);
        counter++;
        if(counter > ACK_MSG_TIMEOUT)
        {
            qDebug() << "LOFF ack message timeout";
            goto RE_TRANSMISSION_LED_OFF;
        }
        if(LED_Off_ack == 0)
        {
            qDebug() << "LOFF ack message received @" << counter;
            break;
        }
    }

}

void ATmega128app::CAN_PET_Action(void)
{
    RE_TRANSMISSION_CAN_PET_ACTION:
    int counter=0;
    can_pet_action_on_ack = 1;

    Serial_SendByte('a');
    Serial_SendByte('0');
    Serial_SendByte('0');

    //ack message check
    for(;;)
    {
        QThread::usleep(1);
        counter++;
        if(counter > ACK_MSG_TIMEOUT)
        {
            qDebug() << "CPA ack message timeout";
            goto RE_TRANSMISSION_CAN_PET_ACTION;
        }
        if(can_pet_action_on_ack == 0)
        {
            qDebug() << "CPA ack message received @" << counter;
            break;
        }
    }
}

void ATmega128app::CAN_PET_Action_off(void)
{
    RE_TRANSMISSION_CAN_PET_ACTION_OFF:
    int counter=0;
    can_pet_action_off_ack = 1;

    Serial_SendByte('a');
    Serial_SendByte('0');
    Serial_SendByte('1');

    //ack message check
    for(;;)
    {
        QThread::usleep(1);
        counter++;
        if(counter > ACK_MSG_TIMEOUT)
        {
            qDebug() << "CPAO ack message timeout";
            goto RE_TRANSMISSION_CAN_PET_ACTION_OFF;
        }
        if(can_pet_action_off_ack == 0)
        {
            qDebug() << "CPAO ack message received @" << counter;
            break;
        }
    }
}


/*
//20180310 long term test
void ATmega128app::LongTermTest_Conv_Off(void)
{
    RE_TRANSMISSION_LONGTERMTTEST_CONV_OFF:
    int counter=0;
    long_term_test_conv_off_ack = 1;

    Serial_SendByte('t');
    Serial_SendByte('0');
    Serial_SendByte('0');

    //ack message check
    for(;;)
    {
        QThread::usleep(1);
        counter++;
        if(counter > ACK_MSG_TIMEOUT)
        {
            qDebug() << "LTVOFF ack message timeout";
            goto RE_TRANSMISSION_LONGTERMTTEST_CONV_OFF;
        }
        if(long_term_test_conv_off_ack == 0)
        {
            qDebug() << "LTVOFF ack message received @" << counter;
            break;
        }
    }
}


void ATmega128app::LongTermTest_Conv_On(void)
{
    RE_TRANSMISSION_LONGTERMTTEST_CONV_ON:
    int counter=0;
    long_term_test_conv_on_ack = 1;

    Serial_SendByte('t');
    Serial_SendByte('0');
    Serial_SendByte('1');

    //ack message check
    for(;;)
    {
        QThread::usleep(1);
        counter++;
        if(counter > ACK_MSG_TIMEOUT)
        {
            qDebug() << "LTVON ack message timeout";
            goto RE_TRANSMISSION_LONGTERMTTEST_CONV_ON;
        }
        if(long_term_test_conv_on_ack == 0)
        {
            qDebug() << "LTVON ack message received @" << counter;
            break;
        }
    }
}



void ATmega128app::LongTermTest_Crusher_Off(void)
{
    RE_TRANSMISSION_LONGTERMTTEST_CRUSHER_OFF:
    int counter=0;
    long_term_test_crusher_off_ack = 1;

    Serial_SendByte('t');
    Serial_SendByte('0');
    Serial_SendByte('2');

    //ack message check
    for(;;)
    {
        QThread::usleep(1);
        counter++;
        if(counter > ACK_MSG_TIMEOUT)
        {
            qDebug() << "LTCOFF ack message timeout";
            goto RE_TRANSMISSION_LONGTERMTTEST_CRUSHER_OFF;
        }
        if(long_term_test_crusher_off_ack == 0)
        {
            qDebug() << "LTCOFF ack message received @" << counter;
            break;
        }
    }
}


void ATmega128app::LongTermTest_Crusher_On(void)
{
    RE_TRANSMISSION_LONGTERMTTEST_CRUSHER_ON:
    int counter=0;
    long_term_test_crusher_on_ack = 1;

    Serial_SendByte('t');
    Serial_SendByte('0');
    Serial_SendByte('3');

    //ack message check
    for(;;)
    {
        QThread::usleep(1);
        counter++;
        if(counter > ACK_MSG_TIMEOUT)
        {
            qDebug() << "LTCON ack message timeout";
            goto RE_TRANSMISSION_LONGTERMTTEST_CRUSHER_ON;
        }
        if(long_term_test_crusher_on_ack == 0)
        {
            qDebug() << "LTCON ack message received @" << counter;
            break;
        }
    }
}


void ATmega128app::LongTermTest_Sorter_Off(void)
{
    RE_TRANSMISSION_LONGTERMTTEST_SORTER_OFF:
    int counter=0;
    long_term_test_sorter_off_ack = 1;

    Serial_SendByte('t');
    Serial_SendByte('0');
    Serial_SendByte('4');

    //ack message check
    for(;;)
    {
        QThread::usleep(1);
        counter++;
        if(counter > ACK_MSG_TIMEOUT)
        {
            qDebug() << "LTSOFF ack message timeout";
            goto RE_TRANSMISSION_LONGTERMTTEST_SORTER_OFF;
        }
        if(long_term_test_sorter_off_ack == 0)
        {
            qDebug() << "LTSOFF ack message received @" << counter;
            break;
        }
    }
}


void ATmega128app::LongTermTest_Sorter_On(void)
{
    RE_TRANSMISSION_LONGTERMTTEST_SORTER_ON:
    int counter=0;
    long_term_test_sorter_on_ack = 1;

    Serial_SendByte('t');
    Serial_SendByte('0');
    Serial_SendByte('5');

    //ack message check
    for(;;)
    {
        QThread::usleep(1);
        counter++;
        if(counter > ACK_MSG_TIMEOUT)
        {
            qDebug() << "LTSON ack message timeout";
            goto RE_TRANSMISSION_LONGTERMTTEST_SORTER_ON;
        }
        if(long_term_test_sorter_on_ack == 0)
        {
            qDebug() << "LTSON ack message received @" << counter;
            break;
        }
    }
}
*/


void ATmega128app::Get_Sensor_Update(void)
{
    Serial_SendByte('u');
    Serial_SendByte('0');
    Serial_SendByte('0');
}


void ATmega128app::MasterReset(void)
{
    Serial_SendByte('r');
    Serial_SendByte('0');
    Serial_SendByte('0');
    QThread::msleep(200);
}


