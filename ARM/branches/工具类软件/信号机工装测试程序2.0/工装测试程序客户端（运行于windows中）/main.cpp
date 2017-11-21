#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickItem>
#include "intergratehandle.h"
//#include "udptest.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    /*
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    QObject*tscCon = engine.rootObjects().first()->findChild<QObject*>("connectTsc");
    if(tscCon)
        IntergrateHandle interQml(tscCon);
    else
        qDebug()<<"can't find tscConnect"<<endl;
*/
    //UdpTest test;
    IntergrateHandle interQml;
    return app.exec();
}
