#include <include/utils.h>
#include <include/conv.h>
#include <include/intermittent.h>

#define _LEA_DST_SIZE 120
#define _LEA_REMAIN_SIZE (400 % _LEA_DST_SIZE)

#define _INPUT_LINE_SIZE 20
#define _STRIDE_ROW_SIZE 1
#define _STRIDE_ROW_LINE_SIZE (_STRIDE_ROW_SIZE * _INPUT_LINE_SIZE)
#define _STRIDE_COL_SIZE 1



void conv_exp(mat_t* input, mat_t* output, mat_t* weight, mat_t* bias) {
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

  uint16_t lea_remain_size = _LEA_REMAIN_SIZE;
  uint16_t lea_remain_size_aligned = MAKE_ALIGN_2(lea_remain_size);
  uintptr_t output_remain_size_offset = lea_remain_size * sizeof(int16_t);
  uintptr_t output_lea_min_size_offset = _LEA_DST_SIZE * sizeof(int16_t);

  uintptr_t lea_src_addr = (uintptr_t)lea_src;
  uintptr_t lea_dst_addr = (uintptr_t)lea_dst;
  
  msp_mac_q15_params mac_params = { .length = MAKE_ALIGN_2(in_channels) };
  msp_offset_q15_params offset_params = { .length = lea_remain_size_aligned, .offset = 0 };

  /* Rebuild the input for MAC */
  uint16_t ptwise_pos = 0;
  for (uint16_t r = 0; r < input_len; r += _STRIDE_ROW_LINE_SIZE) {
    uint16_t m_end = r + input_line_size;
    for (uint16_t m = r; m < m_end; m += _STRIDE_COL_SIZE) {
      uint16_t input_pos = m;
      for (uint16_t n = 0; n < in_channels; ++n) {
        output->data[ptwise_pos] = input->data[input_pos];
        ++ptwise_pos;
        input_pos += input_len;
      }
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
}