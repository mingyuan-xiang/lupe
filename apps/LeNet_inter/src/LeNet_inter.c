#include <libdsp/DSPLib.h>
#include <libmatAbstract/mat.h>
#include <libmsp/mspbase.h>
#include <libmspdriver/driverlib.h>
#include <msp430.h>

#include <include/LeNet_inter.h>
#include <buffer/include/buffer.h>
#include <buffer/include/input.h>


#include <layers/include/conv1_Conv.h>
#include <layers/include/relu1_Clip.h>
#include <layers/include/pool1_AveragePool.h>
#include <layers/include/conv2_Conv.h>
#include <layers/include/relu2_Clip.h>
#include <layers/include/pool2_AveragePool.h>
#include <layers/include/conv3_Conv.h>
#include <layers/include/relu3_Clip.h>
#include <layers/include/Flatten.h>
#include <layers/include/fc1_Gemm.h>
#include <layers/include/relu4_Clip.h>
#include <layers/include/fc2_Gemm.h>

#include <layers/include/utils.h>
#include <layers/include/intermittent.h>


#include <params/include/conv1_Conv.h>
#include <params/include/conv2_Conv.h>
#include <params/include/conv3_Conv.h>
#include <params/include/fc1_Gemm.h>
#include <params/include/fc2_Gemm.h>

#include <libmsppoweroff/poweroff.h>

