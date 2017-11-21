RTSC_sadp: TS_sadp.o
	#gcc -g -o RTSC_sadp TS_sadp.o -pthread -lsadp -L.
	arm-none-linux-gnueabi-gcc -g -o RTSC_sadp TS_sadp.o -pthread -lsadp -L.
	
TS_sadp.o: TS_sadp.c
	#gcc -g -c TS_sadp.c
	arm-none-linux-gnueabi-gcc -g -c TS_sadp.c
	
clean:
	rm *.o RTSC_sadp

