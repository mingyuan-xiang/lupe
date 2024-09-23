#include <layers/include/utils.h>
#include <layers/include/pool1_AveragePool.h>
#include <buffer/include/buffer.h>
#include <layers/include/intermittent.h>

void pool1_AveragePool(mat_t* input, mat_t* output) {
  uint16_t size = input->strides[0];

  /* No need to use double buffering for relu as it doesn't matter. */
  if (intermittent_status[COMPUTE_CK] == INTERMITTENT_LeNet_START) {
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

    intermittent_status[COMPUTE_CK] = INTERMITTENT_relu1_Clip_EXIT;
  }

  if (intermittent_status[COMPUTE_CK] == INTERMITTENT_relu1_Clip_EXIT) {
    intermittent_status[COMPUTE_IO_COL] = 0;
  }
}