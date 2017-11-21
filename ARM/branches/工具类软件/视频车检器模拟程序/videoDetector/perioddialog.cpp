#include "perioddialog.h"
#include <QLabel>
//#include <QLineEdit>
#include <QGridLayout>
#include <QIntValidator>
#include "common.h"

PeriodDialog::PeriodDialog(QWidget *parent):QDialog(parent)
{
    setWindowTitle(tr("PTATitle"));

    QLabel *lDeviceID = new QLabel(tr("DeviceID"));
    QLabel *lDeviceIDNotice = new QLabel(tr("(1-89)"));
    QLabel *lSamplePeriod = new QLabel(tr("SamplePeriod"));
    QLabel *lSamplePeriodNotice = new QLabel(tr("Uint"));
    QLabel *lTotalLaneNum = new QLabel(tr("TotalLaneNum"));
    QLabel *lTotalLaneNumNotice = new QLabel(tr("(1-8)"));
    leDeviceID = new QLineEdit;
    leDeviceID->setValidator(new QIntValidator(1,89,this));
    leSamplePeriod = new QLineEdit;
    leTotalLaneNum = new QLineEdit;
    leTotalLaneNum->setValidator(new QIntValidator(1,8,this));
    pbOk = new QPushButton(tr("OK"));
    QGridLayout *glMain = new QGridLayout(this);
    glMain->addWidget(lDeviceID, 0, 0);
    glMain->addWidget(leDeviceID, 0, 1);
    glMain->addWidget(lDeviceIDNotice, 0, 2);
    glMain->addWidget(lSamplePeriod, 1, 0);
    glMain->addWidget(leSamplePeriod, 1, 1);
    glMain->addWidget(lSamplePeriodNotice, 1, 2);
    glMain->addWidget(lTotalLaneNum, 2, 0);
    glMain->addWidget(leTotalLaneNum, 2, 1);
    glMain->addWidget(lTotalLaneNumNotice, 2, 2);
    glMain->addWidget(pbOk, 3, 2);

    connect(pbOk, SIGNAL(clicked(bool)), this, SLOT(onButtonClicked()));
}
void PeriodDialog::setUiValue(TPS_PERIODS_TIME_INFO &period)
{
    leDeviceID->setText(QString::number(period.uDeviceID));
    leSamplePeriod->setText(QString::number(period.uSamplePeriod));
    leTotalLaneNum->setText(QString::number(period.uTotoalLaneNum));
}
void PeriodDialog::onButtonClicked()
{
   TPS_PERIODS_TIME_INFO tpti;
   QString deviceID = leDeviceID->text();
   QString samplePeriod = leSamplePeriod->text();
   QString totalLaneNum = leTotalLaneNum->text();
   if(!deviceID.isEmpty() && !samplePeriod.isEmpty() && !totalLaneNum.isEmpty())
   {
       tpti.uDeviceID = deviceID.toInt();
       tpti.uSamplePeriod = samplePeriod.toInt();
       tpti.uTotoalLaneNum = totalLaneNum.toInt();
       if(tpti.uDeviceID ==0 || tpti.uTotoalLaneNum ==0)
           return;
       emit sendData(2, (char*)&tpti);
       this->close();
   }
   else
       ERR_MSG(this,tr("Input can't be Empty!"))
}
