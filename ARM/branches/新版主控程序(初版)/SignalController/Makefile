CROSS_COMPILE ?= arm-none-linux-gnueabi-
CC = $(CROSS_COMPILE)gcc
OPTIMIZER = -O3
CFLAGS = $(OPTIMIZER) -Wall -Werror

PROGOBJS = Main.o MyLinkList.o ThreadController.o Util.o LogSystem.o SqQueue.o MyDateTime.o TaskSchedule.o \
	Action.o Channel.o GreenSignalRatio.o Phase.o Schemes.o TimeInterval.o Unit.o CPLD.o LampController.o \
	FollowPhase.o PhaseTurn.o

PGM = SignalController

all:	$(PGM)

$(PGM):	$(PROGOBJS) Makefile
	$(CC) -o $(PGM) $(PROGOBJS) -lpthread ./libcanits.a 

distclean: clean

clean:
	rm -f *.o $(PGM)


