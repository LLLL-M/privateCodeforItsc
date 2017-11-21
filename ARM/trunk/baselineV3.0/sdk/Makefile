ifeq ($(shell uname), Linux)
#linux
TARGET = libtsm.so
CXXFLAGS = -fPIC
LDFLAGS = -lpthread
CXX = g++-7.1.0
LDFLAGS += -L../libforpclinux -lzmq -lczmq
else
#windows
TARGET = libtsm.dll
LDFLAGS += -L../libforwindows -lzmq -lczmq
#LDFLAGS = -Wl,--output-def,tsm.def,--out-implib,tsm.dll.a
#VS = lib /MACHINE:X86 /def:tsm.def
RM = del
OTHER = *.def *.lib *.exp
CXX = g++
endif

CXXFLAGS += --std=c++11 -I../include -I../zmqinclude -Wall

LDFLAGS += -latomic

SRC := $(wildcard *.cpp)
OBJS := $(SRC:.cpp=.o)

.PHONY: clean

all: clean $(TARGET)
	$(RM) *.o
	$(VS)

$(TARGET): $(OBJS)
	$(CXX) -s -shared $(LDFLAGS) $^ -o $@
#	$(CXX) -shared $(LDFLAGS) $^ -o $@ -Wl,-soname,$(LIBNAME).so.$(VER)

clean:
	$(RM) *.o $(TARGET) $(OTHER)
