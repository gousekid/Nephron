#include "zmsg.h"
#include <QDebug>
static char *encode_uuid (const unsigned char *data)
{
  static char hex_char [] = "0123456789ABCDEF";

  assert (data [0] == 0);
  char *uuidstr = new char[34];
  uuidstr [0] = '@';
  int byte_nbr;
  for (byte_nbr = 0; byte_nbr < 16; byte_nbr++) {
      uuidstr [byte_nbr * 2 + 1] = hex_char [data [byte_nbr + 1] >> 4];
      uuidstr [byte_nbr * 2 + 2] = hex_char [data [byte_nbr + 1] & 15];
  }
  uuidstr [33] = 0;
  return (uuidstr);
}
static unsigned char * decode_uuid (const char *uuidstr)
{
   static char
       hex_to_bin [128] = {
          -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* */
          -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* */
          -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* */
           0, 1, 2, 3, 4, 5, 6, 7, 8, 9,-1,-1,-1,-1,-1,-1, /* 0..9 */
          -1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* A..F */
          -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* */
          -1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* a..f */
          -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 }; /* */

   assert (strlen (uuidstr) == 33);
   assert (uuidstr [0] == '@');
   unsigned char *data = new unsigned char[17];
   int byte_nbr;
   data [0] = 0;
   for (byte_nbr = 0; byte_nbr < 16; byte_nbr++)
       data [byte_nbr + 1]
           = (hex_to_bin [uuidstr [byte_nbr * 2 + 1] & 127] << 4)
           + (hex_to_bin [uuidstr [byte_nbr * 2 + 2] & 127]);

   return (data);
}


zmsg::zmsg()
{

}

zmsg::zmsg(const QByteArray &body)
{
    body_set(body);
}

zmsg::zmsg(const QList<QByteArray> &msg)
{
    for (int i = 0; i < msg.size(); ++i)
    {
        if (msg.at(i).size() == 17 && (msg.at(i).at(0) == 0) )
        {
            char *uuidstr = encode_uuid((unsigned char*) msg.at(i).constData());
            push_back(uuidstr);
            delete[] uuidstr;
        }
        else
        {
            push_back(msg.at(i));
        }
    }
}

int zmsg::parts()
{
    return m_part_data.size();
}


QByteArray zmsg::body ()
{
   if (m_part_data.size())
       return m_part_data [m_part_data.size() - 1];
   else
       return emptyByteArray;
}

void zmsg::body_set(const QByteArray &body)
{

    if (m_part_data.size() > 0) {
       m_part_data.erase(m_part_data.end()-1);
    }
    push_back(body);

}

void zmsg::push_back(const QByteArray &part)
{
    m_part_data.push_back(part);
}


// zmsg_push
void zmsg::push_front(const QByteArray &part)
{
    m_part_data.insert(m_part_data.begin(), part);
}




QString zmsg::pop_front()
{
    if (m_part_data.size() == 0)
    {
        return emptyString;
    }
    QString part = QString(m_part_data.first());
    m_part_data.erase(m_part_data.begin());

    return part;
}

QString zmsg::address()
{
    if (m_part_data.size()>0)
    {
        return QString(m_part_data[0].constData());
    }
    else
    {
        return emptyString;
    }
}


void zmsg::wrap(const QByteArray &address, const QByteArray &delim)
{
    if (delim!=NULL) {
        push_front(delim);
    }
    push_front(address);
}

void zmsg::wrap(const QString &address, const QString &delim)
{
    if (delim!=NULL) {
        push_front(delim.toLocal8Bit());
    }
    push_front(address.toLocal8Bit());
}

QString zmsg::unwrap()
{
    if (m_part_data.size() == 0)
    {
        return emptyString;
    }

    QString addr = QString(pop_front());

    if (address().size() > 0 && address().at(0) == 0)
    {
        pop_front();
    }
    return addr;
}

void zmsg::send(ZMQSocket& socket)
{
    QList< QByteArray > message;
    for (int part_nbr = 0; part_nbr < m_part_data.size(); part_nbr++)
    {

        QByteArray data = m_part_data[part_nbr];
        if (data.size() == 33 && data [0] == '@')
        {
            QByteArray uuidbin =  QByteArray((char*)decode_uuid ( data.constData()), 17);
            message += uuidbin;

        }
        else
        {
            message += data;
        }


    }

    try
    {
        socket.sendMessage(message);
    }
    catch (zmq::error_t error)
    {
        assert(error.num()!=0);
    }

    //qDebug() << "zmsg send> " << m_part_data;
    m_part_data.clear();
}


void zmsg::dump()
{

    //qDebug() << "Message dump : " << m_part_data;
}
