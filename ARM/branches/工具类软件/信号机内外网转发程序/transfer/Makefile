#CC=gcc
CC=arm-none-linux-gnueabi-gcc

all: transfer 

transfer: transfer.o libparseini.a liblogsystem.a
	$(CC) $^ -o $@ -lpthread
clean:	
	rm -f transfer *.o
