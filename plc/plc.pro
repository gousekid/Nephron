QT += core  serialbus serialport gui multimedia multimediawidgets xml

QT -= gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

TARGET = plc
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    plcapplication.cpp \
    plctask.cpp \
    modbusinterface.cpp \
    atmega128app.cpp



DESTDIR = $$_PRO_FILE_PWD_/../bin

unix:!macx: LIBS += -L$$OUT_PWD/../common/ -lcommon

INCLUDEPATH += $$PWD/../common
DEPENDPATH += $$PWD/../common

unix:!macx: PRE_TARGETDEPS += $$OUT_PWD/../common/libcommon.a

unix:!macx: LIBS += -L$$PWD/../../../../../usr/local/lib/ -lzmq

INCLUDEPATH += /usr/local/include
DEPENDPATH += /usr/local/include


unix:!macx: PRE_TARGETDEPS += /usr/local/lib/libzmq.a

HEADERS += \
    plcapplication.h \
    plctask.h \
    modbusinterface.h \
    atmega128app.h
