#include <layers/include/utils.h>
#include <layers/include/relu3_Clip.h>
#include <buffer/include/buffer.h>

void relu3_Clip(mat_t* input, mat_t* output) {
  uint16_t size = input->strides[0];
  for (uint16_t i = 0; i < size; ++i) {
    if (input->data[i] > 6144) {
      output->data[i] = 6144;
    } else if (input->data[i] < 0) {
      output->data[i] = 0;
    } else {
      output->data[i] = input->data[i];
    }
  }
}