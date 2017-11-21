#ifndef REALTIMEDIALOG_H
#define REALTIMEDIALOG_H
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QByteArray>

class RealTimeDialog : public QDialog
{
    Q_OBJECT

public:
    RealTimeDialog(QWidget *parent=0);
    void setUiValue(const int lane, const int deviceID);
signals:
    void sendData(int, int);
public slots:
    void onButtonClicked();
private:
   QLineEdit *leByLane;
   QLineEdit *leByDeviceID;
   QPushButton *pbOK;
//   QByteArray *baRealTimeAlarm;
};

#endif // REALTIMEDIALOG_H
