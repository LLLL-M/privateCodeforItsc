CROSS_COMPILE ?= arm-none-linux-gnueabi-
CFLAG=Wall -s -unaligned_access -O3

CC = $(CROSS_COMPILE)gcc
STRIP=$(CROSS_COMPILE)strip

all: ledctrl

ledctrl: \
	./ledctrl.o ./CPLD.o

	$(CC) $^ -lpthread ./libcanits.a -$(CFLAG) -o $@ -Wall
##	$(STRIP) $@

ledctrl.o: ./ledctrl.c
	$(CC) -c $^ -$(CFLAG)

CPLD.o: ./CPLD.c
	$(CC) -c $^ -$(CFLAG)

clean:
	rm  *.o
	rm  ledctrl
