ar -x libsocketcan.a
arm-none-linux-gnueabi-gcc -c canmsg.c -o canmsg.o -lsocketcan -lpthread -L.
ar cr libcanits.a canmsg.o libsocketcan.o
#arm-none-linux-gnueabi-g++ -shared -fPIC -o libcanmsg.so *.cpp -lsocketcan -lpthread -L.


