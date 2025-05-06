LIB = libmspsyncioutils

OBJECTS = mspsyncioutils.o

DEPS += libmatAbstract libmspprintf libmspio

override SRC_ROOT = ../../src

override CFLAGS += \
	-I$(SRC_ROOT)/include/mspsyncioutils \

include $(MAKER_ROOT)/Makefile.$(TOOLCHAIN)
