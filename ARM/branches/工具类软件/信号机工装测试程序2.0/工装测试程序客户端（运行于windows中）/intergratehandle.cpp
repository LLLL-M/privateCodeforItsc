#include "intergratehandle.h"
#include <QQmlApplicationEngine>
#include <QDir>
#include <QQuickView>
#include <QQuickItem>
#include <QUrl>
#include <QDebug>
#include <QWaitCondition>
#include <QMutex>
#include <QDate>
#include <string.h>
#include "udpserver.h"
#include "msg.h"

#define FTEST_TOOL_VER	"v2.0.0"
#define TSC_TYPE_300    1
#define TSC_TYPE_500    2

IntergrateHandle::IntergrateHandle(QObject *obj) : QObject(obj)
{
    timer = new QTimer(this);
    logFile = NULL;
    tscID = 0;
    tscType = 0;

    QQmlApplicationEngine *engine = new QQmlApplicationEngine;
    engine->load(QUrl(QStringLiteral("qrc:/main.qml")));

    rootQml = engine->rootObjects().first();
    //qDebug()<<"rootobj objname: "<<rootObj->objectName()<<endl;
    //QObject *rootWin= rootObj->findChild<QObject*>("rootWindow");
    QObject *tscCon = rootQml->findChild<QObject*>("connectTsc");
    //QObject *tsc300Test = rootObj->findChild<QObject*>("tsc300Test");
    if(NULL == rootQml || tscCon == NULL)
        qDebug()<<"can't find child obj in main qml."<<endl;

    QObject::connect(tscCon, SIGNAL(tscConnect(QString, int)), this, SLOT(handleConnectReq(QString, int)));
    QObject::connect(this, SIGNAL(ConnectionAccepted(QVariant)), rootQml, SLOT(tscConnected(QVariant)));
    //QObject::connect(rootQml, SIGNAL(setLogFilePath(QString)), this, SLOT(LogFileInit(QString)));
    QObject::connect(rootQml, SIGNAL(logStart(QString)), this, SLOT(LogFileInit(QString)));
    QObject::connect(rootQml, SIGNAL(logFinished()), this, SLOT(LogFileClose()));
    QObject::connect(rootQml, SIGNAL(clearLog()), this, SLOT(LogClear()));

    QObject::connect(rootQml, SIGNAL(testOut()), this, SLOT(appExitHandle()));
    //QObject::connect(this, SIGNAL(appExit()), rootQml, SLOT(winExit()));

    //tsc300UIInit(tsc300Test);

    //设置软件版本
    /*
    QObject * aboutDiaglog = rootQml->findChild<QObject*>("aboutDialog");
    if(aboutDiaglog)
    {
        QString sVer("版本:  ");
        sVer.append(FTEST_TOOL_VER);
        sVer.append("-b");
        sVer.append(QDate::currentDate().toString("yyyyMMdd"));
        //QDate date = QDate::currentDate();
        //qDebug()<<" ===> tool ver: "<<date.toString("yyyy/MM/dd");
        qDebug()<<" ===> tool ver: "<<sVer;
        aboutDiaglog->setProperty("version", sVer);
    }
    */

    //QQuickView *view = new QQuickView(QUrl(QStringLiteral("qrc:/ConnectTsc.qml")));
    //QQuickView view(QUrl::fromLocalFile("./ConnectTsc.qml"));
    //QObject *item = (QObject *)view->rootObject();
    //QObject*tscCon = item->findChild<QObject*>("connectTsc");
    //QObject::connect(item, SIGNAL(tscConnect(QString, int)), this, SLOT(handleConnectReq(QString, int)));
    //QObject::connect(item, SIGNAL(testSig()), this, SLOT(forTest()));
    //view->show();
    //qDebug()<<"inter construct..."<<endl;
}

