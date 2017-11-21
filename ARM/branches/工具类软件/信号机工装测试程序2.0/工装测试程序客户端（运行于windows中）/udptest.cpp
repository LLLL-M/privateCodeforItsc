#include <unistd.h>
#include <QDebug>
#include "udptest.h"
#include "udpserver.h"
#include "msg.h"

UdpTest::UdpTest(QObject *parent) : QObject(parent)
{
    serv = new UdpServer(QString("10.13.1.101"));
    connect(serv, SIGNAL(dataRecved(QByteArray*)), this, SLOT(slotRecvData(QByteArray*)));
    /*
    char buff[64]={0};
    ST_UDP_MSG *msg = (ST_UDP_MSG*)buff;
    msg->uVer = FTEST_PRO_VER2;
    msg->uhead = FTEST_UDP_HEAD;
    msg->uType = FTEST_MSG_TEST;
    memcpy((char *)msg->data, "hello world", 11);
    while(1)
    {
        usev.sendData(buff, 11+8);
        sleep(1);
    }
    */
}
void UdpTest::slotRecvData(QByteArray *msg)
{
    qDebug()<<"Recv>> len: "<<msg->length()<<", data:"<<msg->data()<<endl;
    serv->sendData(msg->data(), msg->length());
}
