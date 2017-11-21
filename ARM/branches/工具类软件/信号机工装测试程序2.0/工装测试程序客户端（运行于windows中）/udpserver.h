#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QUdpSocket>
#include <QHostAddress>

class UdpServer: public QObject
{
Q_OBJECT
public:
    UdpServer(QString ip);

    void sendData(char *data, int len);
signals:
    void dataRecved(QByteArray *);
public slots:
    void readPendingDatagrams();
private:
    void init(int port);
    //void processTheDatagram(QByteArray *datagram);

private:
    QUdpSocket *udpSocket;
    //QString ip;
    QHostAddress ipaddr;
    unsigned int port;

    QHostAddress sender;
    quint16 senderPort;
};

#endif // UDPSERVER_H
