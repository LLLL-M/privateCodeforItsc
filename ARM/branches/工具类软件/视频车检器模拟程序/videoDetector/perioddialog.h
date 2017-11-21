#ifndef PERIODDIALOG_H
#define PERIODDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include "data.h"

class PeriodDialog : public QDialog
{
    Q_OBJECT
public:
    PeriodDialog(QWidget *parent=0);
    void setUiValue(TPS_PERIODS_TIME_INFO &period);
signals:
    void sendData(char, char*);
public slots:
    void onButtonClicked();
private:
    QLineEdit *leDeviceID;
    QLineEdit *leSamplePeriod;
    QLineEdit *leTotalLaneNum;
    QPushButton *pbOk;
};

#endif // PERIODDIALOG_H
