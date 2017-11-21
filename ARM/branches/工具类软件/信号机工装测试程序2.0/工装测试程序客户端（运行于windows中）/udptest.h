#ifndef UDPTEST_H
#define UDPTEST_H

#include <QObject>
#include "udpserver.h"

class UdpTest : public QObject
{
    Q_OBJECT
public:
    explicit UdpTest(QObject *parent = 0);

signals:

public slots:
    void slotRecvData(QByteArray *);
private:
    UdpServer *serv;
};

#endif // UDPTEST_H
