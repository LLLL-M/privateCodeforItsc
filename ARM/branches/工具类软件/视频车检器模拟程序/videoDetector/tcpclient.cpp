#include "tcpclient.h"
#include <QThread>

TcpClient::TcpClient(QObject *parent) : QTcpSocket(parent)
{
    this->baData = new QByteArray;
    connect(this, SIGNAL(connected()), this, SLOT(sendData()));
}
void TcpClient::tcpConnect(QHostAddress *ip, int port)
{
    this->connectToHost(*ip, port);
}
void TcpClient::setData(char type, QByteArray &baData)
{
   if(type>0 && type <=4)
   {
       this->type = type;
       this->baData->resize(baData.size());
       memcpy(this->baData->data(), baData.data(), baData.size());
   }
}
void TcpClient::sendData()
{
    this->write(baData->data(), baData->size());
    this->disconnectFromHost();
    emit connectionClose(this->type);
}
