#include <layers/include/utils.h>
#include <layers/include/conv2_Conv.h>
#include <buffer/include/buffer.h>
#include <layers/include/intermittent.h>



#define _LEA_SRC_SIZE 400
#define _INPUT_SIZE 10240
#define _LEA_REMAIN_SIZE (_INPUT_SIZE % _LEA_SRC_SIZE)



void conv2_Conv(mat_t* input, mat_t* input1, mat_t* output) {
  uintptr_t lea_line_size_offset = _LEA_SRC_SIZE * sizeof(int16_t);
  uint16_t lea_input_line_remain_size = _LEA_REMAIN_SIZE;
  uintptr_t lea_input_line_remain_size_offset = lea_input_line_remain_size*sizeof(int16_t);

  uintptr_t input_0_fram_addr = (uintptr_t)input1->data;
  uintptr_t input_1_fram_addr = (uintptr_t)input->data;
  uintptr_t output_fram_addr = (uintptr_t)output->data;

  uintptr_t intermittent_buffer_addr = (uintptr_t)intermittent_buffer;

  
  msp_add_q15_params add_params = { .length = MAKE_ALIGN_2(lea_input_line_remain_size) };

  if (intermittent_status[COMPUTE_CK] == INTERMITTENT_LeNet_START) {
    if (intermittent_status[COMPUTE_IO_ROW] & DOUBLE_BUFFER_WRITE) {
      /* Start transferring to output buffer. Recover from temporary buffer */
      uint16_t line = intermittent_status[COMPUTE_IO_ROW] & DOUBLE_BUFFER_COMPLETE;
      uint16_t size = line;
      if (size > lea_input_line_remain_size) {
        size = _LEA_SRC_SIZE;
      }
      uintptr_t addr = (uintptr_t)(output->data) + (line - size) * sizeof(int16_t);

      DMA_makeTransfer(intermittent_buffer_addr, addr, size);

      intermittent_status[COMPUTE_IO_ROW] = line;
    }
    if (intermittent_status[COMPUTE_IO_ROW] == 0) {
      /*
      * Do the remaining size ADD when the input size is not a multiple
      * of the lea buffer size
      */
      DMA_makeTransfer(input_0_fram_addr, (uintptr_t)lea_src, lea_input_line_remain_size);
      DMA_makeTransfer(input_1_fram_addr, (uintptr_t)lea_tmp, lea_input_line_remain_size);
      msp_add_q15(&add_params, lea_src, lea_tmp, lea_src);

      uint16_t next_r = lea_input_line_remain_size;
      DOUBLE_BUFFER_TRANSFER(next_r, COMPUTE_IO_ROW, (uintptr_t)lea_src, output_fram_addr, lea_input_line_remain_size);

      input_0_fram_addr += lea_input_line_remain_size_offset;
      input_1_fram_addr += lea_input_line_remain_size_offset;
      output_fram_addr += lea_input_line_remain_size_offset;
    }

    /* Compute the rest ADD */
    add_params.length = MAKE_ALIGN_2(_LEA_SRC_SIZE);
    for (uint16_t l = intermittent_status[COMPUTE_IO_ROW]; l < _INPUT_SIZE; l += _LEA_SRC_SIZE) {
      DMA_makeTransfer(input_0_fram_addr, (uintptr_t)lea_src, _LEA_SRC_SIZE);
      DMA_makeTransfer(input_1_fram_addr, (uintptr_t)lea_tmp, _LEA_SRC_SIZE);
      msp_add_q15(&add_params, lea_src, lea_tmp, lea_src);

      uint16_t next_r = l + _LEA_SRC_SIZE;
      DOUBLE_BUFFER_TRANSFER(next_r, COMPUTE_IO_ROW, (uintptr_t)lea_src, output_fram_addr, _LEA_SRC_SIZE);

      input_0_fram_addr += lea_line_size_offset;
      input_1_fram_addr += lea_line_size_offset;
      output_fram_addr += lea_line_size_offset;
    }
  }

  if (intermittent_status[COMPUTE_CK] == INTERMITTENT_LeNet_END) {
    intermittent_status[COMPUTE_IO_ROW] = 0;
  }
}