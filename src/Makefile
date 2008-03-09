
.SUFFIXES: .o .c .h .cpp .s .asm .rc
.PHONY: builds clean

ROOT = ..
TARGET = ../bin/mud

OBJS =	commands.o \
	data.o \
	hash.o \
	main.o	\
	misc.o\
	string.o \
	interface/interface.o \
	interface/tcp.o \
	interface/telnet/login.o \
	interface/telnet/register.o \
	interface/telnet/telnet.o \
	objs/login.o \
	objs/mobile.o \
	objs/object.o \
	objs/user.o \

include $(ROOT)/config.mk

LIBS = -lcrypt `xml2-config --libs`
CFLAGS += `xml2-config --cflags`

all: builds $(TARGET) $(LIB_TARGET)

builds:

$(TARGET): builds $(OBJS)
	$(LD) $(LDFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

$(LIB_TARGET): builds $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)
	$(RANLIB) $(LIB_TARGET)

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

.cpp.o:
	$(CXX) $(CFLAGS) $(CXXFLAGS) -c -o $@ $<

clean:
	$(RM) $(OBJS)

