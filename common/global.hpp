// Copyright 2011-2014 Johann Duscher (a.k.a. Jonny Dee). All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
//
//    1. Redistributions of source code must retain the above copyright notice, this list of
//       conditions and the following disclaimer.
//
//    2. Redistributions in binary form must reproduce the above copyright notice, this list
//       of conditions and the following disclaimer in the documentation and/or other materials
//       provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY JOHANN DUSCHER ''AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
// FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// The views and conclusions contained in the software and documentation are those of the
// authors and should not be interpreted as representing official policies, either expressed
// or implied, of Johann Duscher.

#ifndef NZMQT_GLOBAL_HPP
#define NZMQT_GLOBAL_HPP

#include <QtCore/qglobal.h>

#define DEV_MODE

#define BROADCAST_IPC "ipc://nephron-broadcast"
#define REPORT_IPC_PLC "ipc://nephron-report-plc"
#define REPORT_IPC_CLS "ipc://nephron-report-cls"
#define REPORT_IPC_UI "ipc://nephron-report-ui"
#define REPORT_IPC_PRINTER "ipc://nephron-report-printer"

//#define NEURAL_IPC "ipc://nephron-neuralnetwork"
#define NEURAL_IPC "ipc://nephron-mdp" //tcp://localhost:5555"
#define NEURALBROKER_IPC "ipc://nephron-mdp" //tcp://*:5555"
#define HEARTBEAT_LIVENESS  3       //  3-5 is reasonable
#define HEARTBEAT_INTERVAL  2500    //  msecs
#define HEARTBEAT_EXPIRY    HEARTBEAT_INTERVAL * HEARTBEAT_LIVENESS

//  This is the version of MDP/Client we implement
#define MDPC_CLIENT         "MDPC0X"

//  MDP/Client commands, as strings
#define MDPC_REQUEST        "\001"
#define MDPC_REPORT         "\002"
#define MDPC_NAK            "\003"


#define MDPW_WORKER         "MDPW0X"

//  MDP/Worker commands, as strings
#define MDPW_READY          "\001"
#define MDPW_REQUEST        "\002"
#define MDPW_REPLY         "\003"
#define MDPW_HEARTBEAT      "\004"
#define MDPW_DISCONNECT     "\005"


#define CLS_CAN         0
#define CLS_PET         1
#define CLS_RECYLE      2
#define CLS_REUSE20     3
#define CLS_REUSE40     4
#define CLS_REUSE50     5
#define CLS_REUSE100     6
#define CLS_REUSE20_N     7
#define CLS_REUSE40_N     8
#define CLS_REUSE50_N     9
#define CLS_REUSE100_N     10

#define CLS_ETC         11


#define REPORT_PLCSTATE         0
#define REPORT_ERROR            1
#define REPORT_LOG              2
#define REPORT_INFO             3
#define REPORT_CLS              4
#define REPORT_REMOVEBOX        5
#define REPORT_SENSORSTATE      6
#define REPORT_PRINTSTATE       7

#define PUBLISH_PLCSTATE        0
#define PUBLISH_COMMAND         1
#define PUBLISH_BARCODE         2
#define PUBLISH_SENSORSTATE     3
#define PUBLISH_PRINTSTATE      4

#define PS_GETREADY             0
#define PS_DETECTOBJECT         1
#define PS_MOVINGCLSPOINT       2
#define PS_REQUESTCLS           3
#define PS_RECEIVECLS           4
#define PS_SORTERMOVE           5
#define PS_RECYCLINGPROCEED     6
#define PS_CHECKSTORAGE         7
#define PS_RECYCLINGEND         8
#define PS_DOOROPEN             9
#define PS_JAM                  10
#define PS_STORAGE_EMPTY        11
#define PS_STORAGE_FULL         12
#define PS_RETURNOBJECT         13
#define PS_CANCEL_SESSION       14
#define PS_RETURNOBJECT2         15

#define PRINTER_STATE_OK                0
#define PRINTER_STATE_EMPTY_PAPER       1
#define PRINTER_STATE_PAPER_JAM         2
#define PRINTER_STATE_DISCONNECT        3



#define NS_CONNECTED            0
#define NS_DISCONNECTED         1


#define LOGLEVEL_VERY_HIGH      0
#define LOGLEVEL_HIGH           1
#define LOGLEVEL_MODERATE       2
#define LOGLEVEL_LOW            3
#define LOGLEVEL_VERY_LOW       4

#define  PAC_START      0
#define  MACHINE_TYPE   1

#define  MOVING_STEP    8
#define  MOVE_STEP      9
#define  COMPRESS_STEP  10
#define  RETURN_STEP    11
#define  CANCEL_SESSIOIN    12
#define  RETURN_STEP2    24

#define  PET_VALUE      32
#define  CAN_VALUE      33
#define  ERR_VALUE      34

