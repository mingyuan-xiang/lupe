#include <layers/include/utils.h>
#include <layers/include/conv2_Conv.h>
#include <buffer/include/buffer.h>
#include <layers/include/intermittent.h>

/*
 * _LEA_SRC_SIZE will always be mutiple of _FLT_LEN
 * so that _LEA_REMAIN_SIZE will always be multiple of _FLT_LEN.
 */
#define _KERNEL_ROW_SIZE 5
#define _KERNEL_COL_SIZE 5
#define _FLT_LEN (_KERNEL_ROW_SIZE * _KERNEL_COL_SIZE)
#define __LEA_SRC_SIZE 100
#define _LEA_SRC_SIZE (__LEA_SRC_SIZE - (__LEA_SRC_SIZE % _FLT_LEN))
#define _IN_CHANNEL_NUM 6
#define _MAC_LEN (_FLT_LEN * _IN_CHANNEL_NUM)
#define _LEA_REMAIN_SIZE (_MAC_LEN % _LEA_SRC_SIZE)
#define _LEA_REMAIN_SIZE_CHANNEL_CNT (_LEA_REMAIN_SIZE / _FLT_LEN)
#define _LEA_SRC_SIZE_CHANNEL_CNT (_LEA_SRC_SIZE / _FLT_LEN)

#define _LEA_ADD_SIZE 100

#define _LEA_ADD_REMAIN_SIZE (100 % _LEA_ADD_SIZE)


#define _INPUT_LINE_SIZE 14
#define _STRIDE_ROW_SIZE 1
#define _STRIDE_COL_SIZE 1
#define _STRIDE_COL_OFFSET (_STRIDE_COL_SIZE * sizeof(int16_t))


