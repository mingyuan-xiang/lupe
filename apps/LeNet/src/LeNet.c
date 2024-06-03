#include <libdsp/DSPLib.h>
#include <libmatAbstract/mat.h>
#include <libmsp/mspbase.h>
#include <libmspdriver/driverlib.h>
#include <msp430.h>
#include <libmspsyncioutils/mspsyncioutils.h>
#include <libmsptimer/timekeeper.h>

#include <include/LeNet.h>
#include <buffer/include/buffer.h>
#include <buffer/include/input.h>

#include <layers/include/conv1.h>
#include <layers/include/Relu.h>
#include <layers/include/Clip.h>
#include <layers/include/pool1_AveragePool.h>
#include <layers/include/Clip_1.h>
#include <layers/include/conv2.h>
#include <layers/include/Relu_1.h>
#include <layers/include/Clip_2.h>
#include <layers/include/pool2_AveragePool.h>
#include <layers/include/Clip_3.h>
#include <layers/include/conv3.h>
#include <layers/include/Relu_2.h>
#include <layers/include/Clip_4.h>
#include <layers/include/Flatten.h>
#include <layers/include/fc1.h>
#include <layers/include/Relu_3.h>
#include <layers/include/Clip_5.h>
#include <layers/include/fc2.h>
#include <layers/include/Clip_6.h>

#include <params/include/conv1.h>
#include <params/include/conv2.h>
#include <params/include/conv3.h>
#include <params/include/fc1.h>
#include <params/include/fc2.h>