void IntergrateHandle::handleConnectReq(QString ip, int type)
{
   commServ = new UdpServer(ip);
   //qDebug()<<"TSC type : "<<type<<endl;
   tscType = type + 1;
   if(TSC_TYPE_300 == tscType)
       tsc300UIInit();
   else
       tsc500UIInit();
   if(NULL != commServ)
   {
      connect(commServ, SIGNAL(dataRecved(QByteArray*)), this, SLOT(processDatagram(QByteArray*)));
      msgDispatcher(FTEST_MSG_CONNECT, (char *)&tscType, 4);
   }

}
void IntergrateHandle::msgDispatcher(unsigned int type, char *data, int len)
{
    //ST_UDP_MSG *msgsend = (ST_UDP_MSG *)((new QByteArray(sizeof(ST_UDP_MSG), 0))->data());
    QByteArray *msgbuff =  new QByteArray(len + 12, 0);
    ST_UDP_MSG *msgsend = (ST_UDP_MSG *)(msgbuff->data());
    msgsend->uVer = FTEST_PRO_VER2;
    msgsend->uHead = FTEST_UDP_HEAD;
    msgsend->uTscid = tscID;
    switch (type) {
        case FTEST_MSG_CONNECT:
            msgsend->uType = FTEST_MSG_CONNECT;
            break;
        case FTEST_MSG_USB:
            msgsend->uType = FTEST_MSG_USB;
            break;
        case FTEST_MSG_RS232:
            msgsend->uType = FTEST_MSG_RS232;
            break;
        case FTEST_MSG_RS422:
            msgsend->uType = FTEST_MSG_RS422;
            break;
         case FTEST_MSG_RS485:
            msgsend->uType = FTEST_MSG_RS485;
            break;
         case FTEST_MSG_CURVOLT:
            msgsend->uType = FTEST_MSG_CURVOLT;
            break;
         case FTEST_MSG_WIFI:
            msgsend->uType = FTEST_MSG_WIFI;
            break;
         case FTEST_MSG_AUTO:
            msgsend->uType = FTEST_MSG_AUTO;
            break;
         case FTEST_MSG_LAMP:
            msgsend->uType = FTEST_MSG_LAMP;
            break;
         case FTEST_MSG_FRONTBOARD:
            msgsend->uType = FTEST_MSG_FRONTBOARD;
            break;
         case FTEST_MSG_WIRELESS:
            msgsend->uType = FTEST_MSG_WIRELESS;
            break;
         case FTEST_MSG_PEDKEY:
            msgsend->uType = FTEST_MSG_PEDKEY;
            break;
         case FTEST_MSG_WIRELESS_DATA:
            msgsend->uType = FTEST_MSG_WIRELESS_DATA;
            break;

         case FTEST_MSG_IOOUTPUT_H:
            msgsend->uType = FTEST_MSG_IOOUTPUT_H;
            break;
         case FTEST_MSG_IOOUTPUT_L:
            msgsend->uType = FTEST_MSG_IOOUTPUT_L;
            break;
         case FTEST_MSG_IOINPUT:
            msgsend->uType = FTEST_MSG_IOINPUT;
            break;
         case FTEST_MSG_CAR_DETECTOR:
            msgsend->uType = FTEST_MSG_CAR_DETECTOR;
            break;
         case FTEST_MSG_GPS:
            msgsend->uType = FTEST_MSG_GPS;
            break;
         case FTEST_MSG_KEYBOARD:
            msgsend->uType = FTEST_MSG_KEYBOARD;
            break;
         case FTEST_MSG_APP_EXIT:
            msgsend->uType = FTEST_MSG_APP_EXIT;
            break;
        default:
            msgsend->uType = FTEST_MSG_NONE;
            break;
    }
    if(data != NULL && len != 0)
        memcpy((char *)msgsend->data, data, len);
    commServ->sendData(msgbuff->data(), msgbuff->length());
}
void IntergrateHandle::processDatagram(QByteArray *datagram)
{
    int buttonIndex = -1;
    ST_UDP_MSG *msg = (ST_UDP_MSG*)datagram->data();

    //qDebug()<<"==>recv len:"<<datagram->length()<<",ver:"<<msg->uVer<<", head:"<<msg->uhead<<", type:"<<msg->uType<<", data:"<<(char*)msg->data<<endl;
    if(msg->uVer == FTEST_PRO_VER2 && msg->uHead == FTEST_UDP_HEAD)
    {
       switch(msg->uType)
       {
         case FTEST_MSG_CONNECT:
           if(MSG_EX_SUCC == msg->data[0])
                ConnectionAccepted(true);
           else
                ConnectionAccepted(false);

            tscID = msg->uTscid;
           break;
         case FTEST_MSG_REQ_RECONNECT:
           tscID = msg->uTscid;
           restart();
           msgDispatcher(FTEST_MSG_CONNECT, (char *)&tscType, 4);
           break;
         case FTEST_MSG_USB:
           //qDebug()<<"USB flag:"<<msg->data[0]<<endl;
           buttonIndex = 0;
           break;
         case FTEST_MSG_RS232://500
           buttonIndex = 1;
           break;
         case FTEST_MSG_RS422://500
           buttonIndex = 2;
           break;
         case FTEST_MSG_RS485:
           buttonIndex = (tscType == TSC_TYPE_300 ? 1 : 3);
           break;
         case FTEST_MSG_CURVOLT:
           buttonIndex = (tscType == TSC_TYPE_300 ? 10 : 12);
           //qDebug()<<"===>recv response"<<endl;
           break;
         case FTEST_MSG_WIFI:
           buttonIndex = (tscType == TSC_TYPE_300 ? 2 : 4);
           break;
         case FTEST_MSG_AUTO:
           buttonIndex = (tscType == TSC_TYPE_300 ? 3 : 5);
           break;
         case FTEST_MSG_LAMP:
           buttonIndex = (tscType == TSC_TYPE_300 ? 11 : 13);
           qDebug()<<"===>recv response"<<endl;
           break;
         case FTEST_MSG_FRONTBOARD://300
           buttonIndex = 5;
           break;
         case FTEST_MSG_GPS:
           buttonIndex = -1;//6
           break;
         case FTEST_MSG_WIRELESS://300
           buttonIndex = -1;//7;
           break;
         case FTEST_MSG_WIRELESS_DATA://300
           //update wireless button state
           //send signal
           if(msg->data[0] > 0 && msg->data[0] < 6)
               wirelessButtonClicked(msg->data[0]);
           buttonIndex = 9;
           msg->data[0] = MSG_EX_SUCC;
           break;
         case FTEST_MSG_PEDKEY://300
           buttonIndex = 8;
           break;
         case FTEST_MSG_KEYBOARD:
           buttonIndex = -1;
           break;
         case FTEST_MSG_KEYBOARD_DATA:
           buttonIndex = 9;
           break;
         case FTEST_MSG_IOOUTPUT_H://500
         case FTEST_MSG_IOOUTPUT_L://500
           buttonIndex = -1;
           break;
         case FTEST_MSG_IOINPUT://500
           if(MSG_EX_SUCC == msg->data[0])
               ioInputChangeState(true);
           else
               ioInputChangeState(false);
           buttonIndex = -1;
           break;
         case FTEST_MSG_IOINPUT_DATA:
            {
                char states[24] = {1};
                QList<int> list;
                int i = 0;
                memcpy(states, (char *)msg->data, 32);
                while(i < 24)
                {
                   list.append(states[i]);
                   i++;
                }
                QVariant v = QVariant::fromValue(list);
                IoInputUpdateStatus(v);
                writeLog((char *)&msg->data[6]);
                //qDebug()<<"==> update IOInput status..."<<endl;
            }
           break;
         case FTEST_MSG_CAR_DETECTOR://500
           buttonIndex = 15;
           break;
         default:
           buttonIndex = -1;
           break;
       }
       if(tscID != msg->uTscid)
       {
            qDebug()<<"Impossible thing."<<endl;
       }
       if((msg->data[0] == MSG_EX_SUCC || msg->data[0] == MSG_EX_FAIL) && buttonIndex != -1)
       {
#if 0
          if(buttonIndex <= 7)//500 & 300
          {
              if(7 == buttonIndex && TSC_TYPE_300 == tscType)//wireless
              {
                 //interface change
                  /*
                  bool checkstate = wirelessButton->property("checkstate").toBool();
                  if(checkstate)
                      wirelessStateChage(true);
                  else
                      wirelessStateChage(false);
                      */
              }
              else//300: 1-6 ; 500 1-7
              {
                  writeLog((char *)&msg->data[1]);
                  updateButtonState(buttonIndex, msg->data[0]);
                  //qDebug()<<"Response: "<<msg->data[0]<<",str:"<<(char *)msg->data[1]<<endl;
              }
          }
#endif
          if(buttonIndex == 8)//300 - ped button
          {
              int states[8]={0};
              QString buttonlog;
              memcpy(states, &msg->data[1], 8*4);
              QList<int> list;
              int i=0;
              while(i < 8)
              {
                  list.append(states[i]);
                  if(!lastButtonStatus.isEmpty() && list[i] != lastButtonStatus[i])//state changed
                  {
                      //QString str = "行人按钮"+(i+1)+(list[i] == 1 ? "按下" : "松开")+"!\n";
                      char buff[256] = {0};
                      sprintf(buff, "行人按钮%d%s!\n", (i+1), (list[i] == 1 ? "按下" : "松开"));
                      //qDebug()<<"---> 00 update: "<<buff<<endl;
                      buttonlog.append(buff);
                  }
                  i++;
              }
              QVariant v = QVariant::fromValue(list);
              //lastButtonStatus = list;
              lastButtonStatus.clear();
              lastButtonStatus.append(list);
              //qDebug()<<"---> 01lastButtonStatus:"<<lastButtonStatus[0]<<lastButtonStatus[1]<<lastButtonStatus[2]<<lastButtonStatus[3]<<lastButtonStatus[4]<<lastButtonStatus[5]<<lastButtonStatus[6]<<lastButtonStatus[7]<<endl;
              updatePedKeys(v);
              writeLog(buttonlog.toUtf8().data());
          }
          else if(buttonIndex == 9) //300
              writeLog((char *)&msg->data[1]);
          else if(buttonIndex == 15) //500
          {
              int states[48]={0};
              QString carDetlog;
              memcpy(states, &msg->data[1], 48*4);
              QList<int> list;
              int i=0;
              while(i < 48)
              {
                  list.append(states[i]);
                  if(states[i] == 1)
                  {
                      char buff[128] = {0};
                      sprintf(buff, "通道%02d有过车!\n", (i+1));
                      carDetlog.append(buff);
                  }
                  i++;
              }
              QVariant v = QVariant::fromValue(list);
              updateCarDetectors(v);
              //updateCarDetectors(list);
              writeLog(carDetlog.toUtf8().data());
          }
          else
          {
              writeLog((char *)&msg->data[1]);
              updateButtonState(buttonIndex, msg->data[0]);
          }
       }
    }
}
/*********************** 300 functions *****************************/
void IntergrateHandle::tsc300UIInit()
{
    QObject *parent = rootQml->findChild<QObject*>("tsc300Test");
    if(parent == NULL)
    {
        qDebug()<<"Tsc300 parent qml is NULL!"<<endl;
        return;
    }
    QObject::connect(this, SIGNAL(updateButtonState(QVariant, QVariant)), parent, SLOT(setButtonColor(QVariant, QVariant)));
    QObject::connect(this, SIGNAL(restart()), parent, SLOT(init()));

    QObject *usbButton = parent->findChild<QObject *> ("usbButton");
    if(usbButton)
        connect(usbButton, SIGNAL(clicked()), this, SLOT(slotUSBButton()));

    QObject *rs485Button = parent->findChild<QObject *> ("rs485Button");
    if(rs485Button)
        connect(rs485Button, SIGNAL(clicked()), this, SLOT(slotRS485Button()));

/*    QObject *curVoltButton = parent->findChild<QObject *> ("curVoltButton");
    if(curVoltButton)
        connect(curVoltButton, SIGNAL(clicked()), this, SLOT(slotCurVoltButton()));
*/
    QObject *wifi3GButton = parent->findChild<QObject *> ("wifi3GButton");
    if(wifi3GButton)
        connect(wifi3GButton, SIGNAL(clicked()), this, SLOT(slotWifi3GButton()));

    QObject *autoTestButton = parent->findChild<QObject *> ("autoTestButton");
    if(autoTestButton)
        connect(autoTestButton, SIGNAL(clicked()), this, SLOT(slotAutoTestButton()));

    QObject *lampCtrlButton = parent->findChild<QObject *> ("lampCtrlPage");
    if(lampCtrlButton)
        connect(lampCtrlButton, SIGNAL(buttonClicked(QVariant, QVariant)), this, SLOT(slotLampCtrlButton(QVariant, QVariant)));

    QObject *frontBoardButton = parent->findChild<QObject *> ("frontBoardButton");
    if(frontBoardButton)
        connect(frontBoardButton, SIGNAL(clicked()), this, SLOT(slotFrontBoardButton()));

    gpsButton = parent->findChild<QObject *> ("gpsButton");
    if(gpsButton)
        connect(gpsButton, SIGNAL(clicked()), this, SLOT(slotGPSButton()));

    wirelessButton = parent->findChild<QObject *> ("wirelessButton");
    if(wirelessButton)
    {
        connect(wirelessButton, SIGNAL(clicked()), this, SLOT(slotWirelessButton()));
        //connect(this, SIGNAL(wirelessStateChage(QVariant)), wirelessButton, SLOT(changeState(QVariant)));
    }

    QObject * wirelessState = parent->findChild<QObject *> ("wirelessState");
    if(wirelessState)
        connect(this, SIGNAL(wirelessButtonClicked(QVariant)), wirelessState, SLOT(showButtonClicked(QVariant)));

    pedKeysButton = parent->findChild<QObject *> ("pedKeysButton");
    if(pedKeysButton)
        connect(pedKeysButton, SIGNAL(clicked()), this, SLOT(slotPedKeysButton()));
    outputInfo = parent->findChild<QObject *> ("outputText");

    QObject * pedButtons = parent->findChild<QObject *> ("pedButtons");
    if(pedButtons)
        connect(this, SIGNAL(updatePedKeys(QVariant)), pedButtons, SLOT(updateButtonCheckState(QVariant)));

    keyBoardButton = parent->findChild<QObject *> ("keyBoardButton");
    if(keyBoardButton)
        connect(keyBoardButton, SIGNAL(clicked()), this, SLOT(slotKeyBoardButton()));
}
void IntergrateHandle::slotUSBButton()
{
    //qDebug()<<"UsbTest button clicked..."<<endl;
    msgDispatcher(FTEST_MSG_USB, NULL, 0);
}
void IntergrateHandle::slotRS485Button()
{
    //qDebug()<<"RS485Test button clicked..."<<endl;
    msgDispatcher(FTEST_MSG_RS485, NULL, 0);
}
/*
void IntergrateHandle::slotCurVoltButton()
{
    //qDebug()<<"CurVoltTest button clicked..."<<endl;
    msgDispatcher(FTEST_MSG_CURVOLT, NULL, 0);
}
*/
void IntergrateHandle::slotWifi3GButton()
{
    //qDebug()<<"Wifi/3GTest button clicked..."<<endl;
    msgDispatcher(FTEST_MSG_WIFI, NULL, 0);
}
void IntergrateHandle::slotAutoTestButton()
{
    //qDebug()<<"AutoTest button clicked..."<<endl;
    msgDispatcher(FTEST_MSG_USB, NULL, 0);
    msgDispatcher(FTEST_MSG_RS485, NULL, 0);
    //msgDispatcher(FTEST_MSG_CURVOLT, NULL, 0);
    msgDispatcher(FTEST_MSG_WIFI, NULL, 0);
    //msgDispatcher(FTEST_MSG_AUTO, NULL, 0);
}
void IntergrateHandle::slotLampCtrlButton(QVariant buttonindex, QVariant checkstates)
{
    int states[6]={0};
    int i = 0;
    int type = FTEST_MSG_NONE;
    //qDebug()<<"LampCtrlTest button clicked..."<<endl;
    if(buttonindex.toInt() < 1 || buttonindex.toInt() > 2)
        return;
    QVariantList tmp = checkstates.toList();//(QVariantList)checkstates;
    for(i=0; i<6; i++)
       states[i] = tmp[i].toInt();
    qDebug()<<"states: "<<states[0]<<states[1]<<states[2]<<states[3]<<states[4]<<states[5]<<endl;
    if(buttonindex.toInt() == 1)
        type = FTEST_MSG_CURVOLT;
    else
        type = FTEST_MSG_LAMP;

    msgDispatcher(type, (char *)states, 6*4);
    //qDebug()<<"==>msg send..."<<endl;
}
void IntergrateHandle::slotFrontBoardButton()
{
    //qDebug()<<"FrontBoardTest button clicked..."<<endl;
    msgDispatcher(FTEST_MSG_FRONTBOARD, NULL, 0);
}
void IntergrateHandle::slotGPSButton()
{
    int val = 0;
    if(gpsButton->property("checkstate").toBool())
       val = 1;
    else
       val = 2;
    msgDispatcher(FTEST_MSG_GPS, (char *)&val, 4);
}
void IntergrateHandle::slotWirelessButton()
{
    //qDebug()<<"WirelessTest button clicked..."<<endl;
    int state = WIRELESS_CHECK_OFF;
    bool checkstate = wirelessButton->property("checkstate").toBool();
    if(checkstate)
        state = WIRELESS_CHECK_ON;
    //qDebug()<<"wireless send State: "<<state<<endl;
    msgDispatcher(FTEST_MSG_WIRELESS, (char *)&state, 4);
}
void IntergrateHandle::slotPedKeysButton()
{
    //qDebug()<<"PedKeysTest button clicked..."<<endl;
    bool checkstate = pedKeysButton->property("checkstate").toBool();
    if(checkstate)
    {
        connect(timer, SIGNAL(timeout()), this, SLOT(getPedKeysStatus()));
        timer->start(500);//500ms
    }
    else
    {
        disconnect(timer, SIGNAL(timeout()), this, SLOT(getPedKeysStatus()));
        timer->stop();
    }
}
void IntergrateHandle::getPedKeysStatus()
{
    msgDispatcher(FTEST_MSG_PEDKEY, NULL, 0);
}
void IntergrateHandle::slotKeyBoardButton()
{
    int val = 0;
    if(keyBoardButton->property("checkstate").toBool())
       val = 1;
    else
       val = 2;
    msgDispatcher(FTEST_MSG_KEYBOARD, (char *)&val, 4);

}
/*********************** end of 300 functions ***********************/

