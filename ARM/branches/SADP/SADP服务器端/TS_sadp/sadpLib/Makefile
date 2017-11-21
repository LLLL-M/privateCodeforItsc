SOURCES = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJS = $(patsubst %.c, %.o, $(SOURCES))

TOOL_PREFIX =
#COMPILE_TOOLS = $(TOOL_PREFIX)gcc
COMPILE_TOOLS = $(TOOL_PREFIX)arm-none-linux-gnueabi-gcc
AR = $(TOOL_PREFIX)ar
RANLIB = $(TOOL_PREFIX)ranlib

CFLAGS = -D_REENTRANT -O2 -Wall

LIB = libsadp.a

all : $(LIB) 

$(LIB) : $(OBJS)
	$(AR) r $@ $(OBJS)
	$(RANLIB) $@

%.o:%.c
	$(COMPILE_TOOLS) $(CFLAGS) -c $^ -o $@

clean :
	rm -f *.o  $(LIB)

