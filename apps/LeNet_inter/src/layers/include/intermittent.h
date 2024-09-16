#ifndef INTERMITTENT_H
#define INTERMITTENT_H

#define COMPUTE_CK 0
#define COMPUTE_PAD 1
#define COMPUTE_IN_CH 2
#define COMPUTE_OUT_CN 3
#define COMPUTE_IO_ROW 4
#define COMPUTE_IO_COL 5

#define DOUBLE_BUFFER_COMPLETE 0x7FFF // Complete double buffering. No need for recovery
#define DOUBLE_BUFFER_WRITE 0x8000 // Finish transferring to temporary buffer

#define INTERMITTENT_LeNet_START 0
#define INTERMITTENT_conv1_Conv_MAIN 1
#define INTERMITTENT_conv1_Conv_DMA 2
#define INTERMITTENT_conv1_Conv_BIAS 3
#define INTERMITTENT_conv1_Conv_EXIT 4
#define INTERMITTENT_relu1_Clip_MAIN 5
#define INTERMITTENT_relu1_Clip_DMA 6
#define INTERMITTENT_relu1_Clip_BIAS 7
#define INTERMITTENT_relu1_Clip_EXIT 8
#define INTERMITTENT_pool1_AveragePool_MAIN 9
#define INTERMITTENT_pool1_AveragePool_DMA 10
#define INTERMITTENT_pool1_AveragePool_BIAS 11
#define INTERMITTENT_pool1_AveragePool_EXIT 12
#define INTERMITTENT_conv2_Conv_MAIN 13
#define INTERMITTENT_conv2_Conv_DMA 14
#define INTERMITTENT_conv2_Conv_BIAS 15
#define INTERMITTENT_conv2_Conv_EXIT 16
#define INTERMITTENT_relu2_Clip_MAIN 17
#define INTERMITTENT_relu2_Clip_DMA 18
#define INTERMITTENT_relu2_Clip_BIAS 19
#define INTERMITTENT_relu2_Clip_EXIT 20
#define INTERMITTENT_pool2_AveragePool_MAIN 21
#define INTERMITTENT_pool2_AveragePool_DMA 22
#define INTERMITTENT_pool2_AveragePool_BIAS 23
#define INTERMITTENT_pool2_AveragePool_EXIT 24
#define INTERMITTENT_conv3_Conv_MAIN 25
#define INTERMITTENT_conv3_Conv_DMA 26
#define INTERMITTENT_conv3_Conv_BIAS 27
#define INTERMITTENT_conv3_Conv_EXIT 28
#define INTERMITTENT_relu3_Clip_MAIN 29
#define INTERMITTENT_relu3_Clip_DMA 30
#define INTERMITTENT_relu3_Clip_BIAS 31
#define INTERMITTENT_relu3_Clip_EXIT 32
#define INTERMITTENT_Flatten_MAIN 33
#define INTERMITTENT_Flatten_DMA 34
#define INTERMITTENT_Flatten_BIAS 35
#define INTERMITTENT_Flatten_EXIT 36
#define INTERMITTENT_fc1_Gemm_MAIN 37
#define INTERMITTENT_fc1_Gemm_DMA 38
#define INTERMITTENT_fc1_Gemm_BIAS 39
#define INTERMITTENT_fc1_Gemm_EXIT 40
#define INTERMITTENT_relu4_Clip_MAIN 41
#define INTERMITTENT_relu4_Clip_DMA 42
#define INTERMITTENT_relu4_Clip_BIAS 43
#define INTERMITTENT_relu4_Clip_EXIT 44
#define INTERMITTENT_fc2_Gemm_MAIN 45
#define INTERMITTENT_fc2_Gemm_DMA 46
#define INTERMITTENT_fc2_Gemm_BIAS 47
#define INTERMITTENT_fc2_Gemm_EXIT 48
#define INTERMITTENT_LeNet_END 49

#endif