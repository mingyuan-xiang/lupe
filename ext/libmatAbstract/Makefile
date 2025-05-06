LIB = libmatAbstract

OBJECTS = mat.o

DEPS = libfixedAbstract libmspprintf

override SRC_ROOT = ../../src

override CFLAGS += -I $(SRC_ROOT)/include/$(LIB)

include $(MAKER_ROOT)/Makefile.$(TOOLCHAIN)
