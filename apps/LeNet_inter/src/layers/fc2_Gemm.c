#include <layers/include/utils.h>
#include <layers/include/fc2_Gemm.h>
#include <buffer/include/buffer.h>
#include <layers/include/intermittent.h>

#define _LEA_SRC_SIZE 400


void fc2_Gemm(mat_t* input, mat_t* output, mat_t* weight, mat_t* bias) {
  uint16_t input_size = input->dims[1];
  uint16_t output_size = output->dims[1];
  uint16_t lea_input_line_remain_size = input_size % _LEA_SRC_SIZE;
  uintptr_t weight_line_offset = (weight->dims[1]) * sizeof(int16_t);

  uintptr_t input_fram_addr = (uintptr_t)input->data;
  msp_mac_q15_params mac_params = { .length = MAKE_ALIGN_2(lea_input_line_remain_size) };
  int16_t res;

  if (intermittent_status[COMPUTE_CK] == INTERMITTENT_fc2_Gemm_BIAS) {
    /* add bias first so that we don't need to set the input to 0 */
    DMA_makeTransfer((uintptr_t)(bias->data), (uintptr_t)(output->data), output_size);

    intermittent_status[COMPUTE_CK] = INTERMITTENT_fc2_Gemm_MAIN;
  }

  if (intermittent_status[COMPUTE_CK] == INTERMITTENT_fc2_Gemm_MAIN) {
    if (intermittent_status[COMPUTE_IO_COL] & DOUBLE_BUFFER_WRITE) {
      uint16_t idx = intermittent_status[COMPUTE_IO_COL] & DOUBLE_BUFFER_COMPLETE;
      output->data[i] = intermittent_buffer[0];

      intermittent_status[COMPUTE_IO_COL] = idx;
    }

    uintptr_t weight_fram_addr = (uintptr_t)weight->data + \
      intermittent_status[COMPUTE_IO_COL] * weight_line_offset;
    /*
    * Do the remaining size MAC when the input size is not a multiple
    * of the lea buffer size
    */
    /* send input neurons */
    DMA_makeTransfer(input_fram_addr, (uintptr_t)lea_src, lea_input_line_remain_size);

    for (uint16_t i = intermittent_status[COMPUTE_IO_COL]; i < output_size; ++i) {
      /* send weight row */
      DMA_makeTransfer(weight_fram_addr, (uintptr_t)lea_tmp, lea_input_line_remain_size);
      msp_mac_q15(&mac_params, lea_src, lea_tmp, lea_res);

      res = (int16_t)(lea_res[0] >> 15);
      res = __saturated_add_q15(output->data[i], res);
      uint16_t next_i = i + 1;
      DOUBLE_BUFFER_WRITE(next_i, COMPUTE_IO_COL, res, output->data[i])

      weight_fram_addr += weight_line_offset;
    }

    intermittent_status[COMPUTE_CK] = INTERMITTENT_fc2_Gemm_EXIT;
  }

  if (intermittent_status[COMPUTE_CK] == INTERMITTENT_fc2_Gemm_EXIT) {
    intermittent_status[COMPUTE_IO_COL] = 0;
  }
}