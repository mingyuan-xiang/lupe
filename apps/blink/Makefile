EXEC = blink

OBJECTS = main.o

DEPS = libdsp:gcc libmspdriver:gcc libmsp:gcc

# clock config
export LIBMSP_CLK_DCOFREQ = 16
export LIBMSP_CLK_MCLK_DIV = 1

# print config
export LIBUARTIO0_INT_ENABLE = 1
export LIBMSPPRINTF_PRINT_PORT =  0

override CFLAGS += -Wno-pointer-to-int-cast
override CFLAGS += -Wno-missing-braces
include tools/maker/Makefile
