#include "videodetector.h"
#include <QApplication>
#include "common.h"
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    VideoDetector w;
    w.show();

    return a.exec();
}
