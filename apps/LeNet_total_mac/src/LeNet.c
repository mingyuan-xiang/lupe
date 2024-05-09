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
#include <layers/include/pool1_AveragePool.h>
#include <layers/include/conv2.h>
#include <layers/include/Relu_1.h>
#include <layers/include/pool2_AveragePool.h>
#include <layers/include/conv3.h>
#include <layers/include/Relu_2.h>
#include <layers/include/Flatten.h>
#include <layers/include/fc1.h>
#include <layers/include/Relu_3.h>
#include <layers/include/fc2.h>

#include <params/include/conv1.h>
#include <params/include/conv2.h>
#include <params/include/conv3.h>
#include <params/include/fc1.h>
#include <params/include/fc2.h>

uint16_t LeNet(mat_t* model_in) {
  uint32_t time = 0;

  memcpy(conv1_in_meta.data, model_in->data, MAT_GET_SIZE(model_in)*sizeof(uint16_t));
  msp_send_printf("\n============== Input ==============\n");
  /* conv1 */
  start_timer();
  memset(conv1_out_meta.data, 0, MAT_GET_SIZE(&conv1_out_meta)*sizeof(uint16_t));
  conv1(&conv1_in_meta, &conv1_out_meta, &conv1_weight_meta, &conv1_bias_meta);
  time = stop_timer();
  msp_send_printf("\n============== Output of conv1 ==============\n");
  msp_send_printf("total time: %n", time);
  /* Relu */
  start_timer();
  memset(Relu_out_meta.data, 0, MAT_GET_SIZE(&Relu_out_meta)*sizeof(uint16_t));
  Relu(&Relu_in_meta, &Relu_out_meta);
  time = stop_timer();
  msp_send_printf("\n============== Output of Relu ==============\n");
  msp_send_printf("total time: %n", time);
  /* pool1_AveragePool */
  start_timer();
  memset(pool1_AveragePool_out_meta.data, 0, MAT_GET_SIZE(&pool1_AveragePool_out_meta)*sizeof(uint16_t));
  pool1_AveragePool(&pool1_AveragePool_in_meta, &pool1_AveragePool_out_meta);
  time = stop_timer();
  msp_send_printf("\n============== Output of pool1_AveragePool ==============\n");
  msp_send_printf("total time: %n", time);
  /* conv2 */
  start_timer();
  memset(conv2_out_meta.data, 0, MAT_GET_SIZE(&conv2_out_meta)*sizeof(uint16_t));
  conv2(&conv2_in_meta, &conv2_out_meta, &conv2_weight_meta, &conv2_bias_meta);
  time = stop_timer();
  msp_send_printf("\n============== Output of conv2 ==============\n");
  msp_send_printf("total time: %n", time);
  /* Relu_1 */
  start_timer();
  memset(Relu_1_out_meta.data, 0, MAT_GET_SIZE(&Relu_1_out_meta)*sizeof(uint16_t));
  Relu_1(&Relu_1_in_meta, &Relu_1_out_meta);
  time = stop_timer();
  msp_send_printf("\n============== Output of Relu_1 ==============\n");
  msp_send_printf("total time: %n", time);
  /* pool2_AveragePool */
  start_timer();
  memset(pool2_AveragePool_out_meta.data, 0, MAT_GET_SIZE(&pool2_AveragePool_out_meta)*sizeof(uint16_t));
  pool2_AveragePool(&pool2_AveragePool_in_meta, &pool2_AveragePool_out_meta);
  time = stop_timer();
  msp_send_printf("\n============== Output of pool2_AveragePool ==============\n");
  msp_send_printf("total time: %n", time);
  /* conv3 */
  start_timer();
  memset(conv3_out_meta.data, 0, MAT_GET_SIZE(&conv3_out_meta)*sizeof(uint16_t));
  conv3(&conv3_in_meta, &conv3_out_meta, &conv3_weight_meta, &conv3_bias_meta);
  time = stop_timer();
  msp_send_printf("\n============== Output of conv3 ==============\n");
  msp_send_printf("total time: %n", time);
  /* Relu_2 */
  start_timer();
  memset(Relu_2_out_meta.data, 0, MAT_GET_SIZE(&Relu_2_out_meta)*sizeof(uint16_t));
  Relu_2(&Relu_2_in_meta, &Relu_2_out_meta);
  time = stop_timer();
  msp_send_printf("\n============== Output of Relu_2 ==============\n");
  msp_send_printf("total time: %n", time);
  /* Flatten */
  memset(Flatten_out_meta.data, 0, MAT_GET_SIZE(&Flatten_out_meta)*sizeof(uint16_t));
  Flatten(&Flatten_in_meta, &Flatten_out_meta);
  msp_send_printf("\n============== Output of Flatten ==============\n");
  /* fc1 */
  start_timer();
  memset(fc1_out_meta.data, 0, MAT_GET_SIZE(&fc1_out_meta)*sizeof(uint16_t));
  fc1(&fc1_in_meta, &fc1_out_meta, &fc1_weight_meta, &fc1_bias_meta);
  time = stop_timer();
  msp_send_printf("\n============== Output of fc1 ==============\n");
  msp_send_printf("total time: %n", time);
  /* Relu_3 */
  start_timer();
  memset(Relu_3_out_meta.data, 0, MAT_GET_SIZE(&Relu_3_out_meta)*sizeof(uint16_t));
  Relu_3(&Relu_3_in_meta, &Relu_3_out_meta);
  time = stop_timer();
  msp_send_printf("\n============== Output of Relu_3 ==============\n");
  msp_send_printf("total time: %n", time);
  /* fc2 */
  start_timer();
  memset(fc2_out_meta.data, 0, MAT_GET_SIZE(&fc2_out_meta)*sizeof(uint16_t));
  fc2(&fc2_in_meta, &fc2_out_meta, &fc2_weight_meta, &fc2_bias_meta);
  time = stop_timer();
  msp_send_printf("\n============== Output of fc2 ==============\n");
  msp_send_printf("total time: %n", time);
  /* Get the max score */
  uint16_t max = 0;
  for (uint16_t i = 0; i < fc2_out_meta.dims[1]; ++i) {
    max = fc2_out_meta.data[max] > fc2_out_meta.data[i] ? max : i;
  }

  return max;
}