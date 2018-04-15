#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QSerialPort>

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = nullptr);
    
    void serialWrite(char *data, quint64 length);
    void moveToThread(QThread *thread);
private:
    void pserialWrite(char *data, quint64 length);
private:
    QSerialPort *m_serial;
signals:
    void sserialWrite(char *data, quint64 length);
    
public slots:
    void send();
    void send_sms();
    void connect_to_gprs();
    void read_sms();
};

#endif // WORKER_H