uint16_t LeNet(mat_t* model_in) {
  memcpy(conv1_in_meta.data, model_in->data, MAT_GET_SIZE(model_in)*sizeof(uint16_t));
  msp_send_printf("\n============== Input ==============\n");
  msp_send_mat(model_in);
  /* conv1 */
  memset(conv1_out_meta.data, 0, MAT_GET_SIZE(&conv1_out_meta)*sizeof(uint16_t));
  conv1(&conv1_in_meta, &conv1_out_meta, &conv1_weight_meta, &conv1_bias_meta);
  msp_send_printf("\n============== Output of conv1 ==============\n");
  msp_send_mat(&conv1_out_meta);
  /* Relu */
  memset(Relu_out_meta.data, 0, MAT_GET_SIZE(&Relu_out_meta)*sizeof(uint16_t));
  Relu(&Relu_in_meta, &Relu_out_meta);
  msp_send_printf("\n============== Output of Relu ==============\n");
  msp_send_mat(&Relu_out_meta);
  /* Clip */
  memset(Clip_out_meta.data, 0, MAT_GET_SIZE(&Clip_out_meta)*sizeof(uint16_t));
  Clip(&Clip_in_meta, &Clip_out_meta);
  msp_send_printf("\n============== Output of Clip ==============\n");
  msp_send_mat(&Clip_out_meta);
  /* pool1_AveragePool */
  memset(pool1_AveragePool_out_meta.data, 0, MAT_GET_SIZE(&pool1_AveragePool_out_meta)*sizeof(uint16_t));
  pool1_AveragePool(&pool1_AveragePool_in_meta, &pool1_AveragePool_out_meta);
  msp_send_printf("\n============== Output of pool1_AveragePool ==============\n");
  msp_send_mat(&pool1_AveragePool_out_meta);
  /* Clip_1 */
  memset(Clip_1_out_meta.data, 0, MAT_GET_SIZE(&Clip_1_out_meta)*sizeof(uint16_t));
  Clip_1(&Clip_1_in_meta, &Clip_1_out_meta);
  msp_send_printf("\n============== Output of Clip_1 ==============\n");
  msp_send_mat(&Clip_1_out_meta);
  /* conv2 */
  memset(conv2_out_meta.data, 0, MAT_GET_SIZE(&conv2_out_meta)*sizeof(uint16_t));
  conv2(&conv2_in_meta, &conv2_out_meta, &conv2_weight_meta, &conv2_bias_meta);
  msp_send_printf("\n============== Output of conv2 ==============\n");
  msp_send_mat(&conv2_out_meta);
  /* Relu_1 */
  memset(Relu_1_out_meta.data, 0, MAT_GET_SIZE(&Relu_1_out_meta)*sizeof(uint16_t));
  Relu_1(&Relu_1_in_meta, &Relu_1_out_meta);
  msp_send_printf("\n============== Output of Relu_1 ==============\n");
  msp_send_mat(&Relu_1_out_meta);
  /* Clip_2 */
  memset(Clip_2_out_meta.data, 0, MAT_GET_SIZE(&Clip_2_out_meta)*sizeof(uint16_t));
  Clip_2(&Clip_2_in_meta, &Clip_2_out_meta);
  msp_send_printf("\n============== Output of Clip_2 ==============\n");
  msp_send_mat(&Clip_2_out_meta);
  /* pool2_AveragePool */
  memset(pool2_AveragePool_out_meta.data, 0, MAT_GET_SIZE(&pool2_AveragePool_out_meta)*sizeof(uint16_t));
  pool2_AveragePool(&pool2_AveragePool_in_meta, &pool2_AveragePool_out_meta);
  msp_send_printf("\n============== Output of pool2_AveragePool ==============\n");
  msp_send_mat(&pool2_AveragePool_out_meta);
  /* Clip_3 */
  memset(Clip_3_out_meta.data, 0, MAT_GET_SIZE(&Clip_3_out_meta)*sizeof(uint16_t));
  Clip_3(&Clip_3_in_meta, &Clip_3_out_meta);
  msp_send_printf("\n============== Output of Clip_3 ==============\n");
  msp_send_mat(&Clip_3_out_meta);
  /* conv3 */
  memset(conv3_out_meta.data, 0, MAT_GET_SIZE(&conv3_out_meta)*sizeof(uint16_t));
  conv3(&conv3_in_meta, &conv3_out_meta, &conv3_weight_meta, &conv3_bias_meta);
  msp_send_printf("\n============== Output of conv3 ==============\n");
  msp_send_mat(&conv3_out_meta);
  /* Relu_2 */
  memset(Relu_2_out_meta.data, 0, MAT_GET_SIZE(&Relu_2_out_meta)*sizeof(uint16_t));
  Relu_2(&Relu_2_in_meta, &Relu_2_out_meta);
  msp_send_printf("\n============== Output of Relu_2 ==============\n");
  msp_send_mat(&Relu_2_out_meta);
  /* Clip_4 */
  memset(Clip_4_out_meta.data, 0, MAT_GET_SIZE(&Clip_4_out_meta)*sizeof(uint16_t));
  Clip_4(&Clip_4_in_meta, &Clip_4_out_meta);
  msp_send_printf("\n============== Output of Clip_4 ==============\n");
  msp_send_mat(&Clip_4_out_meta);
  /* Flatten */
  memset(Flatten_out_meta.data, 0, MAT_GET_SIZE(&Flatten_out_meta)*sizeof(uint16_t));
  Flatten(&Flatten_in_meta, &Flatten_out_meta);
  msp_send_printf("\n============== Output of Flatten ==============\n");
  msp_send_mat(&Flatten_out_meta);
  /* fc1 */
  memset(fc1_out_meta.data, 0, MAT_GET_SIZE(&fc1_out_meta)*sizeof(uint16_t));
  fc1(&fc1_in_meta, &fc1_out_meta, &fc1_weight_meta, &fc1_bias_meta);
  msp_send_printf("\n============== Output of fc1 ==============\n");
  msp_send_mat(&fc1_out_meta);
  /* Relu_3 */
  memset(Relu_3_out_meta.data, 0, MAT_GET_SIZE(&Relu_3_out_meta)*sizeof(uint16_t));
  Relu_3(&Relu_3_in_meta, &Relu_3_out_meta);
  msp_send_printf("\n============== Output of Relu_3 ==============\n");
  msp_send_mat(&Relu_3_out_meta);
  /* Clip_5 */
  memset(Clip_5_out_meta.data, 0, MAT_GET_SIZE(&Clip_5_out_meta)*sizeof(uint16_t));
  Clip_5(&Clip_5_in_meta, &Clip_5_out_meta);
  msp_send_printf("\n============== Output of Clip_5 ==============\n");
  msp_send_mat(&Clip_5_out_meta);
  /* fc2 */
  memset(fc2_out_meta.data, 0, MAT_GET_SIZE(&fc2_out_meta)*sizeof(uint16_t));
  fc2(&fc2_in_meta, &fc2_out_meta, &fc2_weight_meta, &fc2_bias_meta);
  msp_send_printf("\n============== Output of fc2 ==============\n");
  msp_send_mat(&fc2_out_meta);
  /* Clip_6 */
  memset(Clip_6_out_meta.data, 0, MAT_GET_SIZE(&Clip_6_out_meta)*sizeof(uint16_t));
  Clip_6(&Clip_6_in_meta, &Clip_6_out_meta);
  msp_send_printf("\n============== Output of Clip_6 ==============\n");
  msp_send_mat(&Clip_6_out_meta);
  /* Get the max score */
  uint16_t max = 0;
  for (uint16_t i = 0; i < Clip_6_out_meta.dims[1]; ++i) {
    max = Clip_6_out_meta.data[max] > Clip_6_out_meta.data[i] ? max : i;
  }

  return max;
}