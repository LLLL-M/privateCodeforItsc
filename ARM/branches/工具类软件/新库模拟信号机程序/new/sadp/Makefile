CC=arm-none-linux-gnueabi-gcc
CFLAGS= -I../include -Wall -Werror -O3

sadp: clean
	$(CC) sadp.c -o $@ $(CFLAGS)

clean:
	$(RM) sadp *.o
