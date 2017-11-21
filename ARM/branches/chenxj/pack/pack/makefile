###########################################
#Makefile for programs
###########################################
.PHONY:clean

all:pack clean

INC=
LIB=

CC=gcc
CC_FLAG=-Wall -g

pack:pack.o
	$(CC) $(CC_FLAG) $(INC) $(LIB) -o $@ $^
pack.o:pack.c
	$(CC) $(CC_FLAG) $(INC) $(LIB) -o $@ -c $^



#.PHONY:clean
clean:
	@echo "Removing linked and compiled files......"
	rm *.o