uint16_t LeNet(mat_t* model_in) {
  switch (intermittent_status[COMPUTE_CK]) {
  case INTERMITTENT_LeNet_START:
    DMA_makeTransfer((uintptr_t)(model_in->data), (uintptr_t)(conv1_Conv_in_meta.data), GET_MAT_SIZE(model_in));
    intermittent_status[COMPUTE_CK] = INTERMITTENT_conv1_Conv_PREPARE;
  case INTERMITTENT_conv1_Conv_PREPARE:
  case INTERMITTENT_conv1_Conv_MAIN:
  case INTERMITTENT_conv1_Conv_BIAS:
  case INTERMITTENT_conv1_Conv_EXIT:
  /* conv1_Conv */
    conv1_Conv(&conv1_Conv_in_meta, &conv1_Conv_out_meta, &conv1_weight_meta, &conv1_bias_meta);
    intermittent_status[COMPUTE_CK] = INTERMITTENT_relu1_Clip_PREPARE;
  case INTERMITTENT_relu1_Clip_PREPARE:
  case INTERMITTENT_relu1_Clip_MAIN:
  case INTERMITTENT_relu1_Clip_BIAS:
  case INTERMITTENT_relu1_Clip_EXIT:
  /* relu1_Clip */
    relu1_Clip(&relu1_Clip_in_meta, &relu1_Clip_out_meta);
    intermittent_status[COMPUTE_CK] = INTERMITTENT_pool1_AveragePool_PREPARE;
  case INTERMITTENT_pool1_AveragePool_PREPARE:
  case INTERMITTENT_pool1_AveragePool_MAIN:
  case INTERMITTENT_pool1_AveragePool_BIAS:
  case INTERMITTENT_pool1_AveragePool_EXIT:
  /* pool1_AveragePool */
    pool1_AveragePool(&pool1_AveragePool_in_meta, &pool1_AveragePool_out_meta);
    intermittent_status[COMPUTE_CK] = INTERMITTENT_conv2_Conv_PREPARE;
  case INTERMITTENT_conv2_Conv_PREPARE:
  case INTERMITTENT_conv2_Conv_MAIN:
  case INTERMITTENT_conv2_Conv_BIAS:
  case INTERMITTENT_conv2_Conv_EXIT:
  /* conv2_Conv */
    conv2_Conv(&conv2_Conv_in_meta, &conv2_Conv_out_meta, &conv2_weight_meta, &conv2_bias_meta);
    intermittent_status[COMPUTE_CK] = INTERMITTENT_relu2_Clip_PREPARE;
  case INTERMITTENT_relu2_Clip_PREPARE:
  case INTERMITTENT_relu2_Clip_MAIN:
  case INTERMITTENT_relu2_Clip_BIAS:
  case INTERMITTENT_relu2_Clip_EXIT:
  /* relu2_Clip */
    relu2_Clip(&relu2_Clip_in_meta, &relu2_Clip_out_meta);
    intermittent_status[COMPUTE_CK] = INTERMITTENT_pool2_AveragePool_PREPARE;
  case INTERMITTENT_pool2_AveragePool_PREPARE:
  case INTERMITTENT_pool2_AveragePool_MAIN:
  case INTERMITTENT_pool2_AveragePool_BIAS:
  case INTERMITTENT_pool2_AveragePool_EXIT:
  /* pool2_AveragePool */
    pool2_AveragePool(&pool2_AveragePool_in_meta, &pool2_AveragePool_out_meta);
    intermittent_status[COMPUTE_CK] = INTERMITTENT_conv3_Conv_PREPARE;
  case INTERMITTENT_conv3_Conv_PREPARE:
  case INTERMITTENT_conv3_Conv_MAIN:
  case INTERMITTENT_conv3_Conv_BIAS:
  case INTERMITTENT_conv3_Conv_EXIT:
  /* conv3_Conv */
    conv3_Conv(&conv3_Conv_in_meta, &conv3_Conv_out_meta, &conv3_weight_meta, &conv3_bias_meta);
    intermittent_status[COMPUTE_CK] = INTERMITTENT_relu3_Clip_PREPARE;
  case INTERMITTENT_relu3_Clip_PREPARE:
  case INTERMITTENT_relu3_Clip_MAIN:
  case INTERMITTENT_relu3_Clip_BIAS:
  case INTERMITTENT_relu3_Clip_EXIT:
  /* relu3_Clip */
    relu3_Clip(&relu3_Clip_in_meta, &relu3_Clip_out_meta);
    intermittent_status[COMPUTE_CK] = INTERMITTENT_Flatten_PREPARE;
  case INTERMITTENT_Flatten_PREPARE:
  case INTERMITTENT_Flatten_MAIN:
  case INTERMITTENT_Flatten_BIAS:
  case INTERMITTENT_Flatten_EXIT:
  /* Flatten */
    Flatten(&Flatten_in_meta, &Flatten_out_meta);
    intermittent_status[COMPUTE_CK] = INTERMITTENT_fc1_Gemm_PREPARE;
  case INTERMITTENT_fc1_Gemm_PREPARE:
  case INTERMITTENT_fc1_Gemm_MAIN:
  case INTERMITTENT_fc1_Gemm_BIAS:
  case INTERMITTENT_fc1_Gemm_EXIT:
  /* fc1_Gemm */
    fc1_Gemm(&fc1_Gemm_in_meta, &fc1_Gemm_out_meta, &fc1_weight_meta, &fc1_bias_meta);
    intermittent_status[COMPUTE_CK] = INTERMITTENT_relu4_Clip_PREPARE;
  case INTERMITTENT_relu4_Clip_PREPARE:
  case INTERMITTENT_relu4_Clip_MAIN:
  case INTERMITTENT_relu4_Clip_BIAS:
  case INTERMITTENT_relu4_Clip_EXIT:
  /* relu4_Clip */
    relu4_Clip(&relu4_Clip_in_meta, &relu4_Clip_out_meta);
    intermittent_status[COMPUTE_CK] = INTERMITTENT_fc2_Gemm_PREPARE;
  case INTERMITTENT_fc2_Gemm_PREPARE:
  case INTERMITTENT_fc2_Gemm_MAIN:
  case INTERMITTENT_fc2_Gemm_BIAS:
  case INTERMITTENT_fc2_Gemm_EXIT:
  /* fc2_Gemm */
    fc2_Gemm(&fc2_Gemm_in_meta, &fc2_Gemm_out_meta, &fc2_weight_meta, &fc2_bias_meta);
    intermittent_status[COMPUTE_CK] = INTERMITTENT_LeNet_END;
  default:
  }

  /* Get the max score */
  uint16_t max = 0;
  for (uint16_t i = 0; i < fc2_Gemm_out_meta.dims[1]; ++i) {
    max = fc2_Gemm_out_meta.data[max] > fc2_Gemm_out_meta.data[i] ? max : i;
  }

  return max;
}