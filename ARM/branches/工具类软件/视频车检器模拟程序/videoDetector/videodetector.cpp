#include "videodetector.h"
#include "realtimedialog.h"
#include "interdialog.h"
#include "perioddialog.h"
#include <QSpacerItem>
#include <QMessageBox>
#include <QHostAddress>
//#include <QTcpServer>
//#include <QMovie>
#include "data.h"
#include "common.h"
#include "tcpclient.h"
#include <QTime>
#include <QDate>
#include <QDebug>
#include <QMenu>
#include <QMenuBar>
//#include <QIcon>
#include <QPixmap>
#include <QHBoxLayout>
#include <QFile>
#include <QDataStream>
//#include <QTranslator>
#include <QApplication>


VideoDetector::VideoDetector(QWidget *parent)
    : QDialog(parent)
{
    cLanguage = C_LANGUAGE_CN;
    tLanguage = new QTranslator;
    setLanguage(LANGUAGE_CHINESE);

    setWindowTitle(tr("VDTitle"));
    setWindowFlags(Qt::Widget);
    /*****data init*****/
    QString *sIP;
    iTimeCounter = 0;
    alarmInfo10.byLane = 1;
    alarmInfo10.dwDeviceID = 1;
    realTime20.byLane = 1;
    realTime20.wDeviceID = 1;
    interTime20.wDeviceID = 1;
    interTime20.wSamplePeriod = 60;
    realTime21.byLane = 1;
    realTime21.wDeviceID = 1;
    periodsInfo21.uDeviceID = 1;
    periodsInfo21.uSamplePeriod = 60;
    periodsInfo21.uTotoalLaneNum = 1;
    iProtocolVer = CAMERA_PROTOCOL21;
    sIP = loadConfig(CONF_FILE_NAME);
    qDebug()<<"IP:"<<sIP<<endl;
    /******************menu*******************/
    QMenuBar *mbMenu = new QMenuBar(this);
    mbMenu->setGeometry(0,0,this->width(),24);
    mSetting = mbMenu->addMenu(tr("Setting"));
    //1.0
    aAlarm10 = mSetting->addAction(tr("Alarm10"));
    mSetting->addSeparator();
    //2.0
    aRealTime20 = mSetting->addAction(tr("RealTime20"));
    aInterTime20 = mSetting->addAction(tr("InterTime20"));
    mSetting->addSeparator();
    //2.1
    aRealTime21 = mSetting->addAction(tr("RealAlarm21"));
    aPeriodsTime21 = mSetting->addAction(tr("PeriodAlarm21"));

    mNetwork = mbMenu->addMenu(tr("Network"));
    QActionGroup *agProtocol = new QActionGroup(this);
    aCP10 = mNetwork->addAction(tr("CP10"));
    aCP20 = mNetwork->addAction(tr("CP20"));
    aCP21 = mNetwork->addAction(tr("CP21"));
    aCP10->setCheckable(true);
    aCP20->setCheckable(true);
    aCP21->setCheckable(true);
    agProtocol->addAction(aCP10);
    agProtocol->addAction(aCP20);
    agProtocol->addAction(aCP21);
    aCP21->setChecked(true);
    connect(agProtocol, SIGNAL(triggered(QAction*)), this, SLOT(onProtocolChange(QAction *)));

    mHelp = mbMenu->addMenu(tr("Help"));
    mLanguage = mHelp->addMenu(tr("Language"));
    aChinese = mLanguage->addAction(tr("Chinese"));
    aEnglish = mLanguage->addAction(tr("English"));
    aChinese->setCheckable(true);
    aEnglish->setCheckable(true);
    QActionGroup *agLanguage = new QActionGroup(this);
    agLanguage->addAction(aChinese);
    agLanguage->addAction(aEnglish);
    aChinese->setChecked(true);
    //englishAction->setChecked(true);
    connect(agLanguage, SIGNAL(triggered(QAction*)), this, SLOT(onLanguageChange(QAction*)));
    aAbout = mHelp->addAction(tr("About"));

    connect(aAlarm10, SIGNAL(triggered(bool)), this, SLOT(onActionRealTimeTrigger()));//1.0
    connect(aRealTime20, SIGNAL(triggered(bool)), this, SLOT(onActionRealTimeTrigger()));//2.0
    connect(aInterTime20, SIGNAL(triggered(bool)), this, SLOT(onActionInterTrigger()));//2.0
    connect(aRealTime21, SIGNAL(triggered(bool)), this, SLOT(onActionRealTimeTrigger()));//2.1
    connect(aPeriodsTime21, SIGNAL(triggered(bool)), this, SLOT(onActionPeriodsTrigger()));//2.1
    connect(aAbout, SIGNAL(triggered(bool)), this, SLOT(onActionAbout()));
    /*****************Input********************/
    QHBoxLayout *hblFirstLine = new QHBoxLayout;
    hblFirstLine->setMargin(40);
    hblFirstLine->setSpacing(10);
    lIp = new QLabel(tr("IPaddr"));
    lPort = new QLabel(tr("Port"));
    leIp = new QLineEdit;
    if(sIP && !sIP->isEmpty())
        leIp->setText(*sIP);
    lePort = new QLineEdit;
    cbSC = new QCheckBox(tr("SignalCtrller"));
    hblFirstLine->addWidget(lIp);
    hblFirstLine->addWidget(leIp);
    hblFirstLine->addWidget(lPort);
    hblFirstLine->addWidget(lePort);
    hblFirstLine->addWidget(cbSC);

    pbStart = new QPushButton(tr("Start"));
    pbStop = new QPushButton(tr("Stop"));

    glMain = new QGridLayout(this);
//    QSpacerItem *siSpaceLine = new QSpacerItem(this->width()/2, this->height()/14);
//    glMain->addItem(siSpaceLine, 0, 0, 1, 5);
    int row=0;
    glMain->setMargin(10);
    glMain->setSpacing(10);
    glMain->addLayout(hblFirstLine, 0, 0);

    row = 0;
    QGridLayout *secondLayout = new QGridLayout;
    QSpacerItem *si2Column0 = new QSpacerItem(this->width()/12, this->height()/25);
    QSpacerItem *si2Column1 = new QSpacerItem(this->width()/10, this->height()/25);
    QSpacerItem *si2Column2 = new QSpacerItem(this->width()/10, this->height()/25);
    QSpacerItem *si2Column3 = new QSpacerItem(this->width()/10, this->height()/25);
    QSpacerItem *si2Column4 = new QSpacerItem(this->width()/12, this->height()/25);
    secondLayout->setMargin(5);
    secondLayout->setSpacing(3);
    lDetectorNum = new QLabel(tr("DetectorNum"));
    lSettingStatus = new QLabel(tr(""));
    lLight = new QLabel;
    pRed = new QPixmap(":/images/redlight.png");
    pGreen = new QPixmap(":/images/greenlight.png");
    lLight->setPixmap(*pRed);
    secondLayout->addItem(si2Column1, row, 0);
    secondLayout->addWidget(lDetectorNum, row, 1);
    secondLayout->addWidget(lSettingStatus, row, 2);
    secondLayout->addWidget(lLight, row, 3,3,1);
    row++;
    lVehicles = new QLabel(tr("Vehicles"));
    lStatisPks = new QLabel(tr("StatisPks"));
    lRealTime = new QLabel(tr("0"));
    lPeriods = new QLabel(tr("0"));
    secondLayout->addItem(si2Column2, row, 0);
    secondLayout->addWidget(lVehicles, row, 1);
    secondLayout->addWidget(lRealTime, row, 2);
    row++;
    secondLayout->addItem(si2Column3, row, 0);
    secondLayout->addWidget(lStatisPks, row, 1);
    secondLayout->addWidget(lPeriods, row, 2);
    row++;
    secondLayout->addItem(si2Column0, row, 0);
    row++;
    lMovie = new QLabel;
    lMovie->resize(300,150);
    secondLayout->addItem(si2Column4, row, 0, 1, 1);
    secondLayout->addWidget(lMovie, row, 1, 1, 2);
    row = 1;
    glMain->addLayout(secondLayout, row, 0);

    QHBoxLayout *thirdLayout = new QHBoxLayout;
    QSpacerItem *siColumn0 = new QSpacerItem(this->width()/8, this->height()/10);
    QSpacerItem *siColumn1 = new QSpacerItem(this->width()/3, this->height()/10);
    QSpacerItem *siColumn2 = new QSpacerItem(this->width()/8, this->height()/10);
    thirdLayout->addItem(siColumn0);
    thirdLayout->addWidget(pbStart);
    thirdLayout->addItem(siColumn1);
    thirdLayout->addWidget(pbStop);
    thirdLayout->addItem(siColumn2);
    glMain->addLayout(thirdLayout, 2,0);
//    QPushButton *pbtmp = new QPushButton(tr("tmp"));
//    connect(pbtmp, SIGNAL(clicked(bool)), this, SLOT(onButtonTmpClick()));
//    glMain->addWidget(pbtmp, row, 2);
//    glMain->addWidget(pbStop, row, 4, 1,1);

    //init button status
    cbSC->setChecked(true);
    onCBClicked(true);
    pbStop->setEnabled(false);
    connect(cbSC, SIGNAL(clicked(bool)), this, SLOT(onCBClicked(bool)));
    connect(pbStart, SIGNAL(clicked(bool)), this, SLOT(onPBStartClicked()));
    connect(pbStop, SIGNAL(clicked(bool)), this, SLOT(onPBStopClicked()));
    connect(this, SIGNAL(finished(int)), this, SLOT(OnAppOut()));

    qDebug()<<"---->"<<QCoreApplication::translate("VideoDetector","Setting")<<endl;
}
/*
void VideoDetector::onButtonTmpClick()
{
    ERR_MSG(this,QString::number(realTime.byLane)+"--"+QString::number(realTime.wDeviceID));
}
*/
VideoDetector::~VideoDetector()
{

}
void VideoDetector::onCBClicked(bool status)
{
   if(status)
   {
//     lPort->setVisible(false);
//      lePort->setVisible(false);
      lPort->setEnabled(false);
      lePort->setEnabled(false);
      lePort->setText(tr("****"));
   }
   else
   {
//      lPort->setVisible(true);
//     lePort->setVisible(true);
      lPort->setEnabled(true);
      lePort->setEnabled(true);
      lePort->setText(tr(""));
   }
}
void VideoDetector::onPBStartClicked()
{
    iRealtimeCounter = 0;

    tTimer = new QTimer(this);
    if(startWork())
        return;
    tTimer->start(500);//500ms

    //QMovie *mMovie = new QMovie(":/images/carflow.gif");
    //lMovie->setMovie(mMovie);
    //mMovie->start();
    iPeriodsCounter = 0;
    setWidgetStatus(false);
    dataFlowUpdate();
}
void VideoDetector::onPBStopClicked()
{
    if(iTimeCounter%2)
        connect2Server();
    tTimer->stop();
    delete tTimer;
    tTimer = NULL;

    setWidgetStatus(true);
}
void VideoDetector::setPictureStatus(bool status)
{
   if(status)
   {
      lLight->setPixmap(*pRed);
   }
   else
   {
      lLight->setPixmap(*pGreen);
   }
}
void VideoDetector::setWidgetStatus(bool status)
{
    if(status)//stop
    {
        pbStop->setEnabled(false);
       pbStart->setEnabled(true);
    }
    else//start
    {
        pbStop->setEnabled(true);
       pbStart->setEnabled(false);
    }
    lIp->setEnabled(status);
    leIp->setEnabled(status);
    if(CAMERA_PROTOCOL10 == iProtocolVer)
    {
        aAlarm10->setEnabled(status);
        aCP20->setEnabled(status);
        aCP21->setEnabled(status);

        lPeriods->setEnabled(status);
        lSettingStatus->setText(QString::number(alarmInfo10.byLane));
    }
    else if(CAMERA_PROTOCOL20 == iProtocolVer)
    {
        aRealTime20->setEnabled(status);
        aInterTime20->setEnabled(status);
        aCP10->setEnabled(status);
        aCP21->setEnabled(status);
        lSettingStatus->setText(QString::number(realTime20.byLane));
    }
    else if(CAMERA_PROTOCOL21 == iProtocolVer)
    {
        aRealTime21->setEnabled(status);
        aPeriodsTime21->setEnabled(status);
        aCP10->setEnabled(status);
        aCP20->setEnabled(status);
        lSettingStatus->setText(QString::number(realTime21.byLane));
    }
//    mSetting->setEnabled(status);
    if(!cbSC->isChecked())
    {
         lPort->setEnabled(status);
         lePort->setEnabled(status);
    }
    setPictureStatus(status);
}
void VideoDetector::dataFlowUpdate()
{
    QString tmp[]={"==> ==> ==> ==> ==> ==> ==> ==> ==> ==> ==> ==> ==>",
                   " ==> ==> ==> ==> ==> ==> ==> ==> ==> ==> ==> ==> ==",
                   "> ==> ==> ==> ==> ==> ==> ==> ==> ==> ==> ==> ==> =",
                   "=> ==> ==> ==> ==> ==> ==> ==> ==> ==> ==> ==> ==> "};
    lRealTime->setText(QString::number(iRealtimeCounter));
    lPeriods->setText(QString::number(iPeriodsCounter));

    lMovie->setText(tmp[(iRealtimeCounter+iPeriodsCounter)%4]);
}
int VideoDetector::startWork()
{
   QString ip = leIp->text();
   QString sPort = lePort->text();
   if(ip.isEmpty() || sPort.isEmpty())
   {
        QMessageBox::information(this, tr("Error"), tr("IP and Port can't be empty!"));
       return -1;
   }
   if(sPort.startsWith(tr("****")))
       servPort = 7200;
   else
       servPort = sPort.toInt();
   servIp = new QHostAddress(ip);
  connect(tTimer, SIGNAL(timeout()), this, SLOT(connect2Server()));
  return 0;
}
void VideoDetector::connect2Server()
{
   QByteArray baData;
   char ret=-1;
   char dtype = 0;
   int delay = 0;
   if(iTimeCounter%2)
       dtype = ALARM_REAL_TIME_LEAVE;
   else
       dtype = ALARM_REAL_TIME_ENTER;

   if(CAMERA_PROTOCOL20 == iProtocolVer)
       delay = interTime20.wSamplePeriod*2;
   else if(CAMERA_PROTOCOL21 == iProtocolVer)
       delay = periodsInfo21.uSamplePeriod*2;
   if(delay != 0 && iTimeCounter >= delay)
   {
       dtype = ALARM_STATISTICS;
       iTimeCounter = -1;
   }
   TcpClient *pClient = new TcpClient();
   connect(pClient, SIGNAL(connectionClose(char)), this, SLOT(slotClientReturn(char)));
   if((ret=getData(&baData, dtype)) != -1)
       pClient->setData(dtype, baData);
   pClient->tcpConnect(servIp, servPort);
   iTimeCounter++;
}
void VideoDetector::slotClientReturn(char type)
{
    if(ALARM_REAL_TIME_LEAVE == type)
        iRealtimeCounter++;
    if(ALARM_STATISTICS == type)
        iPeriodsCounter++;
//    qDebug()<< "---->counter1: "<<iRealtimeCounter<<endl;
    dataFlowUpdate();
}
void VideoDetector::setRealTimeInfo(char *buff, char type)
{
    QTime time = QTime::currentTime();
    QDate date = QDate::currentDate();
    qsrand(time.second()+time.msec()*1000);
    int speed = qrand()%125;
    int lane = 0;
    int deviceID =0;

    if(CAMERA_PROTOCOL20 == iProtocolVer)
    {
        lane = realTime20.byLane;
        deviceID = realTime20.wDeviceID;
    }
    else if(CAMERA_PROTOCOL21 == iProtocolVer)
    {
        lane = realTime21.byLane;
        deviceID = realTime21.wDeviceID;
    }
    INTER_TPS_TIME *stTime = (INTER_TPS_TIME*)buff;
    stTime->wYear = htons(date.year());
    stTime->byMonth = date.month();
    stTime->byDay = date.day();
    stTime->byHour = time.hour();
    stTime->byMinute = time.minute();
    stTime->bySecond = time.second();
    stTime->wMilliSec = htons(time.msec());
    NET_DVR_TPS_REAL_TIME_INFO *stRealTime = (NET_DVR_TPS_REAL_TIME_INFO*)(buff+ sizeof(INTER_TPS_TIME));
    stRealTime->byLane = lane;
    stRealTime->byLaneState = 0x01;
    stRealTime->wDeviceID = htons(deviceID);
    if(type == ALARM_REAL_TIME_ENTER)
        stRealTime->byCMD = 0x01;//01-enter, 02-leave, 03-congestion
    else if(type == ALARM_REAL_TIME_LEAVE)
        stRealTime->byCMD = 0x02;//01-enter, 02-leave, 03-congestion
    else
        stRealTime->byCMD = 0x03;//01-enter, 02-leave, 03-congestion
    stRealTime->bySpeed = speed;
}
void VideoDetector::GetRealTimeAlarmData(QByteArray *baBuff, char type)
{
    int length=0;
    if(CAMERA_PROTOCOL10 == iProtocolVer)//1.0
    {
       length = sizeof(INTER_DVR_REQUEST_HEAD_V30)+sizeof(NET_TPS_ALARM);
       baBuff->resize(length);

       INTER_DVR_REQUEST_HEAD_V30 *header = (INTER_DVR_REQUEST_HEAD_V30*)baBuff->data();
       NET_TPS_ALARM *realAlarm =(NET_TPS_ALARM*)(baBuff->data()+sizeof(INTER_DVR_REQUEST_HEAD_V30));
       header->dwLength = sizeof(INTER_DVR_REQUEST_HEAD_V30) + sizeof(NET_TPS_ALARM);
       header->byCommand = 0x3a;
       header->byIPType = 0;
       strcpy(header->sDVRName, "HIK Detector");

       realAlarm->dwDeviceId = htons(alarmInfo10.dwDeviceID);
       if(alarmInfo10.byLane >0 && alarmInfo10.byLane <=8)
       {
           NET_LANE_PARAM *lane = &realAlarm->struTPSInfo.struLaneParam[alarmInfo10.byLane-1];
           lane->byRuleID = alarmInfo10.byLane-1;
           if(type == ALARM_REAL_TIME_ENTER)
               lane->dwVaryType = htons(0x01);//01-enter, 02-leave, 03-congestion
           else if(type == ALARM_REAL_TIME_LEAVE)
               lane->dwVaryType = htons(0x02);//01-enter, 02-leave, 03-congestion
           else
               lane->dwVaryType = htons(0x03);//01-enter, 02-leave, 03-congestion
       }
    }
    else if(CAMERA_PROTOCOL20 == iProtocolVer)//2.0
    {
        length = sizeof(INTER_DVR_REQUEST_HEAD_V30)+sizeof(NET_DVR_TPS_REAL_TIME_ALARM);
        baBuff->resize(length);

        //enter
        INTER_DVR_REQUEST_HEAD_V30 *header = (INTER_DVR_REQUEST_HEAD_V30*)baBuff->data();
        NET_DVR_TPS_REAL_TIME_ALARM *realtimeAlarm =(NET_DVR_TPS_REAL_TIME_ALARM*)(baBuff->data()+sizeof(INTER_DVR_REQUEST_HEAD_V30));
        header->dwLength = sizeof(INTER_DVR_REQUEST_HEAD_V30) + sizeof(NET_DVR_TPS_REAL_TIME_ALARM);
        header->byCommand = 0xb6;
        header->byIPType = 0;
        strcpy(header->sDVRName, "HIK CAMERA");
        setRealTimeInfo((char*)&realtimeAlarm->struTime, type);
    }
    else if(CAMERA_PROTOCOL21 == iProtocolVer)//2.1
    {
        length = sizeof(NET_TPS_ALARM_REALTIME);
        baBuff->resize(length);
        NET_TPS_ALARM_REALTIME *realTimeAlarm = (NET_TPS_ALARM_REALTIME*)baBuff->data();

        realTimeAlarm->struHead.byType = 0xb6;
        realTimeAlarm->struHead.dwLength = sizeof(NET_TPS_ALARM_REALTIME);
        setRealTimeInfo((char*)&realTimeAlarm->struTime, type);
    }
}
void VideoDetector::GetInterAlarmData(QByteArray *baBuff)
{
    QTime time = QTime::currentTime();
    QDate date = QDate::currentDate();
    qsrand(time.second()+time.msec()*1000);

    if(CAMERA_PROTOCOL20 == iProtocolVer)//2.0
    {
        baBuff->resize(sizeof(INTER_DVR_REQUEST_HEAD_V30) + sizeof(NET_DVR_TPS_INTER_TIME_ALARM));
        memset(baBuff->data(), 0, sizeof(INTER_DVR_REQUEST_HEAD_V30) + sizeof(NET_DVR_TPS_INTER_TIME_ALARM));

         INTER_DVR_REQUEST_HEAD_V30 *header = (INTER_DVR_REQUEST_HEAD_V30*)baBuff->data();
         NET_DVR_TPS_INTER_TIME_ALARM *interinfo =(NET_DVR_TPS_INTER_TIME_ALARM*)(baBuff->data()+sizeof(INTER_DVR_REQUEST_HEAD_V30));
         header->dwLength = sizeof(INTER_DVR_REQUEST_HEAD_V30) + sizeof(NET_DVR_TPS_INTER_TIME_ALARM);
         header->byCommand = 0xb7;
         header->byIPType = 0;
         strcpy(header->sDVRName, "HIK CAMERA");

         interinfo->struVerHead.byVersion = 2;
         interinfo->dwChan = htonl(32);
         interinfo->struTPSInterTimeInfo.byStart=0x11;
         interinfo->struTPSInterTimeInfo.byCMD=0x08;
         interinfo->struTPSInterTimeInfo.wDeviceID=htons(interTime20.wDeviceID);
         interinfo->struTPSInterTimeInfo.wDataLen=htons(0x31);
         interinfo->struTPSInterTimeInfo.byTotalLaneNum=4;
         interinfo->struTPSInterTimeInfo.dwSamplePeriod=htonl(interTime20.wSamplePeriod);

         interinfo->struTPSInterTimeInfo.struStartTime.wYear = htons(date.year());
         interinfo->struTPSInterTimeInfo.struStartTime.byMonth = date.month();
         interinfo->struTPSInterTimeInfo.struStartTime.byDay = date.day();
         interinfo->struTPSInterTimeInfo.struStartTime.byHour = time.hour();
         interinfo->struTPSInterTimeInfo.struStartTime.byMinute = time.minute();
         interinfo->struTPSInterTimeInfo.struStartTime.bySecond = time.second();
         interinfo->struTPSInterTimeInfo.struStartTime.wMilliSec = htons(time.msec());

         int lane = qrand();
         for(int i =0; i<8; i++)
         {
             int rand = qrand();
             NET_DVR_LANE_PARAM *statisticsParam = &interinfo->struTPSInterTimeInfo.struLaneParam[i];
             statisticsParam->byLane = lane%43+i;
             statisticsParam->bySpeed = rand%120;
             statisticsParam->dwLaneVolume = htonl(rand%50);
             statisticsParam->dwTimeHeadway = htonl(rand%30);
             statisticsParam->dwSpaceHeadway = htonl(rand%50);
             statisticsParam->wSpaceOccupyRation = htons(rand%542);
             statisticsParam->wTimeOccupyRation = htons(rand%243);
         }
    }
    else if(CAMERA_PROTOCOL21 == iProtocolVer)
    {
       baBuff->resize(sizeof(NET_TPS_ALARM_INTER_TIME));
       memset(baBuff->data(), 0, sizeof(NET_TPS_ALARM_INTER_TIME));
       NET_TPS_ALARM_INTER_TIME *interInfo = (NET_TPS_ALARM_INTER_TIME*)baBuff->data();
       interInfo->struHead.byType = 0xb7;
       interInfo->struHead.dwLength = sizeof(NET_TPS_ALARM_INTER_TIME);

       interInfo->struTPSInterInfo.struStartTime.wYear = htons(date.year());
       interInfo->struTPSInterInfo.struStartTime.byMonth = date.month();
       interInfo->struTPSInterInfo.struStartTime.byDay = date.day();
       interInfo->struTPSInterInfo.struStartTime.byHour= time.hour();
       interInfo->struTPSInterInfo.struStartTime.byMinute = time.minute();
       interInfo->struTPSInterInfo.struStartTime.bySecond= time.second();
       interInfo->struTPSInterInfo.struStartTime.wMilliSec = htons(time.msec());

       interInfo->struTPSInterInfo.byCMD = 0x08;
       interInfo->struTPSInterInfo.byStart = 0xfe;
       interInfo->struTPSInterInfo.wDeviceID = htons(periodsInfo21.uDeviceID);
       interInfo->struTPSInterInfo.byTotalLaneNum = periodsInfo21.uTotoalLaneNum;
       interInfo->struTPSInterInfo.wDataLen = htons(0x31);
       interInfo->struTPSInterInfo.dwSamplePeriod = htonl(periodsInfo21.uSamplePeriod);

       int lane = qrand();
       for(int i=0; i<periodsInfo21.uTotoalLaneNum; i++)
       {
           int rand = qrand();
           NET_TPS_LANE_PARAM *lane0 = &interInfo->struTPSInterInfo.struLaneParam[i];
           lane0->byLane = lane%43+i;
           lane0->bySpeed = rand%125;
           lane0->dwLightVehicle = htonl(rand%50);
           lane0->dwMidVehicle= htonl(rand%30);
           lane0->dwHeavyVehicle= htonl(rand%20);
           lane0->dwTimeHeadway=htonl(rand%10);
           lane0->dwSpaceHeadway=htonl(rand%20);
           lane0->wTimeOccupyRation= htons(10+rand%350);
           lane0->wSpaceOccupyRation= htons(20+rand%500);
       }
    }
}
char VideoDetector::getData(QByteArray *baBuff, char type)
{
   char ret = -1;
   if(baBuff)
   {
       switch(type)
       {
           case ALARM_STATISTICS:
             GetInterAlarmData(baBuff);
             ret = MSG_INTER_ALARM_INFO;
            break;
           case ALARM_REAL_TIME_ENTER:
           case ALARM_REAL_TIME_LEAVE:
             GetRealTimeAlarmData(baBuff, type);
             ret = MSG_REAL_TIME_ALARM_INFO;
            break;
           default:
            break;
       }
   }
   return ret;
}
void VideoDetector::slotSetRealTime(int lane, int deviceID)
{
    if(CAMERA_PROTOCOL10 == iProtocolVer)
    {
        alarmInfo10.byLane = lane;
        alarmInfo10.dwDeviceID = deviceID;
    }
    else if(CAMERA_PROTOCOL20 == iProtocolVer)
    {
        realTime20.byLane = lane;
        realTime20.wDeviceID = deviceID;
    }
    else if(CAMERA_PROTOCOL21 == iProtocolVer)
    {
        realTime21.byLane = lane;
        realTime21.wDeviceID = deviceID;
    }
   qDebug()<<"lane:"<<lane<<"deviceID:"<<deviceID<<endl;
}
void VideoDetector::onActionRealTimeTrigger()
{
    int lane=0;
    int deviceID=0;
    if(CAMERA_PROTOCOL10 == iProtocolVer)
    {
        lane = alarmInfo10.byLane;
        deviceID = alarmInfo10.dwDeviceID;
    }
    else if(CAMERA_PROTOCOL20 == iProtocolVer)
    {
        lane = realTime20.byLane;
        deviceID = realTime20.wDeviceID;
    }
    else if(CAMERA_PROTOCOL21 == iProtocolVer)
    {
        lane = realTime21.byLane;
        deviceID = realTime21.wDeviceID;
    }
    RealTimeDialog *dRealTime = new RealTimeDialog();
    dRealTime->setUiValue(lane, deviceID);
    connect(dRealTime, SIGNAL(sendData(int,int)), this, SLOT(slotSetRealTime(int,int)));
    dRealTime->show();
    //qDebug()<<"---> sub window func out..."<<endl;
}
void VideoDetector::slotSetStatisticsTime(char type, char *data)
{
   if(type == 1)
   {
       TPS_INTER_TIME_INFO20 *pInter = (TPS_INTER_TIME_INFO20*)data;
       interTime20.wDeviceID = pInter->wDeviceID;
       interTime20.wSamplePeriod = pInter->wSamplePeriod;
       qDebug()<<"deviceID:"<<pInter->wDeviceID<<"samplePeriod:"<<pInter->wSamplePeriod<<endl;
   }
   else if(type == 2)
   {
       TPS_PERIODS_TIME_INFO *pPeriod = (TPS_PERIODS_TIME_INFO*)data;
       periodsInfo21.uDeviceID = pPeriod->uDeviceID;
       periodsInfo21.uSamplePeriod = pPeriod->uSamplePeriod;
       periodsInfo21.uTotoalLaneNum = pPeriod->uTotoalLaneNum;
       qDebug()<<"deviceID:"<<pPeriod->uDeviceID<<"samplePeriod:"<<pPeriod->uSamplePeriod<<"TotalLane:"<<pPeriod->uTotoalLaneNum<<endl;
   }
}
void VideoDetector::onActionInterTrigger()
{
    InterDialog *inter = new InterDialog;
    inter->setUiValue(interTime20.wDeviceID, interTime20.wSamplePeriod);
    connect(inter, SIGNAL(sendData(char,char*)), this, SLOT(slotSetStatisticsTime(char,char*)));
    inter->show();
}
void VideoDetector::onActionAbout()
{
   QString info = tr("\t\t\tVideo Detector 2.0\n\t\t(Video Dectector simulator program)\n\n\
    Simulate video Camera send data.\n\
    Using camera protocol 2.1(defalut Protocol) send realtime data every second(include enter and leave message),\
send periods statistics data every 60 secnds(default value, set to 0 will cancel send statistics message).");
   //QMessageBox::about(this, tr("About VideoDetector2.0"), info);
    QPixmap *iAbout = new QPixmap(":/images/about.png");
    QMessageBox *about = new QMessageBox(this);
    about->setWindowTitle(tr("About Video Detector2.0"));
    about->setIconPixmap(*iAbout);
    about->setText(info);
    about->addButton(tr("OK"), QMessageBox::AcceptRole);
    about->show();
}
void VideoDetector::onProtocolChange(QAction *act)
{
//    qDebug()<<"1.0:"<<aCP10->isChecked()<<"2.0:"<<aCP20->isChecked()<<"2.1:"<<aCP21->isChecked()<<endl;
    qDebug()<<"--->"<<act->text()<<":"<<act->isChecked()<<endl;
    if(act->text().endsWith("1.0"))
        iProtocolVer = CAMERA_PROTOCOL10;
    else if(act->text().endsWith("2.0"))
        iProtocolVer = CAMERA_PROTOCOL20;
    else if(act->text().endsWith("2.1"))
        iProtocolVer = CAMERA_PROTOCOL21;
    qDebug()<<"ver: "<<iProtocolVer<<endl;
}
void VideoDetector::onActionPeriodsTrigger()
{
    PeriodDialog *pd = new PeriodDialog();
    pd->setUiValue(periodsInfo21);
    connect(pd, SIGNAL(sendData(char, char*)), this, SLOT(slotSetStatisticsTime(char,char*)));
    pd->show();
}
QString* VideoDetector::loadConfig(QString filename)
{
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug()<<"File not exist!"<<endl;
        return NULL;
    }
    QDataStream in(&file);
    QString *ip = new QString;
    in>>*ip;
    qDebug()<<"IP0: "<<ip<<endl;
    return ip;
}
void VideoDetector::OnAppOut()
{
   saveConfig(CONF_FILE_NAME);
}
void VideoDetector::saveConfig(QString filename)
{
    QString ip = leIp->text();
    if(!ip.isEmpty())
    {
       QFile file(filename);
       if(file.open(QIODevice::WriteOnly | QIODevice::Truncate))
       {
          qDebug()<<"write IP: "<<ip<<endl;
          QDataStream out(&file);
          out<<ip;
          file.close();
       }
       else
           ERR_MSG(this, tr("Open file error!"));
    }
    else
        qDebug()<<"IP is Empty!"<<endl;
}
void VideoDetector::setLanguage(QString file)
{
    if(tLanguage->load(file))
    {
        qApp->installTranslator(tLanguage);
   		this->repaint();
        //this->update();
    }
    else
        ERR_MSG(this, tr("load language file error!"));
    qDebug()<<"---->"<<QCoreApplication::translate("VideoDetector","Setting")<<endl;
}
void VideoDetector::onLanguageChange(QAction * act)
{
    QString sLan;
    if(cLanguage == C_LANGUAGE_CN)
        sLan = "英语";
    else
        sLan = "English";
    qDebug()<<"language action triggered..."<<act->text()<<endl;
   if(act && act->text().startsWith(sLan))
   {
       setLanguage(LANGUAGE_ENGLISH);
       qDebug()<<"set to english..."<<endl;
   }
   else
       setLanguage(LANGUAGE_CHINESE);
}
void VideoDetector::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch(e->type())
    {
        case QEvent::LanguageChange:
            reTranslate();
            break;
        default:
            break;
    }
}
void VideoDetector::reTranslate()
{
   char* sWin = "VideoDetector";
   setWindowTitle(tLanguage->translate(sWin, "VDTitle"));
   /**************menu************/
   //setting
   mSetting->setTitle(tLanguage->translate(sWin, "Setting"));
   aAlarm10->setText(tLanguage->translate(sWin, "Alarm10"));
   aRealTime20->setText(tLanguage->translate(sWin, "RealTime20"));
   aInterTime20->setText(tLanguage->translate(sWin, "InterTime20"));
   aRealTime21->setText(tLanguage->translate(sWin, "RealAlarm21"));
   aPeriodsTime21->setText(tLanguage->translate(sWin, "PeriodAlarm21"));
   //network
   mNetwork->setTitle(tLanguage->translate(sWin, "Network"));
   aCP10->setText(tLanguage->translate(sWin, "CP10"));
   aCP20->setText(tLanguage->translate(sWin, "CP20"));
   aCP21->setText(tLanguage->translate(sWin, "CP21"));
   //help
   mHelp->setTitle(tLanguage->translate(sWin, "Help"));
   mLanguage->setTitle(tLanguage->translate(sWin, "Language"));
   aChinese->setText(tLanguage->translate(sWin, "Chinese"));
   aEnglish->setText(tLanguage->translate(sWin, "English"));
   aAbout->setText(tLanguage->translate(sWin, "About"));
  /****Input****/
   lIp->setText(tLanguage->translate(sWin, "IPaddr"));
   lPort->setText(tLanguage->translate(sWin, "Port"));
   cbSC->setText(tLanguage->translate(sWin, "SignalCtrller"));
   pbStart->setText(tLanguage->translate(sWin, "Start"));
   pbStop->setText(tLanguage->translate(sWin, "Stop"));
   /****status***/
   lDetectorNum->setText(tLanguage->translate(sWin, "DetectorNum"));
   lVehicles->setText(tLanguage->translate(sWin, "Vehicles"));
   lStatisPks->setText(tLanguage->translate(sWin, "StatisPks"));
}
