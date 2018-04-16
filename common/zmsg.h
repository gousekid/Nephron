#ifndef ZMSG_H
#define ZMSG_H

#include <QList>
#include <QByteArray>

#include "nzmqt.hpp"

static const QString emptyString;
static const QByteArray emptyByteArray;
class zmsg
{
public:
    zmsg();

    zmsg(const QByteArray &body);

    zmsg(const QList<QByteArray> &msg);

    virtual ~zmsg()
    {

        m_part_data.clear();
    }
    int parts() ;
    void body_set(const QByteArray &body);
    void push_front(const QByteArray &part);
    void push_back(const QByteArray &part);
    QByteArray body();
    QString pop_front() ;
    QString address() ;
    void wrap(const QByteArray &address, const QByteArray &delim);
    void wrap(const QString &address, const QString &delim);
    QString unwrap() ;

    void send(ZMQSocket& socket);
    void dump();
private:
    QList<QByteArray> m_part_data;


};

#endif // ZMSG_H
