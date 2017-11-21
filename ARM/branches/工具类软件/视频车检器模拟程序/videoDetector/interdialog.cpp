#include "interdialog.h"
#include <QGridLayout>
#include <QLabel>
#include <QIntValidator>
#include <winsock2.h>
#include "data.h"
#include "common.h"


InterDialog::InterDialog(QWidget*parent) : QDialog(parent)
{
    setWindowTitle(tr("ITATitle"));

    QGridLayout *glMain = new QGridLayout(this);
    QLabel *lDeviceID = new QLabel(tr("DeviceID"));
    leDeviceID = new QLineEdit;
    leDeviceID->setValidator(new QIntValidator(1,89,this));
    QLabel *lDeviceIDNotice = new QLabel(tr("(1-89)"));
    QLabel *lSamplePeriod = new QLabel(tr("SamplePeriod"));
    leSamplePeriod = new QLineEdit;
    QLabel *lSamplePeriodNotice = new QLabel(tr("Uint"));

    pbOK = new QPushButton(tr("OK"));

    glMain->addWidget(lDeviceID, 0, 0);
    glMain->addWidget(leDeviceID, 0, 1);
    glMain->addWidget(lDeviceIDNotice, 0, 2);
    glMain->addWidget(lSamplePeriod, 1, 0);
    glMain->addWidget(leSamplePeriod, 1, 1);
    glMain->addWidget(lSamplePeriodNotice, 1, 2);
    glMain->addWidget(pbOK, 2, 1, 1, 1);
    connect(pbOK, SIGNAL(clicked(bool)), this, SLOT(onButtonClicked()));
}
void InterDialog::onButtonClicked()
{
    QString sDeviceID = leDeviceID->text();
    QString sSamplePeriod = leSamplePeriod->text();
    if(!sDeviceID.isEmpty() && !sSamplePeriod.isEmpty())
    {
        TPS_INTER_TIME_INFO20 data;
        data.wDeviceID = sDeviceID.toInt();
        if(data.wDeviceID ==0)
            return;
        data.wSamplePeriod = sSamplePeriod.toInt();
       emit sendData(1, (char*)&data);
       this->close();
    }
    else
        ERR_MSG(this, tr("Input can't be empty!"))
}
void InterDialog::setUiValue(const int deviceID, const int samplePeriod)
{
    leDeviceID->setText(QString::number(deviceID));
    leSamplePeriod->setText(QString::number(samplePeriod));
}
