#include "realtimedialog.h"
#include <QGridLayout>
#include <QIntValidator>
#include <QLabel>
#include <winsock2.h>
#include "data.h"
#include "common.h"


RealTimeDialog::RealTimeDialog(QWidget*parent) : QDialog(parent)
{
    setWindowTitle(tr("RTATitle"));

    QGridLayout *glMain = new QGridLayout(this);
    QLabel *lRealTimeByLane = new QLabel(tr("Lane"));
    QLabel *lByLaneNotice = new QLabel(tr("(1-49)"));
    QLabel *lRealTimeByDeviceID = new QLabel(tr("DeviceID"));
    QLabel *lByDeviceIDNotice = new QLabel(tr("(1-89)"));
    leByLane = new QLineEdit;
    leByLane->setValidator(new QIntValidator(1,49,this));
    leByDeviceID = new QLineEdit;
    leByDeviceID->setValidator(new QIntValidator(1,89,this));
    pbOK = new QPushButton(tr("OK"));
    glMain->addWidget(lRealTimeByLane, 0, 0);
    glMain->addWidget(leByLane, 0, 1);
    glMain->addWidget(lByLaneNotice, 0, 2);
    glMain->addWidget(lRealTimeByDeviceID, 1, 0);
    glMain->addWidget(leByDeviceID, 1, 1);
    glMain->addWidget(lByDeviceIDNotice, 1, 2);
    glMain->addWidget(pbOK, 2, 1, 1, 1);
    connect(pbOK, SIGNAL(clicked(bool)), this, SLOT(onButtonClicked()));
}
void RealTimeDialog::onButtonClicked()
{
    QString sByLane = leByLane->text();
    QString sByDeviceID = leByDeviceID->text();
    if(!sByLane.isEmpty() && !sByDeviceID.isEmpty())
    {
       int byLane = sByLane.toInt();
       int wDeviceID = sByDeviceID.toInt();
       if(byLane == 0 || wDeviceID == 0)
           return;
       emit sendData(byLane, wDeviceID);
       this->close();
    }
    else
        ERR_MSG(this, tr("Input can't be empty!"))
}
void RealTimeDialog::setUiValue(const int lane, const int deviceID)
{
   leByLane->setText(QString::number(lane));
   leByDeviceID->setText(QString::number(deviceID));
}
