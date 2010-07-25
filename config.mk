
SDRL = ~/sdrl
SDM = ..

ARFLAGS = rc
#CFLAGS = -O3 -Wall -I$(SDRL)/include -L$(SDRL)/lib
#LDFLAGS = -L$(SDRL)/lib
#CFLAGS = -pg -O3 -Wall -I$(SDRL)/include
#LDFLAGS = -pg -L$(SDRL)/lib
CFLAGS = -g3 -DDEBUG -Wall -I$(SDM)/include -I$(SDRL)/include
LDFLAGS = -L$(SDRL)/lib

CC	= gcc
CXX	= g++
LD	= gcc
AS	= as
AR	= ar
RANLIB	= ranlib
DEPEND	= $(CC) -MM

