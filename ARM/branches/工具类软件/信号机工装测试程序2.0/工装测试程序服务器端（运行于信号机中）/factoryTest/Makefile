CC=arm-none-linux-gnueabi-gcc
LDFLAGS=-pthread
ftest: main.o ftest.o canmsg.o common.o cpld.o gps.o libcanits.a
clean:
	$(RM) *.o ftest