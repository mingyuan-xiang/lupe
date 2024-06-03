#include <layers/include/utils.h>
#include <layers/include/Clip_4.h>
#include <buffer/include/buffer.h>

void Clip_4(mat_t* input, mat_t* output) {
  uint16_t size = input->strides[0];
  for (uint16_t i = 0; i < size; ++i) {
    if (input->data[i] > 4096) {
      output->data[i] = 4096;
    } else if (input->data[i] < -4096) {
      output->data[i] = -4096;
    } else {
      output->data[i] = input->data[i];
    }
  }
}