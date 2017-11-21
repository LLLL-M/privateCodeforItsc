#ifndef VIDEODETECTOR_H
#define VIDEODETECTOR_H

#include <QDialog>
//#include <QMainWindow>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QTimer>
#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>
#include <QPixmap>
#include <QTranslator>
//#include <QMenuBar>
//#include <QMenu>
#include "data.h"

class VideoDetector : public QDialog
{
    Q_OBJECT

public:
    VideoDetector(QWidget *parent = 0);
    ~VideoDetector();
     void dataFlowUpdate();
    int startWork();
    void setWidgetStatus(bool);
    void setPictureStatus(bool);
    char getData(QByteArray *baBuff, char type);
    void GetRealTimeAlarmData(QByteArray *baBuff, char type);
    void GetInterAlarmData(QByteArray *baBuff);
private:
//    QByteArray *getData();
//   void ndRealTimeData();
//    void sendStatisticsData();
    void setRealTimeInfo(char* , char);
    QString* loadConfig(QString filename);
    void saveConfig(QString filename);
    void setLanguage(QString);
    void reTranslate();
public slots:
    void onCBClicked(bool);
    void onPBStartClicked();
    void onPBStopClicked();
//    void onTimerTimeOut();
//    void slotConnected();

    void slotClientReturn(char);
    void connect2Server();

//    void onActionAlarm10Trigger();
    void onActionRealTimeTrigger();
    void onActionInterTrigger();
    void onActionPeriodsTrigger();
    void onActionAbout();
    void onProtocolChange(QAction*);

    void slotSetRealTime(int, int);
    void slotSetStatisticsTime(char, char*);
//    void onButtonTmpClick();
    void OnAppOut();
    void onLanguageChange(QAction *);
protected:
    void changeEvent(QEvent *);
private:
    QGridLayout *glMain;
    QLabel *lIp;
    QLabel *lPort;
    QLineEdit *leIp;
    QLineEdit *lePort;
    QCheckBox *cbSC;
    QLabel *lSettingStatus;
    QLabel *lLight;
    QLabel *lRealTime;
    QLabel *lPeriods;
    QPixmap *pRed;
    QPixmap *pGreen;
    QPushButton *pbStart;
    QPushButton *pbStop;
    QTimer *tTimer;
    QLabel *lMovie;
//    QMenuBar *mbMenu;
    QMenu *mSetting;
    QMenu *mNetwork;
    QMenu *mHelp;
    QAction *aChinese;
    QAction *aEnglish;
    QAction *aAbout;
    QLabel *lDetectorNum;
    QLabel *lVehicles;
    QLabel *lStatisPks;
    QMenu *mLanguage;
    QAction *aAlarm10;
    QAction *aRealTime20;
    QAction *aInterTime20;
    QAction *aRealTime21;
    QAction *aPeriodsTime21;
    QAction *aCP10;
    QAction *aCP20;
    QAction *aCP21;

    QTranslator *tLanguage;
    char cLanguage;
    QHostAddress *servIp;
    int servPort;
    int iRealtimeCounter;
    int iPeriodsCounter;
    int iTimeCounter;
    int iProtocolVer;
//    QTcpServer *tcpServ;
//    QTcpSocket *tcpSocket;
    TPS_ALARM_INFO10 alarmInfo10;
    TPS_REAL_TIME_INFO20 realTime20;
    TPS_INTER_TIME_INFO20 interTime20;
    TPS_REAL_TIME_INFO20 realTime21;
    TPS_PERIODS_TIME_INFO periodsInfo21;
};

#endif // VIDEODETECTOR_H
