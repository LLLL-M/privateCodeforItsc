#!/bin/sh
DEF="\033[m"
RED="\033[0;32;31m"
YEL="\033[1;33m"
error()
{
        printf "${RED}$1${DEF}\n" >&2 
}
right()
{
        printf "${YEL}$1${DEF}\n"
}

echo "You shoule run this script like this: "
right "'./buile.sh CROSS_COMPILE=arm-imx6ul-linux-gnueabihf- TARGET=zmq/czmq/jsoncpp/unqlite/libsocketcan/all'"
right "default: CROSS_COMPILE=arm-imx6ul-linux-gnueabihf- TARGET=all"

#"******setting environment ******"
if [ $# -eq 2 ]; then
	export $1
	export $2
elif [ $# -eq 1 ]; then
	export $1
fi

if [ -z $CROSS_COMPILE ]; then
	CROSS_COMPILE=arm-imx6ul-linux-gnueabihf-
fi
if [ -z $TARGET ]; then
	TARGET=all
fi

CC=${CROSS_COMPILE}gcc
AR=${CROSS_COMPILE}ar
STRIP=${CROSS_COMPILE}strip
CXX=${CROSS_COMPILE}g++
LD=${CROSS_COMPILE}ld 
RANLIB=${CROSS_COMPILE}ranlib
PREFIX=${PWD}
if [ -n "$CROSS_COMPILE" ]; then
	HOST=${CROSS_COMPILE%-*}
	HOST=${HOST##*/}
fi

compile_zmq()
{
	right "******execute cross compile zmq******"
	tar zxvf zeromq-4.2.1.tar.gz
	local DIR="zeromq-4.2.1"
	cd $DIR
	./configure --prefix=$PREFIX/ --host=$HOST
	make clean -f ./Makefile
	make -j 4 -f ./Makefile || (error "zmq make fail";exit)
	make install -f ./Makefile
	right "******zmq cross compile finished******"
	cd $PREFIX
	$STRIP -s $PREFIX/lib/libzmq.so.?.?.?
	rm -rf $DIR
}

compile_czmq()
{
	right "******execute cross compile czmq******"
	tar zxvf czmq-4.0.2.tar.gz
	local DIR="czmq-4.0.2"
	cd $DIR
	./configure --prefix=$PREFIX/ --host=$HOST --libdir="${PREFIX}/lib" --includedir="${PREFIX}/include" libzmq_CFLAGS="-I${PREFIX}/include" libzmq_LIBS="-L${PREFIX}/lib" --disable-zmakecert --disable-czmq_selftest 
	make clean -f ./Makefile
	make -j 4 -f ./Makefile || (error "czmq make fail";exit)
	make install -f ./Makefile
	right "******czmq cross compile finished******"
	cd $PREFIX
	$STRIP -s $PREFIX/lib/libczmq.so.?.?.?
	rm -rf $DIR
}

compile_jsoncpp()
{
	right "******execute cross compile json******"
	tar jxvf jsoncpp-master.tar.bz2
	local DIR="jsoncpp-master"
	cd $DIR/src/lib_json
	$CXX -fPIC -I../../include -c *.cpp
	$AR -crs libjson.a *.o
	$CXX -shared -fPIC *.cpp -I../../include -o libjson.so
	$STRIP -s *.so *.a
	cp -a libjson.a libjson.so $PREFIX/lib
	right "******json cross compile finished******"
	cd $PREFIX
	rm -rf $DIR
}

compile_unqlite()
{
	right "******execute cross compile unqlite******"
	tar jxvf unqlite.tar.bz2
	local DIR="unqlite"
	cd $DIR
	$CC -fPIC -I. -c *.c
	$CC -shared -fPIC *.c -I. -o libunqlite.so
	$STRIP -s *.so
	cp -a libunqlite.so $PREFIX/lib
	right "******unqlite cross compile finished******"
	cd $PREFIX
	rm -rf $DIR
}

compile_libsocketcan()
{
	right "******execute cross compile libsocketcan******"
	tar jxvf libsocketcan-0.0.10.tar.bz2
	local DIR="libsocketcan-0.0.10"
	cd $DIR
	./configure --host=$HOST CC=$CC --prefix=$PREFIX/
	(make && make install) || (error "libsocketcan make fail";exit)
	right "******libsocketcan cross compile finished******"
	cd $PREFIX
	rm -rf $DIR
}

#"******begin to compile libs******"
if [ $TARGET == 'zmq' ]; then
	compile_zmq
elif [ $TARGET == 'czmq' ]; then
	compile_zmq
	compile_czmq
elif [ $TARGET == 'jsoncpp' ]; then
	compile_jsoncpp
elif [ $TARGET == 'unqlite' ]; then
	compile_unqlite
elif [ $TARGET == 'libsocketcan' ]; then
	compile_libsocketcan
else
	compile_zmq
	compile_czmq
	compile_jsoncpp
	compile_unqlite
	compile_libsocketcan
fi
