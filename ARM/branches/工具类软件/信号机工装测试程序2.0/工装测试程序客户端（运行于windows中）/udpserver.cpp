#include "msg.h"
#include "udpserver.h"
#include <QDebug>

UdpServer::UdpServer(QString ip)
{
    //this->ip = ip;
    this->ipaddr.setAddress(ip);
    this->port = UDP_SOCK_PORT;
    //QHostAddress tscip(this->ip);
    init(this->port);
}
void UdpServer::init(int port)
{
    udpSocket = new QUdpSocket(this);
    udpSocket->bind(QHostAddress::Any, port);

    connect(udpSocket, SIGNAL(readyRead()),
            this, SLOT(readPendingDatagrams()));
}
void UdpServer::readPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
//        QHostAddress sender;
//        quint16 senderPort;

        udpSocket->readDatagram(datagram.data(), datagram.size(),
                                &sender, &senderPort);

        //qDebug()<<"03 - udpserver: recv data."<<endl;
        dataRecved(&datagram);
    }
}
void UdpServer::sendData(char *data, int len)
{
    if(NULL != data && 0 != len)
        udpSocket->writeDatagram(data, len, this->ipaddr, this->port);
}
/*
void UdpServer::processTheDatagram(QByteArray *datagram)
{
   ST_UDP_MSG *msg = (ST_UDP_MSG*)datagram->data();
   if(msg->uVer == FTEST_PRO_VER2 && msg->uhead == FTEST_UDP_HEAD)
   {
      switch(msg->uType)
      {
        case FTEST_MSG_CONNECT:
          break;
        case FTEST_MSG_USB:
          break;
        default:
          break;
      }
   }
}
*/
