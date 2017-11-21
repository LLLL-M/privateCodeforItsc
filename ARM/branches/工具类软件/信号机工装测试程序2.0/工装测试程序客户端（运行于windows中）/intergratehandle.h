#ifndef INTERGRATEHANDLE_H
#define INTERGRATEHANDLE_H

#include <QTimer>
#include <QFile>
#include <QObject>
//#include <QWidget>
#include <QCloseEvent>
#include "udpserver.h"
#include "msg.h"

class IntergrateHandle : public QObject
{
    Q_OBJECT
public:
    explicit IntergrateHandle(QObject* parent = 0);
    void msgDispatcher(unsigned int type, char *data, int len);//发送数据函数
    void writeLog(const char *);//写日志函数（包括界面日志和文件日志）
signals:
    void ConnectionAccepted(QVariant);//连接响应信号（参数为true表示连接陈功，false表示连接失败）
    //void appExit();
    //300
    void updateButtonState(QVariant, QVariant);//更新界面测试项按钮成功（绿色）失败（红色）状态颜色
    void updatePedKeys(QVariant);//更新行人按钮状态（按下或松开，同时包括8组按钮的当前状态）
    void restart();//重新开始测试，界面各部分恢复到默认状态
    void wirelessStateChage(QVariant);	//无线遥控器测试界面切换信号	false-恢复到默认状态 true-切换到遥控器按钮状态显示界面
    void wirelessButtonClicked(QVariant); //无线遥控器按钮按下信号，参数：按钮值 范围1-5(自动，手动，黄闪，全红，步进)
    //500
    void ioInputChangeState(QVariant);
    void IoInputUpdateStatus(QVariant);
    void updateCarDetectors(QVariant);	//更新界面车检器过车状态
public slots:
    void handleConnectReq(QString ip, int type);//处理来自界面的连接请求
    void processDatagram(QByteArray *);//处理来自信号机的消息
    void appExitHandle();

    //300
    void slotUSBButton();//点击usb测试后的处理函数
    void slotRS485Button();//点击RS485测试后的处理函数
    //void slotCurVoltButton();//点击电流电压测试后的处理函数
    void slotWifi3GButton();//点击Wifi/3G测试后的处理函数
    void slotAutoTestButton();//点击自动测试后的处理函数
    void slotLampCtrlButton(QVariant, QVariant);//点击灯控板指示灯测试后的处理函数
    void slotFrontBoardButton();//点击前面板指示灯测试后的处理函数
    void slotGPSButton();
    void slotWirelessButton();//点击无线遥控器测试后的处理函数
    void slotPedKeysButton();//点击行人按钮测试后的处理函数
    void getPedKeysStatus();//向信号机发送获取行人按键状态函数
    void slotKeyBoardButton();
    //500
    void slotButtonClicked(int);//测试按钮按下处理函数
    void getCarDetectorStatus();//向信号机发送获取车检器状态函数

    //日志
    void LogFileInit(QString);//日志文件初始化函数
    void LogFileClose();//关闭日志文件
    void LogClear();//日志清除
private:
    void tsc300UIInit();
    void tsc500UIInit();

    UdpServer *commServ;//udp服务器，提供收发数据接口
    QObject *rootQml;
    QObject *outputInfo;//界面（qml）显示日志控件
    //300
    QObject *pedKeysButton;//行人按键测试按钮
    QList<int> lastButtonStatus;//上一次8组行人按键状态
    QObject *wirelessButton;//无线遥控器测试按钮
    QObject *gpsButton;
    QObject *keyBoardButton;
    //500
    QObject *obTsc500;
    QObject *ioInputButton;
    QObject *carDetectorButton;
    QObject *carDetPage;
   // QObject *lampCtrlPage;

    QTimer *timer;//内部定时器

    QFile *logFile;//日志文件
    unsigned int tscID;//信号机标识号
    unsigned int tscType;

};

#endif // INTERGRATEHANDLE_H
