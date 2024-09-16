#include <layers/include/utils.h>
#include <layers/include/relu2_Clip.h>
#include <buffer/include/buffer.h>

void relu2_Clip(mat_t* input, mat_t* output) {
  uint16_t size = input->strides[0];

  if (intermittent_status[COMPUTE_CK] == INTERMITTENT_relu2_Clip_MAIN) {
    for (uint16_t i = intermittent_status[COMPUTE_IO_COL]; i < size; ++i) {
      if (input->data[i] > 6144) {
        output->data[i] = 6144;
      } else if (input->data[i] < 0) {
        output->data[i] = 0;
      } else {
        output->data[i] = input->data[i];
      }

      intermittent_status[COMPUTE_IO_COL]++;
    }

    intermittent_status[COMPUTE_CK] = INTERMITTENT_relu2_Clip_EXIT;
  }

  if (intermittent_status[COMPUTE_CK] == INTERMITTENT_relu2_Clip_EXIT) {
    intermittent_status[COMPUTE_IO_COL] = 0;
  }
}