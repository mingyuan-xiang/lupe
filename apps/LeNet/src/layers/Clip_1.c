#include <layers/include/utils.h>
#include <layers/include/Clip_1.h>
#include <buffer/include/buffer.h>
#include <libmspsyncioutils/mspsyncioutils.h>
#include <libmsptimer/timekeeper.h>

void Clip_1(mat_t* input, mat_t* output) {
  uint16_t size = input->strides[0];
  for (uint16_t i = 0; i < size; ++i) {
    if (input->data[i] > 8192) {
      output->data[i] = 8192;
    } else if (input->data[i] < -8192) {
      output->data[i] = -8192;
    } else {
      output->data[i] = input->data[i];
    }
  }
}