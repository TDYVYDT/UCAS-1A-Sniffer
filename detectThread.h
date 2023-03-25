#ifndef DETECTTHREAD_H
#define DETECTTHREAD_H

#include <QThread>
#include "sniffer.h"
#include <QMutex>

class detectThread : public QThread {

    Q_OBJECT

public:
    sniffer my_sniffer;
    QMutex my_mutex;
    volatile bool is_lock;
    bool close;

public:
    detectThread(QObject*);
    ~detectThread();

protected:
    void run();

signals:
    void send_packet_info(QVector<QString>);

};

#endif // DETECTTHREAD_H
