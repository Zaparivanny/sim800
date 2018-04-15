#include "worker.h"
#include "../../../src/sim900.h"

#include <QDebug>
#include <QSerialPort>
#include "example_config.h"
#include <iostream>

Worker::Worker(QObject *parent) : QObject(parent)
{
    connect(this, &Worker::sserialWrite, this, &pserialWrite);
    
    m_serial = new QSerialPort();
    m_serial->setPortName("COM6");
    m_serial->setBaudRate(QSerialPort::Baud115200);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);
    m_serial->open(QIODevice::ReadWrite);
    
    connect(m_serial, &QSerialPort::errorOccurred, [=](QSerialPort::SerialPortError error){
        qDebug() << "QSerialPort::error" << error;
    });
    
    connect(m_serial, &QSerialPort::readyRead, [=](){
        QByteArray data = m_serial->readAll();
        qDebug() << "QSerialPort" << data;
        for(int i = 0; i < data.size(); i++)
        {
            simx_receive(data[i]);
        }
        
    });
}

void Worker::send()
{
    sim_reply_t reply;
    simx_test(&reply);
    simx_wait_reply();
    qDebug() << "[Worker::send1]" << reply.status;
    simx_pin_is_required(&reply);
    simx_wait_reply();
    qDebug() << "[Worker::send2]" << reply.status;
    simx_network_registration(&reply);
    simx_wait_reply();
    qDebug() << "[Worker::send3]" << reply.status;
}

void Worker::send_sms()
{
    sim_reply_t reply;
    simx_send_sms(&reply, PHONE_NUMBER, "test");
    simx_wait_reply();
    qDebug() << "[Worker::send2]" << reply.status;
}

void Worker::connect_to_gprs()
{
    sim_reply_t reply;
    simx_is_attach_to_GPRS(&reply);
    simx_wait_reply();
    if(reply.status != SIM300_OK){return;}
    
    simx_deactivate_gprs_pdp(&reply);
    simx_wait_reply();
    if(reply.status != SIM300_SHUT_OK){return;}
    
    simx_current_connection_status(&reply);
    simx_wait_reply();
    if(reply.status != SIM300_OK){return;}
    
    simx_set_gprs_config(&reply, APN, APN_USER_NAME, APN_PASSWORD);
    simx_wait_reply();
    if(reply.status != SIM300_OK){return;}
    
    simx_bring_up_wireless_connection(&reply);
    simx_wait_reply();
    if(reply.status != SIM300_OK){return;}
    
    sim_ip_t ip;
    simx_get_local_ip(&reply, &ip);
    simx_wait_reply();
    if(reply.status != SIM300_OK){return;}
    std::cout << "IP:" << (int)ip.addr0 << "." << (int)ip.addr1 << "."
                       << (int)ip.addr2 << "."<< (int)ip.addr3 << "." << std::endl;
    
}
sim_sms_t sms;
void Worker::read_sms()
{
    sim_reply_t reply;
    
    char buffer[200];
    sms.msg = buffer;
    sms.msg_length = 200;
    simx_read_sms(&reply, &sms, 4);
    simx_wait_reply();
    if(reply.status != SIM300_OK){return;}
    std::cout << "SMS:" << sms.msg;
}

void Worker::pserialWrite(char *data, quint64 length)
{
    QByteArray str((char*)data, length);
    qDebug() << str << length;
    m_serial->write(data, length);
}

void Worker::moveToThread(QThread *thread)
{
    QObject::moveToThread(thread);
    m_serial->moveToThread(thread);
}

