QT += core
QT -= gui

CONFIG += c++11

TARGET = TrafficSignalControl
#CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app
RC_FILE += icon.rc

INCLUDEPATH += include/
INCLUDEPATH += hik/
INCLUDEPATH += libits/

LIBS += D:\work\qt_prj\TrafficSignalControl\sqlite3.dll
LIBS += libws2_32 libwinmm

QMAKE_CXXFLAGS += -Wno-unused-parameter -Wno-sign-compare -Wno-write-strings

SOURCES += main.cpp \
    hik/communication.cpp \
    hik/countdown.cpp \
    hik/download.cpp \
    hik/hiktsc.cpp \
    hik/sqlite_conf.cpp \
    hik/upload.cpp \
    libits/calculate.cpp \
    libits/faultlog.cpp \
    libits/its.cpp \
    libits/phasecontrol.cpp \
    libits/phasedriver.cpp \
    libits/redsigcheck.cpp \
    libits/strategycontrol.cpp \
    hik/extconfig.cpp \
    libits/channelhandle.cpp \
    hik/chanlockcontrol.cpp

HEADERS += \
    include/common.h \
    include/configureManagement.h \
    include/file.h \
    include/hik.h \
    include/HikConfig.h \
    include/its.h \
    include/lock.h \
    include/manage.h \
    include/msg.h \
    include/platform.h \
    include/protocol.h \
    include/sem.h \
    include/sqlite3.h \
    include/thread.h \
    include/tsc.h \
    hik/communication.h \
    hik/config.h \
    hik/countdown.h \
    hik/custom.h \
    hik/desc.h \
    hik/hikconf.h \
    hik/hiktsc.h \
    hik/ignorephase.h \
    hik/sqlite_conf.h \
    libits/calculate.h \
    libits/clock.h \
    libits/faultlog.h \
    libits/gettime.hpp \
    libits/inductive.hpp \
    libits/ipc.h \
    libits/lfq.h \
    libits/phasecontrol.h \
    libits/phasedriver.h \
    libits/redsigcheck.h \
    libits/special.hpp \
    libits/step.hpp \
    libits/strategycontrol.h \
    libits/transition.hpp \
    libits/followphase.hpp \
    hik/chanlockcontrol.h \
    libits/channelhandle.h