void conv(mat_t* input, mat_t* output, mat_t* weight) {
  uint16_t out_channels = output->dims[1];
  uint16_t output_len = output->strides[1];
  uint16_t input_len = input->strides[1];
  uint16_t kernel_row_size = _KERNEL_ROW_SIZE;
  uint16_t kernel_col_size = _KERNEL_COL_SIZE;

  uintptr_t flt_lea_addr = (uintptr_t)lea_flt;
  uintptr_t flt_fram_addr = (uintptr_t)(weight->data);
  uintptr_t mac_size = _LEA_REMAIN_SIZE;
  uintptr_t flt_channel_remain_offset = _LEA_REMAIN_SIZE * sizeof(int16_t);
  uintptr_t flt_channel_src_offset = _LEA_SRC_SIZE * sizeof(int16_t);
  uintptr_t flt_channel_offset = weight->strides[0] * sizeof(int16_t);
  uintptr_t flt_addr_col_offset = kernel_col_size * sizeof(int16_t);
  uintptr_t input_fram_addr = (uintptr_t)(input->data);
  uintptr_t input_channel_offset = input_len * sizeof(int16_t);
  uintptr_t lea_src_addr = (uintptr_t)lea_src;

  uint16_t lea_mac_remain_size_aligned = MAKE_ALIGN_2(_LEA_REMAIN_SIZE);
  uint16_t lea_mac_size_aligned = MAKE_ALIGN_2(_LEA_SRC_SIZE);

  uint16_t input_line_size = input->dims[3];
  uint16_t output_line_size = output->dims[3];
  uint16_t output_line_num = output->dims[2];
  uint16_t input_line_size_offset = input_line_size * sizeof(int16_t);
  uint16_t input_line_strided_size = input_line_size * _STRIDE_ROW_SIZE;
  uint16_t input_line_size_strided_offset = input_line_strided_size * sizeof(int16_t);

  uint16_t lea_remain_size = _LEA_ADD_REMAIN_SIZE;
  uintptr_t output_lea_min_size_offset = _LEA_ADD_SIZE * sizeof(int16_t);
  uint16_t lea_remain_size_aligned = MAKE_ALIGN_2(lea_remain_size);
  msp_mac_q15_params mac_params = { .length = lea_mac_remain_size_aligned };
  msp_add_q15_params add_params = { .length = lea_remain_size_aligned };
  msp_offset_q15_params offset_params = { .length = lea_remain_size_aligned, .offset = 0 };

  uintptr_t intermittent_buffer_addr = (uintptr_t)intermittent_buffer;
  uintptr_t intermittent_mac_buffer_addr = intermittent_buffer_addr + sizeof(int16_t);

  /* No need for double buffering (for loop indices) as each output is independant */
  if (intermittent_status[COMPUTE_CK] == INTERMITTENT_LeNet_i_START) { 

    VOLATILE_WRITE(MAC_PREPARE, COMPUTE_SWITCH);
    VOLATILE_WRITE(INTERMITTENT_conv2_Conv_MAIN, COMPUTE_CK);
  }

  /* convolution */
  if (intermittent_status[COMPUTE_CK] == INTERMITTENT_conv2_Conv_MAIN) {
    if (intermittent_status[COMPUTE_SWITCH] == MAC_COMPUTE) {
      if (intermittent_status[COMPUTE_OUT_CH] & DOUBLE_BUFFER_WRITE) {
        uint16_t idx = intermittent_status[COMPUTE_OUT_CH] & DOUBLE_BUFFER_COMPLETE;
        uint16_t pos = intermittent_status[COMPUTE_IO_ROW] * output_line_size + \
          intermittent_status[COMPUTE_IO_COL] + (idx - 1) * output_len;
        output->data[pos] = intermittent_buffer[0];
        VOLATILE_WRITE(idx, COMPUTE_OUT_CH);
      }

      if (intermittent_status[COMPUTE_OUT_CH] >= out_channels) {
        VOLATILE_WRITE(MAC_END, COMPUTE_SWITCH);
      } else {
        uint16_t size = _LEA_REMAIN_SIZE;
        if (intermittent_status[COMPUTE_IN_CH] != 0) {
          size = _LEA_SRC_SIZE;
        }

        DMA_makeTransfer(intermittent_mac_buffer_addr, lea_src_addr, size);
      }
    }

    if (intermittent_status[COMPUTE_IN_CH] & DOUBLE_BUFFER_WRITE) {
      uint16_t idx = intermittent_status[COMPUTE_IN_CH] & DOUBLE_BUFFER_COMPLETE;
      VOLATILE_WRITE(MAC_PREPARE, COMPUTE_SWITCH);
      VOLATILE_WRITE(idx, COMPUTE_IN_CH);
    }

    if (intermittent_status[COMPUTE_IN_CH] >= _MAC_LEN) {
      uint16_t next_idx = intermittent_status[COMPUTE_IO_COL] + 1;
      DOUBLE_BUFFER_ASSIGN(next_idx, COMPUTE_IO_COL, 0, intermittent_status[COMPUTE_IN_CH]);
    }

    if (intermittent_status[COMPUTE_IO_COL] & DOUBLE_BUFFER_WRITE) {
      uint16_t idx = intermittent_status[COMPUTE_IO_COL] & DOUBLE_BUFFER_COMPLETE;
      VOLATILE_WRITE(intermittent_buffer[0], COMPUTE_IN_CH);
      VOLATILE_WRITE(idx, COMPUTE_IO_COL);
    }

    if (intermittent_status[COMPUTE_IO_COL] >= output_line_size) {
      uint16_t next_idx = intermittent_status[COMPUTE_IO_ROW] + 1;
      DOUBLE_BUFFER_ASSIGN(next_idx, COMPUTE_IO_ROW, 0, intermittent_status[COMPUTE_IO_COL]);
    }

    if (intermittent_status[COMPUTE_IO_ROW] & DOUBLE_BUFFER_WRITE) {
      uint16_t idx = intermittent_status[COMPUTE_IO_ROW] & DOUBLE_BUFFER_COMPLETE;
      VOLATILE_WRITE(intermittent_buffer[0], COMPUTE_IO_COL);
      VOLATILE_WRITE(idx, COMPUTE_IO_ROW);
    }

    uint16_t out_pos = intermittent_status[COMPUTE_IO_ROW] * output_line_size + \
      intermittent_status[COMPUTE_IO_COL];
    uint16_t tmp_out_pos;
    input_fram_addr += intermittent_status[COMPUTE_IO_ROW] * input_line_size_strided_offset;

    for (uint16_t m = intermittent_status[COMPUTE_IO_ROW]; m < output_line_num; ++m) {
      uintptr_t tmp_input_addr = input_fram_addr + intermittent_status[COMPUTE_IO_COL] * _STRIDE_COL_OFFSET;
      for (uint16_t n = intermittent_status[COMPUTE_IO_COL]; n < output_line_size; ++n) {
        /* Use intermittent_buffer to hold reshaped input */
        uintptr_t mac_buffer_addr = lea_src_addr;
        uintptr_t flt_tmp_addr = flt_fram_addr + intermittent_status[COMPUTE_IN_CH] * sizeof(int16_t);

  /*************************************************************
  * Do the reminder of MAC first
  ************************************************************/
        if (intermittent_status[COMPUTE_IN_CH] == 0) {
          uintptr_t tmp_channel_addr = tmp_input_addr;
          uintptr_t flt_mac_addr = flt_tmp_addr + intermittent_status[COMPUTE_OUT_CH] * flt_channel_offset;

          switch (intermittent_status[COMPUTE_SWITCH]) {
          case MAC_PREPARE: {
            /* assemble input to a matrix in mac_buffer */
            for (uint16_t i = 0; i < _LEA_REMAIN_SIZE_CHANNEL_CNT; ++i) {
              uintptr_t tmp_input_row_addr = tmp_channel_addr;
              for (uint16_t k = 0; k < kernel_row_size; ++k) {
                DMA_makeTransfer(tmp_input_row_addr, mac_buffer_addr, kernel_col_size);

                tmp_input_row_addr += input_line_size_offset;
                mac_buffer_addr += flt_addr_col_offset;
              }
              tmp_channel_addr += input_channel_offset;
            }

            DMA_makeTransfer(lea_src_addr, intermittent_mac_buffer_addr, mac_size)
            VOLATILE_WRITE(MAC_COMPUTE, COMPUTE_SWITCH);
          }
          case MAC_COMPUTE: {
            tmp_out_pos = out_pos + intermittent_status[COMPUTE_OUT_CH] * output_len;
            mac_size = _LEA_REMAIN_SIZE;
            mac_params.length = lea_mac_remain_size_aligned;
            for (uint16_t j = intermittent_status[COMPUTE_OUT_CH]; j < out_channels; ++j) {
              DMA_makeTransfer(flt_mac_addr, flt_lea_addr, mac_size);
              
              msp_mac_q15(&mac_params, lea_src, lea_flt, lea_res);

              int16_t mac_out = (int16_t)(lea_res[0] >> 16);
              uint16_t next_j = j + 1;
              DOUBLE_BUFFER_ASSIGN(next_j, COMPUTE_OUT_CH, mac_out, output->data[tmp_out_pos]);

              flt_mac_addr += flt_channel_offset;
              tmp_out_pos += output_len;
            }

            VOLATILE_WRITE(MAC_END, COMPUTE_SWITCH);
          }
          case MAC_END:
            flt_tmp_addr += flt_channel_remain_offset;
            VOLATILE_WRITE(0, COMPUTE_OUT_CH);
          }
          DOUBLE_BUFFER_ASSIGN(_LEA_REMAIN_SIZE, COMPUTE_IN_CH, MAC_PREPARE, intermittent_status[COMPUTE_SWITCH]);
        }

  /*************************************************************
  * Do the rest of MAC
  ************************************************************/

        mac_size = _LEA_SRC_SIZE;
        mac_params.length = lea_mac_size_aligned;
        uint16_t input_pos = (_LEA_REMAIN_SIZE_CHANNEL_CNT + \
          _LEA_SRC_SIZE_CHANNEL_CNT * (intermittent_status[COMPUTE_IN_CH] / _LEA_SRC_SIZE)) * input_len;
        uintptr_t tmp_channel_addr = tmp_input_addr + input_pos * sizeof(int16_t);
        for (uint16_t l = intermittent_status[COMPUTE_IN_CH]; l < _MAC_LEN; l += _LEA_SRC_SIZE) {
          uintptr_t flt_mac_addr = flt_tmp_addr + intermittent_status[COMPUTE_OUT_CH] * flt_channel_offset;
          tmp_out_pos = out_pos + intermittent_status[COMPUTE_OUT_CH] * output_len;
          mac_buffer_addr = lea_src_addr;

          switch (intermittent_status[COMPUTE_SWITCH]) {
          case MAC_PREPARE: {
            /* assemble input to a matrix in mac_buffer */
            for (uint16_t i = 0; i < _LEA_SRC_SIZE_CHANNEL_CNT; ++i) {
              uintptr_t tmp_input_row_addr = tmp_channel_addr;
              for (uint16_t k = 0; k < kernel_row_size; ++k) {
                DMA_makeTransfer(tmp_input_row_addr, mac_buffer_addr, kernel_col_size);

                tmp_input_row_addr += input_line_size_offset;
                mac_buffer_addr += flt_addr_col_offset;
              }
              tmp_channel_addr += input_channel_offset;
            }

            DMA_makeTransfer(lea_src_addr, intermittent_mac_buffer_addr, mac_size)
            VOLATILE_WRITE(MAC_COMPUTE, COMPUTE_SWITCH);
          }
          case MAC_COMPUTE: {
            for (uint16_t j = intermittent_status[COMPUTE_OUT_CH]; j < out_channels; ++j) {
              DMA_makeTransfer(flt_mac_addr, flt_lea_addr, mac_size);
              
              msp_mac_q15(&mac_params, lea_src, lea_flt, lea_res);

              int16_t mac_out = (int16_t)(lea_res[0] >> 16);
              mac_out =  __saturated_add_q15(mac_out, output->data[tmp_out_pos]);
              uint16_t next_j = j + 1;
              DOUBLE_BUFFER_ASSIGN(next_j, COMPUTE_OUT_CH, mac_out, output->data[tmp_out_pos]);

              flt_mac_addr += flt_channel_offset;
              tmp_out_pos += output_len;
            }

            VOLATILE_WRITE(MAC_END, COMPUTE_SWITCH);
          }
          case MAC_END:
            flt_tmp_addr += flt_channel_src_offset;
            VOLATILE_WRITE(0, COMPUTE_OUT_CH);
          }

          uint16_t next_l = l + _LEA_SRC_SIZE;
          DOUBLE_BUFFER_ASSIGN(next_l, COMPUTE_IN_CH, MAC_PREPARE, intermittent_status[COMPUTE_SWITCH]);
        }

        uint16_t next_n = n + 1;
        DOUBLE_BUFFER_ASSIGN(next_n, COMPUTE_IO_COL, 0, intermittent_status[COMPUTE_IN_CH]);
        tmp_input_addr += _STRIDE_COL_OFFSET;
        out_pos++;
      }

      uint16_t next_m = m + 1;
      DOUBLE_BUFFER_ASSIGN(next_m, COMPUTE_IO_ROW, 0, intermittent_status[COMPUTE_IO_COL]);
      input_fram_addr += input_line_size_strided_offset;
    }

    VOLATILE_WRITE(INTERMITTENT_conv2_Conv_EXIT, COMPUTE_CK);
  }

  if (intermittent_status[COMPUTE_CK] == INTERMITTENT_conv2_Conv_EXIT) {
    intermittent_status[COMPUTE_SWITCH] = PAD_START;
    intermittent_status[COMPUTE_IO_COL] = 0;
    intermittent_status[COMPUTE_IO_ROW] = 0;
    intermittent_status[COMPUTE_IN_CH] = 0;
    intermittent_status[COMPUTE_OUT_CH] = 0;
  }
}