QT += core multimedia serialport
QT -= gui

CONFIG += c++11

DESTDIR = $$_PRO_FILE_PWD_/../bin


TARGET = classifier
CONFIG += console
CONFIG -= app_bundle


TEMPLATE = app

SOURCES += main.cpp \
    classifierapplication.cpp \
    barcodereader.cpp \
    classifier.cpp \
    detector.cpp \
    neurogenie.cpp
 #  cameraframegrabber.cpp

unix:!macx: LIBS += -L$$OUT_PWD/../common/ -lcommon

unix:!macx: LIBS += -lboost_system -lglog -lopencv_core -lopencv_highgui
unix:!macx: LIBS += -lgflags -lprotobuf -lboost_filesystem  -lleveldb -lsnappy -lm
INCLUDEPATH += $$PWD/../common
INCLUDEPATH += $$PWD/../include
INCLUDEPATH += /home/superbin/caffe/include/
INCLUDEPATH += /usr/local/cuda/bin
INCLUDEPATH += /usr/local/bin
INCLUDEPATH += /usr/local/cuda/include

INCLUDEPATH += $$PWD/../nephron_classify/src/

DEPENDPATH += $$PWD/../common

unix:!macx: PRE_TARGETDEPS += $$OUT_PWD/../common/libcommon.a


#unix:!macx: LIBS +=  /home/superbin/Nephron/data/neurogenie.so
#unix:!macx: LIBS +=  /home/superbin/Nephron/data/classifier.so
#unix:!macx: LIBS +=  /home/superbin/Nephron/data/detector.so

#unix:!macx: LIBS +=  /home/superbin/Nephron/data/neurogenie.so

unix:!macx: LIBS += -L/usr/local/lib
unix:!macx: LIBS += -L/home/superbin/caffe/build/lib
unix:!macx: LIBS += -L/usr/local/cuda/lib64
unix:!macx: LIBS += -L/usr/local/lib/ -lzmq

unix:!macx: LIBS += /home/superbin/caffe/build/lib/libcaffe.so

INCLUDEPATH += /usr/local/include
DEPENDPATH += /usr/local/include

unix:!macx: PRE_TARGETDEPS += /usr/local/lib/libzmq.a

HEADERS += \
    classifierapplication.h \
    barcodereader.h \
    classifier.hpp \
    detector.hpp \
    neurogenie.hpp
#   cameraframegrabber.h

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += opencv