/*************************** 500 functions ***************************/
void IntergrateHandle::tsc500UIInit()
{
    obTsc500= rootQml->findChild<QObject*>("tsc500Test");
    if(NULL == obTsc500)
    {
        qDebug()<<"Tsc500 parent qml is NULL!"<<endl;
        return;
    }
    connect(obTsc500, SIGNAL(clicked(int)), this, SLOT(slotButtonClicked(int)));
    QObject::connect(this, SIGNAL(updateButtonState(QVariant, QVariant)), obTsc500, SLOT(setButtonColor(QVariant, QVariant)));
    outputInfo = obTsc500->findChild<QObject *> ("outputText500");

    QObject *ioOutputPage = obTsc500->findChild<QObject*>("IOOutputPage");
    if(ioOutputPage)
        connect(ioOutputPage, SIGNAL(buttonClicked(int)), this, SLOT(slotButtonClicked(int)));
    ioInputButton = obTsc500->findChild<QObject*>("ioInputButton");
    if(ioInputButton)
        connect(this, SIGNAL(ioInputChangeState(QVariant)), ioInputButton, SLOT(changeState(QVariant)));

    QObject *ioInputPage = obTsc500->findChild<QObject*>("IOInputPage");
    if(ioInputPage)
        connect(this, SIGNAL(IoInputUpdateStatus(QVariant)), ioInputPage, SLOT(updateIOStatus(QVariant)));

    carDetectorButton = obTsc500->findChild<QObject*>("carDetectorButton");
    carDetPage = obTsc500->findChild<QObject*>("CarDetPage");
    if(carDetPage)
       connect(this, SIGNAL(updateCarDetectors(QVariant)), carDetPage, SLOT(updateCarDetStatus(QVariant)));

    outputInfo = obTsc500->findChild<QObject *> ("outputText500");

}
void IntergrateHandle::slotButtonClicked(int buttonIndex)
{
    int state[6] = {0};
    int len = 0;
    unsigned int msgType = FTEST_MSG_NONE;
    switch (buttonIndex) {
    case 1://usb
        msgType = FTEST_MSG_USB;
        break;
    case 2://232
        msgType = FTEST_MSG_RS232;
        break;
    case 3://422
        msgType = FTEST_MSG_RS422;
        break;
    case 4://485
        msgType = FTEST_MSG_RS485;
        break;
    //case 5://cur&volt
    //    msgType = FTEST_MSG_CURVOLT;
    //    break;
    case 5://wifi/3G
        msgType = FTEST_MSG_WIFI;
        break;
    case 6://auto
        msgType = FTEST_MSG_AUTO;
        msgDispatcher(FTEST_MSG_USB, NULL, 0);
        msgDispatcher(FTEST_MSG_RS232, NULL, 0);
        msgDispatcher(FTEST_MSG_RS422, NULL, 0);
        msgDispatcher(FTEST_MSG_RS485, NULL, 0);
        //msgDispatcher(FTEST_MSG_CURVOLT, NULL, 0);
        msgDispatcher(FTEST_MSG_WIFI, NULL, 0);
        break;
    case 7://cur&volt
    case 8://lampctrl
    {
        msgType = (buttonIndex == 7 ?  FTEST_MSG_CURVOLT : FTEST_MSG_LAMP);
        QObject *lampCtrlPage = obTsc500->findChild<QObject*>("lampCtrlPage");
        if(lampCtrlPage)
        {
            int i = 0;
            QVariantList tmp =  lampCtrlPage->property("states").toList();
            for(i=0; i<6; i++)
                state[i] = tmp[i].toInt();
            len = 6*4;
        }
        qDebug()<<"states: "<<state[0]<<state[1]<<state[2]<<state[3]<<state[4]<<state[5]<<endl;
    }
        break;
    case 9://IO Output ==> check sub button : 51&52
        break;
    case 10://GPS
    {
        msgType = FTEST_MSG_GPS;
        QObject *obGps =  obTsc500->findChild<QObject*>("gpsButton");
        if(obGps)
        {
            if(obGps->property("checkstate").toBool())
                state[0] = 1;
            else
                state[0] = 2;
            len = 4;
        }
    }
        break;
    case 11://IO Input
        msgType = FTEST_MSG_IOINPUT;
        state[0] = IOINPUT_CHECK_OFF;
        if(ioInputButton->property("checkstate").toBool())
           state[0] = IOINPUT_CHECK_ON;
        len = 4;
        break;
    case 12://car detector
        {
            bool checkstate = carDetectorButton->property("checkstate").toBool();
            //qDebug()<<" ===>checkstate: "<<checkstate<<endl;
            if(checkstate)
            {
                qDebug()<<"===> Timer start..."<<endl;
                connect(timer, SIGNAL(timeout()), this, SLOT(getCarDetectorStatus()));
                timer->start(1000);//500ms
            }
            else
            {
                disconnect(timer, SIGNAL(timeout()), this, SLOT(getCarDetectorStatus()));
                timer->stop();
            }
        }
        break;
    case 13://keyboard
    {
        msgType = FTEST_MSG_KEYBOARD;
        QObject *obKeyboard =  obTsc500->findChild<QObject*>("keyBoardButton");
        if(obKeyboard)
        {
            if(obKeyboard->property("checkstate").toBool())
                state[0] = 1;
            else
                state[0] = 2;
            len = 4;
        }
    }
        break;
     case 51://io Output Hight
        msgType = FTEST_MSG_IOOUTPUT_H;
        break;
     case 52://io output low
        msgType = FTEST_MSG_IOOUTPUT_L;
       break;
    default:
        break;
    }
    if(FTEST_MSG_NONE != msgType && FTEST_MSG_AUTO != msgType)
        msgDispatcher(msgType, (char *)state, len);
}
void IntergrateHandle::getCarDetectorStatus()
{
   msgDispatcher(FTEST_MSG_CAR_DETECTOR, NULL, 0);
}
/************************* end of 500 functions *********************/

