#ifndef IPCBASE_H
#define IPCBASE_H

#include <QObject>
#include <QDebug>
#include <QEventLoop>
#include <QThread>
#include "nzmqt.hpp"



#define BROADCAST_MSG   "BRD"


class IPCBase : public QObject
{
    Q_OBJECT
    typedef QObject super;
public:


protected:
    IPCBase(QObject* parent);

    virtual void startImpl() = 0;

    static void sleep(unsigned long msecs);

private:
    class ThreadTools : private QThread
    {
    public:
        using QThread::msleep;

    private:
        ThreadTools() {}
    };

signals:
    void finished();
    void failure(const QString& what);

public slots:
    void start();
    void stop();
};

#endif // IPCBASE_H

