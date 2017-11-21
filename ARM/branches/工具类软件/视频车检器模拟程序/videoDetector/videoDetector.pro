#-------------------------------------------------
#
# Project created by QtCreator 2016-08-22T15:37:05
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = videoDetector
TEMPLATE = app


SOURCES += main.cpp\
        videodetector.cpp \
    realtimedialog.cpp \
    interdialog.cpp \
    tcpclient.cpp \
    perioddialog.cpp

HEADERS  += videodetector.h \
         data.h	\
    realtimedialog.h \
    common.h \
    interdialog.h \
    tcpclient.h \
    perioddialog.h

RESOURCES += \
    image.qrc

LIBS += -lws2_32

RC_FILE += icon.rc

TRANSLATIONS += translate_CN.ts translate_EN.ts

#CODECFORTR = utf-8
#DEFAULTCODEC = utf-8
#CODEC = utf-8