/************************* for log file *****************************/
void IntergrateHandle::LogFileInit(QString orderNum)
{
    /*QString logFilePath = QDir::currentPath();
    QString logFilePath = QCoreApplication::applicationDirPath();
    logFilePath = QCoreApplication::applicationFilePath();
    qDebug()<<"recv log path: "<<logFilePath<<endl;
    QString logFilePath("300.log");
    TSC300_2017-05-19_10-33-45.log*/
//    QString logFileName = (tscType == TSC_TYPE_300 ? QString("TSC300_") : QString("TSC500_"));
//    logFileName.append(QDate::currentDate().toString("yyyy-MM-dd"));
//    logFileName.append("_");
//    logFileName.append(QTime::currentTime().toString("hh-mm-ss"));

    if(orderNum.isEmpty())
        return;
    QString logFileName = orderNum;
    logFileName.append((tscType == TSC_TYPE_300 ? QString("_TSC300") : QString("_TSC500")));
    logFileName.append(".log");
    logFile = new QFile(logFileName);
    if(!logFile || !logFile->open(QIODevice::WriteOnly | QIODevice::Text |QIODevice::Append))
    {
        qDebug()<<"打开日志文件失败！"<<endl;
        return;
    }
    logFile->write(QDate::currentDate().toString("yyyy/MM/dd dddd").toUtf8().data());
    logFile->write(QTime::currentTime().toString(" hh:mm:ss").toUtf8().data());
    logFile->write("\n");
}
void IntergrateHandle::writeLog(const char *log)
{
    int length = outputInfo->property("length").toInt();
    //qDebug()<<"Current text Len: "<<length<<" canConvert:"<<length.canConvert<int>()<<" type:"<<length.type()<<endl;
    //日志显示
    QMetaObject::invokeMethod(outputInfo, "insert", Qt::DirectConnection, Q_ARG(int, length), Q_ARG(QString, QString(log)));
    //outputInfo->setProperty("text", (char*)&msg->data[1]);
    //写日志
    if(logFile)
    {
        logFile->write(log, qstrlen(log));
        logFile->flush();
    }
}
void IntergrateHandle::LogClear()
{
    QMetaObject::invokeMethod(outputInfo, "clear", Qt::DirectConnection);
}
void IntergrateHandle::LogFileClose()
{
   if(logFile)
       logFile->close();
}
/*************************** end of log ********************************/
//void IntergrateHandle::appExit(QCloseEvent* ev)
void IntergrateHandle::appExitHandle()
{
   msgDispatcher(FTEST_MSG_APP_EXIT, NULL, 0);
   QCoreApplication::exit();
}

