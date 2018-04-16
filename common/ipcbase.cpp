#include "ipcbase.h"


IPCBase::IPCBase(QObject* parent)
    : super(parent)
{
}

void IPCBase::start()
{
    try
    {
        startImpl();
    }
    catch (const ZMQException& ex)
    {
        qWarning() << Q_FUNC_INFO << "Exception:" << ex.what();
        emit failure(ex.what());
        emit finished();
    }
}

void IPCBase::stop()
{
    emit finished();
}

void IPCBase::sleep(unsigned long msecs)
{
    ThreadTools::msleep(msecs);
}

