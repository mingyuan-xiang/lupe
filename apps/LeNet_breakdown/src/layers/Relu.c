#include <layers/include/utils.h>
#include <layers/include/Relu.h>
#include <buffer/include/buffer.h>
#include <libmspsyncioutils/mspsyncioutils.h>
#include <libmsptimer/timekeeper.h>

void Relu(mat_t* input, mat_t* output) {
  uint16_t size = input->strides[0];
  
  for (uint16_t i = 0; i < size; ++i) {
    int16_t val = input->data[i];
    output->data[i] = val < 0 ? 0 : val;
  }
}