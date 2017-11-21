
CC = arm-none-linux-gnueabi-gcc
CFLAGS = -I./
LDFLAGS = -L./ -lsqlite3
PG = sqlite3_dbupdate

all:clean pg
	

clean:
	rm -f $(PG)

pg:
	$(CC) main.c -o $(PG) $(CFLAGS) $(LDFLAGS)