#define  GLASS_VALUE    32

#define  PET_FULL       16
#define  PET_80P        17
#define  CAN_FULL       18
#define  CAN_80P        19

#define  CAN_BIN_EMPTY  20
#define  PET_BIN_EMPTY  21

#define  GLASS_FULL         16
#define  GLASS_BIN_EMPTY    17
#define  GLASS_DOOR_TOP_S     18
#define  GLASS_DOOR_BOTTOM_S  19

#define  PLC_STARTIR    176
#define  PLC_ENDIR      180
#define  PLC_JAMSENSOR  60

#define PLC_SORTER_IDLE     64
#define PLC_SORTER_LEFT     65
#define PLC_SORTER_RIGHT    66
#define PLC_SORTER_CENTER   67

#define PLC_CAN_FULL_SENSOR    152 //can full
#define PLC_CAN_80_SENSOR      153 //can 80
#define PLC_PET_FULL_SENSOR    154 //pet full
#define PLC_PET_80_SENSOR      155 //pet 80
#define PLC_CAN_TRAY_SENSOR    156 //can tray
#define PLC_PET_TRAY_SENSOR    157 //pet tray

#define PLC_G_SORTER_TOP_SENSOR     181
#define PLC_G_SORTER_BOTTOM_SENSOR  182
#define PLC_G_SLING_TOP_SENSOR      183
#define PLC_G_SLING_BOTTOM_SENSOR   184
#define PLC_G_TRASH_TOP_SENSOR      185
#define PLC_G_TRASH_BOTTOM_SENSOR   186
#define PLC_G_TRAY_SENSOR           187

#define  DOOR_TOP_S     22
#define  DOOR_BOTTOM_S  23

#define  GEARED_M_JAM   40
#define  GLASS_GEARED_M_JAM   159


#define  PLC_STATUS_RESET     200
#define  IR_STATUS_RESET      201

#define PLC_STATUS_BLOCK_ADDR       8
#define IR_STATUS_BLOCK_ADDR        80

#define  PLC_STATUS_RESET_VALUE     0
#define  IR_STATUS_RESET_VALUE      0

#define  GEARED_MOTOR_START     148



#define  SELF_TEST_START        160
#define  SELF_TEST_STEPPING_F   161
#define  SELF_TEST_STEPPING_R   162
#define  SELF_TEST_SORTER_L     163
#define  SELF_TEST_SORTER_R     164
#define  SELF_TEST_SORTER_C     165
#define  SELF_TEST_GEARED_F     166
#define  SELF_TEST_GEARED_R     167
#define  SELF_TEST_GEARED_S     168
#define  SELF_TEST_NONE         169

#define  SELF_GTEST_START        152
#define  SELF_GTEST_STEPPING_F   153
#define  SELF_GTEST_STEPPING_R   154
#define  SELF_GTEST_GEARED_F     155
#define  SELF_GTEST_GEARED_R     156
#define  SELF_GTEST_SORTER_UD    157




#define  PLCMD_START            "Start"
#define  PLCMD_STOP             "Stop"

#define  PLCMD_CHECKSTART       "CheckStart"
#define  PLCMD_CHECKSTOP        "CheckStop"
#define  PLCMD_STEPPING_F       "SteppingF"
#define  PLCMD_STEPPING_R       "SteppingR"
#define  PLCMD_SORTER_LEFT      "SorterL"
#define  PLCMD_SORTER_RIGHT     "SorterR"
#define  PLCMD_SORTER_CENTER    "SorterC"
#define  PLCMD_GEARED_F         "GearedF"
#define  PLCMD_GEARED_R         "GearedR"
#define  PLCMD_GEARED_S         "GearedS"
#define  PLCMD_QUERY_STATE      "PLCState"

#define  PROCESS_MAIN   "main"
#define  PROCESS_UI     "ui"
#define  PROCESS_PLC    "plc"
#define  PROCESS_PRINTER    "printer"
#define  PROCESS_CLS    "classifier"

//rohs edit : 20180204
#define  PLCMD_CONVEYOR_LONG_ON     "ConveyorLongTestOn"
#define  PLCMD_CONVEYOR_LONG_OFF    "ConveyorLongTestOff"
#define  PLCMD_CRUSHER_LONG_ON      "CrusherLongTestOn"
#define  PLCMD_CRUSHER_LONG_OFF     "CrusherLongTestOff"
#define  PLCMD_SORTER_LONG_ON       "SorterLongTestOn"
#define  PLCMD_SORTER_LONG_OFF      "SorterLongTestOff"


#define REQ_TIMEOUT    15000
#define SW_UPDATE_CHECKTIME    3600000
//#define SW_UPDATE_CHECKTIME    120000
void setCurrentLogLevel(int level);
void qLog(int level, const QString & owner, const QString & category, const QString &description);






#endif // NZMQT_GLOBAL_HPP
