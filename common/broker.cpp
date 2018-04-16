#include "broker.h"

#include <QTimer>
#include <QDateTime>
#include "global.hpp"


void Broker::startImpl()
{
    bind(NEURALBROKER_IPC);
    QTimer::singleShot(HEARTBEAT_INTERVAL, this, SLOT(sendHeartToWorker()));
}

void Broker::sendHeartToWorker()
{

    purge_workers ();

    for (QSet<worker*>::iterator it = m_waiting.begin();
          it != m_waiting.end() && (*it)!=0; it++) {
        ////qDebug() << "sendHeartToWorker>>> " << (*it)->m_service;
        worker_send (*it, MDPW_HEARTBEAT, "", NULL);
    }

   QTimer::singleShot(HEARTBEAT_INTERVAL, this, SLOT(sendHeartToWorker()));

}

void  Broker::bind (const QString&  endpoint)
{
   m_endpoint = endpoint;
   m_socket->bindTo(m_endpoint);
   ////qDebug() << "I: MDP broker/0.1.1 is active at " << endpoint;
}

void Broker::messageReceived(const QList<QByteArray> & message)
{
    if (m_verbose) {
        //qDebug() << "Broker> " << message;
    }
//qDebug() << "Broker >>> Message Incomming (" << message << ")";
    zmsg * msg = new zmsg(message);

    QString sender = QString(msg->pop_front());
    msg->pop_front(); //empty message
    QString header = QString(msg->pop_front());

//              std::cout << "sbrok, sender: "<< sender << std::endl;
//              std::cout << "sbrok, header: "<< header << std::endl;
//              std::cout << "msg size: " << msg->parts() << std::endl;
//              msg->dump();
    if (QString::compare(header,MDPC_CLIENT) == 0) {
        client_process (sender, msg);
    }
    else if (header.compare(MDPW_WORKER) == 0) {
        worker_process (sender, msg);
    }
    else {
        //qDebug() << "Broker> " << message;
        delete msg;
    }
}


void Broker::purge_workers ()
{
   QQueue<worker*> toCull;
   uint now = QDateTime::currentDateTime().toTime_t();

   for (QSet<worker*>::iterator wrk = m_waiting.begin(); wrk != m_waiting.end(); ++wrk)
   {

       if ((*wrk)->m_expiry <= now)
           toCull.enqueue(*wrk);
   }

   for (QQueue<worker*>::iterator wrk = toCull.begin(); wrk != toCull.end(); ++wrk)
   {
       if (m_verbose) {
          //qDebug() << "I: deleting expired worker: %s", (*wrk)->m_identity;
       }
       worker_delete(*wrk, 0);
   }
}

service * Broker::service_require (const QString&  name)
{
   assert (name.size()>0);
   if (m_services.count(name)) {
      return m_services[name];
   } else {
       service * srv = new service(name);
       m_services.insert(name, srv);
       if (m_verbose) {
            //qDebug() << "I: received message:";
       }
       return srv;
   }
}

void Broker::service_dispatch (service *srv, zmsg *msg)
{
   assert (srv);
   if (msg) {                    //  Queue message if any
       srv->m_requests.push_back(msg);
   }

   purge_workers ();
   while (! srv->m_waiting.empty() && ! srv->m_requests.empty())
   {
       // Choose the most recently seen idle worker; others might be about to expire
       QList<worker*>::iterator wrk = srv->m_waiting.begin();
       QList<worker*>::iterator next = wrk;
       for (++next; next != srv->m_waiting.end(); ++next)
       {
          if ((*next)->m_expiry > (*wrk)->m_expiry)
             wrk = next;
       }

       zmsg *msg = srv->m_requests.front();
       srv->m_requests.pop_front();
       worker_send (*wrk, MDPW_REQUEST, "", msg);
       m_waiting.remove(*wrk);
       srv->m_waiting.erase(wrk);
       delete msg;
   }
}
void Broker::service_internal (const QString&  service_name, zmsg *msg)
{
  if (QString::compare(service_name, "mmi.service", Qt::CaseInsensitive) == 0) {
      service * srv = m_services[QString(msg->body())];
      if (srv && srv->m_workers) {
          msg->body_set("200");
      } else {
          msg->body_set("404");
      }
  } else {
      msg->body_set("501");
  }

  //  Remove & save client return envelope and insert the
  //  protocol header and service name, then rewrap envelope.
  QString client = msg->unwrap();
  msg->wrap(MDPC_CLIENT, service_name);
  msg->wrap(client, "");
  msg->send (*m_socket);
}


//  ---------------------------------------------------------------------
   //  Creates worker if necessary

worker * Broker::worker_require (const QString&  identity)
{
    assert (identity.length()!=0);

    //  self->workers is keyed off worker identity
    if (m_workers.count(identity)) {
      return m_workers[identity];
    } else {
      worker *wrk = new worker(identity);
      m_workers.insert(identity, wrk);
      if (m_verbose) {
         //qDebug() << "I: registering new worker: %s" << identity;
      }
      return wrk;
    }
}

//  ---------------------------------------------------------------------
//  Deletes worker from all data structures, and destroys worker

