CC=arm-none-linux-gnueabi-gcc
CFLAGS=-Ireadline 
LDFLAGS=-lncurses -lpthread
tools_test: tools_test.o canmsg.o readline/libreadline.a libsocketcan.a

clean:
	$(RM) *.o tools_test
