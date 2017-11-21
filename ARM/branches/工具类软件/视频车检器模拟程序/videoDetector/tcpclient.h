#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QTcpSocket>
#include <QHostAddress>
#include <QByteArray>

class TcpClient : public QTcpSocket
{
    Q_OBJECT
public:
    TcpClient(QObject *parent = 0);
    void tcpConnect(QHostAddress *ip, int port);
    void setData(char type, QByteArray &baData);
signals:
    void connectionClose(char type);
public slots:
    void sendData();
//    void slotDisconnected();
private:
    char type;
    QByteArray *baData;
};

#endif // TCPCLIENT_H
