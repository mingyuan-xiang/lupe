#include <include/utils.h>
#include <include/conv.h>
#include <include/lupe_buffer.h>
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
  uint16_t input_size = input->strides[0];

  uintptr_t output_fram_addr = (uintptr_t)(output->data);
  uintptr_t input_fram_addr = (uintptr_t)(input->data);
  uintptr_t input_channel_addr = input_fram_addr;
  uintptr_t tmp_input_addr = input_channel_addr;
  uintptr_t output_addr_offset = output_len * sizeof(int16_t);

  uint16_t input_line_size = input->dims[3];

  uint16_t lea_remain_size = _LEA_REMAIN_SIZE;
  uint16_t lea_remain_size_aligned = MAKE_ALIGN_2(lea_remain_size);
  uintptr_t remain_size_offset = lea_remain_size * sizeof(int16_t);
  uintptr_t lea_min_size_offset = _LEA_DST_SIZE * sizeof(int16_t);

  uintptr_t lea_src_addr = (uintptr_t)lea_src;
  uintptr_t lea_dst_addr = (uintptr_t)lea_dst;
  msp_add_q15_params add_params = { .length = lea_remain_size_aligned };
  msp_fill_q15_params fill_params = { .length = lea_remain_size_aligned, .value = 0 };
  msp_mpy_q15_params mpy_params = { .length = lea_remain_size_aligned };

  uint16_t weight_pos = 0;
  uint16_t tmp_weight_pos = 0;
  
  uint16_t out_bias_pos = 0;
  /* add bias */
  for (uint16_t i = 0; i < out_channels; ++i) {
    for (uint16_t j = 0; j < output_len; ++j) {
      output->data[out_bias_pos] = bias->data[i];
      ++out_bias_pos;
    }
  }

  /* get strided input */
  uint16_t ptwise_pos = 0;
  for (uint16_t n = 0; n < input_size; n += input_len) {
    uint16_t r_end = n + input_len;
    for (uint16_t r = n; r < r_end; r += _STRIDE_ROW_LINE_SIZE) {
      uint16_t m_end = r + input_line_size;
      for (uint16_t m = r; m < m_end; m += _STRIDE_COL_SIZE) {
        lupe_buffer_meta.data[ptwise_pos] = input->data[m];
        ++ptwise_pos;
      }
    }
  }

  /* convolution */
  for (uint16_t i = 0; i < out_channels; ++i) {
    input_channel_addr = input_fram_addr;
    
    DMA_makeTransfer(output_fram_addr, lea_dst_addr, lea_remain_size);
    
    add_params.length = lea_remain_size_aligned;
    fill_params.length = lea_remain_size_aligned;
    mpy_params.length = lea_remain_size_aligned;
    
    tmp_input_addr = input_channel_addr;
    tmp_weight_pos = weight_pos;
    for (uint16_t j = 0; j < in_channels; ++j) {
      DMA_makeTransfer(tmp_input_addr, lea_src_addr, lea_remain_size);
      
      fill_params.value = weight->data[tmp_weight_pos];

      msp_fill_q15(&fill_params, lea_flt);

      msp_mpy_q15(&mpy_params, lea_src, lea_flt, lea_flt);
      
      /* right shift the output to match up the fixed point format */
      msp_add_q15(&add_params, lea_flt, lea_flt, lea_flt);
      
      msp_add_q15(&add_params, lea_flt, lea_dst, lea_dst);
      
      ++tmp_weight_pos;
      tmp_input_addr += output_addr_offset;
    }
    DMA_makeTransfer(lea_dst_addr, output_fram_addr, lea_remain_size);
    input_channel_addr += remain_size_offset;
    output_fram_addr += remain_size_offset;
    
    
    add_params.length = _LEA_DST_SIZE;
    fill_params.length = _LEA_DST_SIZE;
    mpy_params.length = _LEA_DST_SIZE;
    for (uint16_t m = lea_remain_size; m < output_len; m += _LEA_DST_SIZE) {
      DMA_makeTransfer(output_fram_addr, lea_dst_addr, _LEA_DST_SIZE);

      tmp_input_addr = input_channel_addr;
      tmp_weight_pos = weight_pos;
      for (uint16_t j = 0; j < in_channels; ++j) {
        DMA_makeTransfer(tmp_input_addr, lea_src_addr, _LEA_DST_SIZE);
        
        fill_params.value = weight->data[tmp_weight_pos];

        msp_fill_q15(&fill_params, lea_flt);

        msp_mpy_q15(&mpy_params, lea_src, lea_flt, lea_flt);
        
        /* right shift the output to match up the fixed point format */
        msp_add_q15(&add_params, lea_flt, lea_flt, lea_flt);
        
        msp_add_q15(&add_params, lea_flt, lea_dst, lea_dst);
        
        ++tmp_weight_pos;
        tmp_input_addr += output_addr_offset;
      }
      DMA_makeTransfer(lea_dst_addr, output_fram_addr, _LEA_DST_SIZE);
      input_channel_addr += lea_min_size_offset;
      output_fram_addr += lea_min_size_offset;
    }
    weight_pos += in_channels;
  }
}