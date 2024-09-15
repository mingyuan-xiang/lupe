EXEC = LeNet_inter

OBJECTS = main.o LeNet_inter.o buffer/input.o buffer/buffer.o layers/utils.o layers/conv1_Conv.o layers/relu1_Clip.o layers/pool1_AveragePool.o layers/conv2_Conv.o layers/relu2_Clip.o layers/pool2_AveragePool.o layers/conv3_Conv.o layers/relu3_Clip.o layers/Flatten.o layers/fc1_Gemm.o layers/relu4_Clip.o layers/fc2_Gemm.o params/conv1_Conv.o params/conv2_Conv.o params/conv3_Conv.o params/fc1_Gemm.o params/fc2_Gemm.o

DEPS = libdsp:gcc libmsppoweroff:gcc libmsptimer:gcc libmspsyncioutils:gcc libmatAbstract:gcc libfixedAbstract:gcc libmspprintf:gcc libmspio:gcc libmspdriver:gcc libmsp:gcc

# clock config
export LIBMSP_CLK_DCOFREQ = 16
export LIBMSP_CLK_MCLK_DIV = 1

# print config
export LIBUARTIO0_INT_ENABLE = 1
export LIBMSPPRINTF_PRINT_PORT = 0

override CFLAGS += -Wno-pointer-to-int-cast
override CFLAGS += -Wno-missing-braces
include tools/maker/Makefile