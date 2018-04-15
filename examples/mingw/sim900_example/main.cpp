#include <QCoreApplication>
#include <QSerialPort>
#include <QDebug>
#include <QThread>
#include <QObject>
#include "../../../src/sim900.h"
#include "stdint.h"
#include "worker.h"

//QSerialPort g_serial;

Worker *worker;

void simx_callback_send(uint8_t *data, uint16_t length)
{
    worker->sserialWrite((char*)data, length);
}

void simx_callback_tcp_msg(sim_con_status_t con_status, uint8_t n)
{
    
}

void simx_callback_tcp_data(uint8_t *data, uint16_t length, uint8_t n)
{
    
}

void simx_callback_sms_received(uint16_t number)
{
    
}

void simx_callback_pdp_deact()
{
    
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    QThread *thread = new QThread;
    
    worker = new Worker();
    worker->moveToThread(thread);
    thread->start();
    
    //QMetaObject::invokeMethod(worker, "send", Qt::DirectConnection);
    //QMetaObject::invokeMethod(worker, "send_sms", Qt::DirectConnection);
    //QMetaObject::invokeMethod(worker, "connect_to_gprs", Qt::DirectConnection);
    QMetaObject::invokeMethod(worker, "read_sms", Qt::DirectConnection);
    
    return a.exec();
}
