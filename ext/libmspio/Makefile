LIB = libmspio

OBJECTS = uartio.o

DEPS += libmsp

override SRC_ROOT = ../../src

override CFLAGS += \
	-I$(SRC_ROOT)/include/libmspio \

include $(MAKER_ROOT)/Makefile.$(TOOLCHAIN)
