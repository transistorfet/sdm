
SDM = ..

ARFLAGS = rc
#CFLAGS = -O3 -Wall -I$(SDM)/include
#LDFLAGS =
#CFLAGS = -pg -O3 -Wall -I$(SDM)/include
#LDFLAGS = -pg
CFLAGS = -g3 -DDEBUG -Wall -I$(SDM)/include
LDFLAGS =

CC	= gcc
CXX	= g++
LD	= g++
AS	= as
AR	= ar
RANLIB	= ranlib
DEPEND	= $(CC) -MM

