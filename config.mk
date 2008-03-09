
ARFLAGS = rc
#CFLAGS = -O3 -Wall -I$(ROOT)/include -L$(ROOT)/lib
#LDFLAGS = -L$(ROOT)/lib
#CFLAGS = -pg -O3 -Wall -I$(ROOT)/include
#LDFLAGS = -pg -L$(ROOT)/lib
CFLAGS = -g3 -DDEBUG -Wall -I$(ROOT)/include
LDFLAGS = -L$(ROOT)/lib

CC	= gcc
CXX	= g++
LD	= gcc
AS	= as
AR	= ar
RANLIB	= ranlib
DEPEND	= $(CC) -MM

