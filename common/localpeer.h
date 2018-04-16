#ifndef LOCALPEER_H
#define LOCALPEER_H

#include <QObject>

#include <QLocalServer>
#include <QLocalSocket>
#include <QDir>

#include "lockedfile.h"

class LocalPeer : public QObject
{
    Q_OBJECT
public:
    explicit LocalPeer(QObject *parent = 0, const QString &appId = QString());
    bool isClient();
    bool sendMessage(const QString &message, int timeout);
    QString applicationId() const
        { return id; }

Q_SIGNALS:
    void messageReceived(const QString &message);

protected Q_SLOTS:
    void receiveConnection();

protected:
    QString id;
    QString socketName;
    QLocalServer* server;
    QtLP_Private::LockedFile lockFile;

private:
    static const char* ack;
};

#endif // LOCALPEER_H
