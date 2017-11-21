CC=arm-none-linux-gnueabi-gcc
#CC=gcc
CFLAGS= -I../include -Wall -O3

sadp: clean
	$(CC) sadp.c -o $@ $(CFLAGS) -lpthread

clean:
	$(RM) sadp *.o
