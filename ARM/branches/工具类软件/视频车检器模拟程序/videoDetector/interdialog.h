#ifndef INTERDIALOG_H
#define INTERDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QByteArray>

class InterDialog : public QDialog
{
    Q_OBJECT

public:
    InterDialog(QWidget *parent=0);
    void setUiValue(const int deviceID, const int samplePeriod);
signals:
    void sendData(char, char*);
public slots:
    void onButtonClicked();
private:
    QLineEdit *leDeviceID;
    QLineEdit *leSamplePeriod;

    QPushButton *pbOK;
};

#endif // INTERDIALOG_H
