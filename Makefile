#
# Automatically generated by tools/build.pl
#

.SUFFIXES: .o .c .h .cpp .s .asm .rc
.PHONY: builds clean

ROOT = .

include $(ROOT)/config.mk

all: builds $(TARGET) $(LIB_TARGET)

builds:
	$(MAKE) -C src all

clean:
	$(MAKE) -C src clean

