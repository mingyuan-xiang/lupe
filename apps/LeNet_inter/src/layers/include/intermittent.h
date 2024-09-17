#ifndef INTERMITTENT_H
#define INTERMITTENT_H

#define COMPUTE_CK 0
#define COMPUTE_PAD 1
#define COMPUTE_IN_CH 2
#define COMPUTE_OUT_CH 3
#define COMPUTE_IO_ROW 4
#define COMPUTE_IO_COL 5
#define MAIN_LOOP 6
#define COUNTER 7

#define GET_OFFSET(x) ((x) * 2)

#define DOUBLE_BUFFER_COMPLETE 0x7FFF // Complete double buffering. No need for recovery
#define DOUBLE_BUFFER_WRITE 0x8000 // Finish transferring to temporary buffer

#define PAD_START 0
#define PAD_MAIN 1
#define PAD_TRANSFER 2
#define PAD_MEMSET 3
#define PAD_END 4

#define INTERMITTENT_LeNet_START 0
#define INTERMITTENT_conv1_Conv_PREPARE 1
#define INTERMITTENT_conv1_Conv_MAIN 2
#define INTERMITTENT_conv1_Conv_BIAS 3
#define INTERMITTENT_conv1_Conv_EXIT 4
#define INTERMITTENT_conv1_Conv_DMA 5
#define INTERMITTENT_relu1_Clip_PREPARE 6
#define INTERMITTENT_relu1_Clip_MAIN 7
#define INTERMITTENT_relu1_Clip_BIAS 8
#define INTERMITTENT_relu1_Clip_EXIT 9
#define INTERMITTENT_relu1_Clip_DMA 10
#define INTERMITTENT_pool1_AveragePool_PREPARE 11
#define INTERMITTENT_pool1_AveragePool_MAIN 12
#define INTERMITTENT_pool1_AveragePool_BIAS 13
#define INTERMITTENT_pool1_AveragePool_EXIT 14
#define INTERMITTENT_pool1_AveragePool_DMA 15
#define INTERMITTENT_conv2_Conv_PREPARE 16
#define INTERMITTENT_conv2_Conv_MAIN 17
#define INTERMITTENT_conv2_Conv_BIAS 18
#define INTERMITTENT_conv2_Conv_EXIT 19
#define INTERMITTENT_conv2_Conv_DMA 20
#define INTERMITTENT_relu2_Clip_PREPARE 21
#define INTERMITTENT_relu2_Clip_MAIN 22
#define INTERMITTENT_relu2_Clip_BIAS 23
#define INTERMITTENT_relu2_Clip_EXIT 24
#define INTERMITTENT_relu2_Clip_DMA 25
#define INTERMITTENT_pool2_AveragePool_PREPARE 26
#define INTERMITTENT_pool2_AveragePool_MAIN 27
#define INTERMITTENT_pool2_AveragePool_BIAS 28
#define INTERMITTENT_pool2_AveragePool_EXIT 29
#define INTERMITTENT_pool2_AveragePool_DMA 30
#define INTERMITTENT_conv3_Conv_PREPARE 31
#define INTERMITTENT_conv3_Conv_MAIN 32
#define INTERMITTENT_conv3_Conv_BIAS 33
#define INTERMITTENT_conv3_Conv_EXIT 34
#define INTERMITTENT_conv3_Conv_DMA 35
#define INTERMITTENT_relu3_Clip_PREPARE 36
#define INTERMITTENT_relu3_Clip_MAIN 37
#define INTERMITTENT_relu3_Clip_BIAS 38
#define INTERMITTENT_relu3_Clip_EXIT 39
#define INTERMITTENT_relu3_Clip_DMA 40
#define INTERMITTENT_Flatten_PREPARE 41
#define INTERMITTENT_Flatten_MAIN 42
#define INTERMITTENT_Flatten_BIAS 43
#define INTERMITTENT_Flatten_EXIT 44
#define INTERMITTENT_Flatten_DMA 45
#define INTERMITTENT_fc1_Gemm_PREPARE 46
#define INTERMITTENT_fc1_Gemm_MAIN 47
#define INTERMITTENT_fc1_Gemm_BIAS 48
#define INTERMITTENT_fc1_Gemm_EXIT 49
#define INTERMITTENT_fc1_Gemm_DMA 50
#define INTERMITTENT_relu4_Clip_PREPARE 51
#define INTERMITTENT_relu4_Clip_MAIN 52
#define INTERMITTENT_relu4_Clip_BIAS 53
#define INTERMITTENT_relu4_Clip_EXIT 54
#define INTERMITTENT_relu4_Clip_DMA 55
#define INTERMITTENT_fc2_Gemm_PREPARE 56
#define INTERMITTENT_fc2_Gemm_MAIN 57
#define INTERMITTENT_fc2_Gemm_BIAS 58
#define INTERMITTENT_fc2_Gemm_EXIT 59
#define INTERMITTENT_fc2_Gemm_DMA 60
#define INTERMITTENT_LeNet_END 61

#endif