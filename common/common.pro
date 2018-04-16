#-------------------------------------------------
#
# Project created by QtCreator 2016-06-16T12:25:03
#
#-------------------------------------------------
QT       += core network

TARGET = common
TEMPLATE = lib
CONFIG += staticlib



SOURCES += \
    nzmqt.cpp \
    ipcbase.cpp \
    publisher.cpp \
    subscriber.cpp \
    broker.cpp \
    zmsg.cpp \
    worker.cpp \
    client.cpp \
    utils.cpp

HEADERS += \
    zmq.hpp \
    global.hpp \
    impl.hpp \
    nzmqt.hpp \
    ipcbase.h \
    publisher.h \
    subscriber.h \
    broker.h \
    zmsg.h \
    worker.h \
    client.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}

unix:!macx: LIBS += -L$$PWD/../lib/ -lzmq

INCLUDEPATH += $$PWD/../include
DEPENDPATH += $$PWD/../include

unix:!macx: PRE_TARGETDEPS += $$PWD/../lib/libzmq.a
