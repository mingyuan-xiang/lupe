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


#include <params/include/conv1_Conv.h>
#include <params/include/conv2_Conv.h>
#include <params/include/conv3_Conv.h>
#include <params/include/fc1_Gemm.h>
#include <params/include/fc2_Gemm.h>

uint16_t LeNet_inter(mat_t* model_in) {
  DMA_makeTransfer((uintptr_t)(model_in->data), (uintptr_t)(conv1_Conv_in_meta.data), GET_MAT_SIZE(model_in));
  /* conv1_Conv */
  conv1_Conv(&conv1_Conv_in_meta, &conv1_Conv_out_meta, &conv1_weight_meta, &conv1_bias_meta);
  /* relu1_Clip */
  relu1_Clip(&relu1_Clip_in_meta, &relu1_Clip_out_meta);
  /* pool1_AveragePool */
  pool1_AveragePool(&pool1_AveragePool_in_meta, &pool1_AveragePool_out_meta);
  /* conv2_Conv */
  conv2_Conv(&conv2_Conv_in_meta, &conv2_Conv_out_meta, &conv2_weight_meta, &conv2_bias_meta);
  /* relu2_Clip */
  relu2_Clip(&relu2_Clip_in_meta, &relu2_Clip_out_meta);
  /* pool2_AveragePool */
  pool2_AveragePool(&pool2_AveragePool_in_meta, &pool2_AveragePool_out_meta);
  /* conv3_Conv */
  conv3_Conv(&conv3_Conv_in_meta, &conv3_Conv_out_meta, &conv3_weight_meta, &conv3_bias_meta);
  /* relu3_Clip */
  relu3_Clip(&relu3_Clip_in_meta, &relu3_Clip_out_meta);
  /* Flatten */
  Flatten(&Flatten_in_meta, &Flatten_out_meta);
  /* fc1_Gemm */
  fc1_Gemm(&fc1_Gemm_in_meta, &fc1_Gemm_out_meta, &fc1_weight_meta, &fc1_bias_meta);
  /* relu4_Clip */
  relu4_Clip(&relu4_Clip_in_meta, &relu4_Clip_out_meta);
  /* fc2_Gemm */
  fc2_Gemm(&fc2_Gemm_in_meta, &fc2_Gemm_out_meta, &fc2_weight_meta, &fc2_bias_meta);
  /* Get the max score */
  uint16_t max = 0;
  for (uint16_t i = 0; i < fc2_Gemm_out_meta.dims[1]; ++i) {
    max = fc2_Gemm_out_meta.data[max] > fc2_Gemm_out_meta.data[i] ? max : i;
  }

  return max;
}