void Broker::worker_delete (worker *&wrk, int disconnect)
{
  assert (wrk);
  if (disconnect) {
      worker_send (wrk, (char*)MDPW_DISCONNECT, "", NULL);
  }

  if (wrk->m_service) {
      for(QList<worker*>::iterator it = wrk->m_service->m_waiting.begin();
            it != wrk->m_service->m_waiting.end();) {
         if (*it == wrk) {
            it = wrk->m_service->m_waiting.erase(it);
         }
         else {
            ++it;
         }
      }
      wrk->m_service->m_workers--;
  }
  m_waiting.remove(wrk);
  //  This implicitly calls the worker destructor
  m_workers.remove(wrk->m_identity);
  delete wrk;
}


//  ---------------------------------------------------------------------
//  Process message sent to us by a worker

void Broker::worker_process (const QString&  sender, zmsg *msg)
{
   assert (msg && msg->parts() >= 1);     //  At least, command

   QString command = msg->pop_front();
   bool worker_ready = m_workers.count(sender)>0;
   worker *wrk = worker_require (sender);
   //qDebug() << "worker_process >>>>>> " << sender;
   msg->dump();
   if (command.compare (MDPW_READY) == 0) {
       if (worker_ready)  {              //  Not first command in session
           worker_delete (wrk, 1);
       }
       else {
           if (sender.size() >= 4  //  Reserved service name
           &&  sender.startsWith("mmi.") ) {
               worker_delete (wrk, 1);
           } else {
               //  Attach worker to service and mark as idle
               QString service_name = msg->pop_front ();
               wrk->m_service = service_require (service_name);
               wrk->m_service->m_workers++;
               worker_waiting (wrk);
           }
       }
   } else {
      if (command.compare (MDPW_REPLY) == 0) {
          //qDebug() << "Worker Reply>>>> " ;
          if (worker_ready) {

              //  Remove & save client return envelope and insert the
              //  protocol header and service name, then rewrap envelope.
              QString client = msg->unwrap ();
              //qDebug() << " Reply>>to >>  " << client;
              msg->wrap (MDPC_CLIENT, wrk->m_service->m_name);
              msg->push_front("");
              msg->push_front(client.toLocal8Bit());

              msg->send (*m_socket);
              worker_waiting (wrk);
          }
          else {
              worker_delete (wrk, 1);
          }
      } else {
         if (command.compare (MDPW_HEARTBEAT) == 0) {
             if (worker_ready) {
                 wrk->m_expiry = QDateTime::currentDateTime().toTime_t() + HEARTBEAT_EXPIRY;
             } else {
                 worker_delete (wrk, 1);
             }
         } else {
            if (command.compare (MDPW_DISCONNECT) == 0) {
                worker_delete (wrk, 0);
            } else {
                //qDebug() << "E: invalid input message " << command;
                //qDebug() << "            body message " << msg;
            }
         }
      }
   }
   delete msg;
}

//  ---------------------------------------------------------------------
//  Send message to worker
//  If pointer to message is provided, sends that message
void Broker::worker_send (worker *worker, const QString&  command, const QString& option, zmsg *msg)
{
    msg = (msg ? new zmsg(*msg) : new zmsg ());
    //qDebug() << "I: worker_send" << command;
    //  Stack protocol envelope to start of message
    if (option.size()>0) {                 //  Optional frame after command
        msg->push_front (option.toLocal8Bit());
    }
    msg->push_front (command.toLocal8Bit());
    msg->push_front (MDPW_WORKER);
    //  Stack routing envelope to start of message
    msg->wrap(worker->m_identity, "");
    msg->dump();
    if (m_verbose) {
        //qDebug() << "I: sending %s to worker" << command;
        //qDebug() << "message body: " << msg;
    }
    msg->send (*m_socket);
    delete msg;
}

//  ---------------------------------------------------------------------
//  This worker is now waiting for work

void Broker::worker_waiting (worker *worker)
{
    assert (worker);
    //  Queue to broker and service waiting lists
    m_waiting.insert(worker);
    worker->m_service->m_waiting.push_back(worker);

    worker->m_expiry = QDateTime::currentDateTime().toTime_t() + HEARTBEAT_EXPIRY;
    // Attempt to process outstanding requests
    service_dispatch (worker->m_service, 0);
}

//  ---------------------------------------------------------------------
//  Process a request coming from a client

void Broker::client_process (const QString&  sender, zmsg *msg)
{
    assert (msg && msg->parts () >= 2);     //  Service name + body
    if (m_verbose) {
        //qDebug() << "client_process for " << sender;
        msg->dump();
    }
    QString service_name = msg->pop_front();
    if (m_verbose) {
        msg->dump();
    }
    service *srv = service_require (service_name);
    //  Set reply return address to client sender
    msg->wrap (sender, "");
    if (m_verbose) {
        msg->dump();
    }
    if (service_name.length() >= 4  &&  service_name.startsWith("mmi.")) {
        //qDebug() << "service_internal for " << sender;
        service_internal (service_name, msg);
    }
    else
    {
        //qDebug() << "service_dispatch for " << sender;
        service_dispatch (srv, msg);
    }
}

