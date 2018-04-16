QT += core network
QT -= gui

CONFIG += c++11

TARGET = main
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    mainapplicaton.cpp \
    infoserve.cpp \
    imageuploader.cpp \
    orderer.cpp \
    softwareupdater.cpp \
    filedownloader.cpp \
    depositservice.cpp

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
    mainapplicaton.h \
    infoserve.h \
    imageuploader.h \
    orderer.h \
    softwareupdater.h \
    filedownloader.h \
    depositservice.h
