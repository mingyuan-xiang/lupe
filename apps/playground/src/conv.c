#include <include/utils.h>
#include <include/conv.h>
#include <include/intermittent.h>

#define _LEA_DST_SIZE 120
#define _LEA_REMAIN_SIZE (400 % _LEA_DST_SIZE)

#define _INPUT_LINE_SIZE 20
#define _STRIDE_ROW_SIZE 1
#define _STRIDE_ROW_LINE_SIZE (_STRIDE_ROW_SIZE * _INPUT_LINE_SIZE)
#define _STRIDE_COL_SIZE 1



void conv(mat_t* input, mat_t* output, mat_t* weight, mat_t* bias) {
  uint16_t in_channels = input->dims[1];
  uint16_t out_channels = output->dims[1];
  uint16_t output_len = output->strides[1];
  uint16_t input_len = input->strides[1];

  uintptr_t flt_lea_addr = (uintptr_t)lea_flt;
  uintptr_t flt_fram_addr = (uintptr_t)(weight->data);
  uintptr_t flt_addr_offset = in_channels * sizeof(int16_t);
  uintptr_t mac_fram_offset = in_channels * sizeof(int16_t);
  uintptr_t output_fram_addr = (uintptr_t)(output->data);

  uint16_t input_line_size = input->dims[3];
  uint16_t output_line_num = output->dims[2];
  uint16_t output_line_size = output->dims[3];

  uint16_t lea_remain_size = _LEA_REMAIN_SIZE;
  uint16_t lea_remain_size_aligned = MAKE_ALIGN_2(lea_remain_size);
  uintptr_t output_remain_size_offset = lea_remain_size * sizeof(int16_t);
  uintptr_t output_lea_min_size_offset = _LEA_DST_SIZE * sizeof(int16_t);

  uintptr_t lea_src_addr = (uintptr_t)lea_src;
  uintptr_t lea_dst_addr = (uintptr_t)lea_dst;
  
  msp_mac_q15_params mac_params = { .length = MAKE_ALIGN_2(in_channels) };
  msp_offset_q15_params offset_params = { .length = lea_remain_size_aligned, .offset = 0 };

  /* Rebuild the input for MAC */
    /* No need for double buffering (for loop indices) as each output is independent */
  if (intermittent_status[COMPUTE_CK] == INTERMITTENT_features_features_1_pw_conv_linear_pw_conv_linear_0_Conv_PREPARE) {
    switch (intermittent_status[COMPUTE_SWITCH]) {
    case RESHAPE_START: {
      uint16_t ptwise_pos = intermittent_status[COMPUTE_IO_ROW] * in_channels * output_line_size + \
        in_channels * intermittent_status[COMPUTE_IO_COL] + intermittent_status[COMPUTE_IN_CH];
      uint16_t input_row = intermittent_status[COMPUTE_IO_ROW] * _STRIDE_ROW_LINE_SIZE;
      for (uint16_t r = intermittent_status[COMPUTE_IO_ROW]; r < output_line_num; ++r) {
        uint16_t input_col = intermittent_status[COMPUTE_IO_COL] * _STRIDE_COL_SIZE;
        for (uint16_t m = intermittent_status[COMPUTE_IO_COL]; m < output_line_size; ++m) {
          uint16_t input_pos = input_row + input_col + input_len * intermittent_status[COMPUTE_IN_CH];
          for (uint16_t n = intermittent_status[COMPUTE_IN_CH]; n < in_channels; ++n) {
            output->data[ptwise_pos] = input->data[input_pos];

            uint16_t next_n = n + 1;
            VOLATILE_WRITE(next_n, COMPUTE_IN_CH);
            ++ptwise_pos;
            input_pos += input_len;
          }

          VOLATILE_WRITE(0, COMPUTE_IN_CH);
          uint16_t next_m = m + 1;
          VOLATILE_WRITE(next_m, COMPUTE_IO_COL);

          input_col += _STRIDE_COL_SIZE;
        }

        VOLATILE_WRITE(0, COMPUTE_IO_COL);
        uint16_t next_r = r + 1;
        VOLATILE_WRITE(next_r, COMPUTE_IO_ROW);

        input_row += _STRIDE_ROW_LINE_SIZE;
      }

      VOLATILE_WRITE(RESHAPE_ST_RESET, COMPUTE_SWITCH);
    }
    case RESHAPE_ST_RESET:
      VOLATILE_WRITE(0, COMPUTE_IO_COL); 
      VOLATILE_WRITE(0, COMPUTE_IO_ROW); 
      VOLATILE_WRITE(0, COMPUTE_IN_CH); 
      VOLATILE_WRITE(RESHAPE_END, COMPUTE_SWITCH);
    default:
      break;
    }
  }

  // /* convolution */
  // for (uint16_t i = 0; i < out_channels; ++i) {
  //   offset_params.offset = bias->data[i];
  //   uintptr_t mac_input_addr = (uintptr_t)lupe_buffer_meta.data;

  //   DMA_makeTransfer(flt_fram_addr, flt_lea_addr, in_channels);
  //   for (uint16_t l = 0; l < lea_remain_size; ++l) {
  //     DMA_makeTransfer(mac_input_addr, lea_src_addr, in_channels);
  //     msp_mac_q15(&mac_params, lea_src, lea_flt, lea_res);
  //     lea_dst[l] = (int16_t)(lea_res[0] >> 15);

  //     mac_input_addr += mac_fram_offset;
  //   }
  //   offset_params.length = lea_remain_size_aligned;
  //   msp_offset_q15(&offset_params, lea_dst, lea_dst);

  //   DMA_makeTransfer(lea_dst_addr, output_fram_addr, lea_remain_size);
  //   output_fram_addr += output_remain_size_offset;
  //   offset_params.length = _LEA_DST_SIZE;

  //   for (uint16_t m = lea_remain_size; m < output_len; m += _LEA_DST_SIZE) {
  //     for (uint16_t l = 0; l < _LEA_DST_SIZE; ++l) {
  //       DMA_makeTransfer(mac_input_addr, lea_src_addr, in_channels);
  //       msp_mac_q15(&mac_params, lea_src, lea_flt, lea_res);
  //       lea_dst[l] = (int16_t)(lea_res[0] >> 15);

  //       mac_input_addr += mac_fram_offset;
  //     }
  //     msp_offset_q15(&offset_params, lea_dst, lea_dst);

  //     DMA_makeTransfer(lea_dst_addr, output_fram_addr, _LEA_DST_SIZE);
  //     output_fram_addr += output_lea_min_size_offset;
  //   }
  //   flt_fram_addr += flt_addr_offset;
  // }

  if (intermittent_status[COMPUTE_CK] == INTERMITTENT_features_features_1_pw_conv_linear_pw_conv_linear_0_Conv_EXIT) {
    intermittent_status[COMPUTE_SWITCH] = PAD_START;
    intermittent_status[COMPUTE_IO_COL] = 0;
    intermittent_status[COMPUTE_IO_ROW] = 0;
    intermittent_status[COMPUTE_IN_CH] = 0;
    intermittent_status[COMPUTE_OUT_CH] = 0;
  }
}