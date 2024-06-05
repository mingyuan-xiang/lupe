EXEC = LeNet

OBJECTS = main.o LeNet.o buffer/input.o buffer/buffer.o buffer/lupe_buffer.o layers/utils.o layers/conv1.o layers/Relu.o layers/Clip.o layers/pool1_AveragePool.o layers/Clip_1.o layers/conv2.o layers/Relu_1.o layers/Clip_2.o layers/pool2_AveragePool.o layers/Clip_3.o layers/conv3.o layers/Relu_2.o layers/Clip_4.o layers/Flatten.o layers/fc1.o layers/Relu_3.o layers/Clip_5.o layers/fc2.o layers/Clip_6.o params/conv1.o params/conv2.o params/conv3.o params/fc1.o params/fc2.o

DEPS = libdsp:gcc libmsptimer:gcc libmspsyncioutils:gcc libmatAbstract:gcc libfixedAbstract:gcc libmspprintf:gcc libmspio:gcc libmspdriver:gcc libmsp:gcc

# clock config
export LIBMSP_CLK_DCOFREQ = 16
export LIBMSP_CLK_MCLK_DIV = 1

# print config
export LIBUARTIO0_INT_ENABLE = 1
export LIBMSPPRINTF_PRINT_PORT = 0

override CFLAGS += -Wno-pointer-to-int-cast
override CFLAGS += -Wno-missing-braces
include tools/maker/Makefile