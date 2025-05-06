LIB = libmspprintf

OBJECTS = mspprintf.o

DEPS += libmspio libmsp 

override SRC_ROOT = ../../src

override CFLAGS += \
	-I$(SRC_ROOT)/include/libmspprintf \

include $(MAKER_ROOT)/Makefile.$(TOOLCHAIN)
