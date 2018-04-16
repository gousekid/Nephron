#ifndef BARCODEREADER_H
#define BARCODEREADER_H

#include <QObject>
#include <QtSerialPort/QSerialPort>

class BarcodeReader : public QObject
{
    Q_OBJECT
public:
    explicit BarcodeReader(QObject *parent = 0);
    void sendTrigger(bool bOn);

signals:
    void reportError(uint);
    void coderead(const QString&);
public slots:
    void handleRead();
    void handleError(QSerialPort::SerialPortError error);
    void reconnect();
private:
     QSerialPort *serial;
     void sendString(const char *str);



};

#endif // BARCODEREADER_H
