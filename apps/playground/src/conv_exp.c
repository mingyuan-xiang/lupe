#include <include/utils.h>
#include <include/conv.h>
#include <include/intermittent.h>

#define _LEA_ADD_SIZE 100

#define _LEA_REMAIN_SIZE (126 % _LEA_ADD_SIZE)
#define _PADDING_TOP 1
#define _PADDING_BOTTOM 1
#define _PADDING_RIGHT 1
#define _PADDING_LEFT 1

#define _STRIDE_ROW_SIZE 2
#define _STRIDE_COL_SIZE 2

#define _INPUT_ROW_SIZE 51
#define _INPUT_COL_SIZE 14
#define _KERNEL_ROW_SIZE 10
#define _KERNEL_COL_SIZE 4
#define _OUTPUT_ROW_SIZE (((_INPUT_ROW_SIZE - _KERNEL_ROW_SIZE) / _STRIDE_ROW_SIZE) + 1)
#define _OUTPUT_COL_SIZE (((_INPUT_COL_SIZE - _KERNEL_COL_SIZE) / _STRIDE_COL_SIZE) + 1)
#define _KERNEL_SIZE_ALIGNED MAKE_ALIGN_2(_KERNEL_COL_SIZE)

#define _FIR_OVERLAP_SIZE ((_KERNEL_SIZE_ALIGNED - (_KERNEL_SIZE_ALIGNED - _KERNEL_COL_SIZE) - 1) / _STRIDE_COL_SIZE)


#define __LEA_SRC_SIZE 100
/* _LEA_SRC_SIZE will always be mutiple of 2 */
#define _LEA_SRC_SIZE (__LEA_SRC_SIZE - (_KERNEL_SIZE_ALIGNED - _KERNEL_COL_SIZE) * 2)
#define _FIR_TOTAL_SIZE (_OUTPUT_ROW_SIZE * _INPUT_COL_SIZE)
#define _FIR_INPUT_SIZE (_LEA_SRC_SIZE - (_LEA_SRC_SIZE % _INPUT_COL_SIZE))
#define _FIR_INPUT_REMAIN_SIZE (_FIR_TOTAL_SIZE % _FIR_INPUT_SIZE)

#define _FIR_OUTPUT_REMAIN_SIZE (_FIR_INPUT_REMAIN_SIZE - _KERNEL_COL_SIZE + 1)
#define _FIR_OUTPUT_REMAIN_SIZE_ALIGNED MAKE_ALIGN_2(_FIR_OUTPUT_REMAIN_SIZE)
#define _FIR_ADD_OUTPUT_REMAIN_SIZE ((_FIR_INPUT_REMAIN_SIZE / _INPUT_COL_SIZE) * _OUTPUT_COL_SIZE)
#define _FIR_ADD_OUTPUT_REMAIN_SIZE_ALIGNED MAKE_ALIGN_2(_FIR_ADD_OUTPUT_REMAIN_SIZE)

#define _FIR_OUTPUT_SIZE (_FIR_INPUT_SIZE - _KERNEL_COL_SIZE + 1)
#define _FIR_OUTPUT_SIZE_ALIGNED MAKE_ALIGN_2(_FIR_OUTPUT_SIZE)
#define _FIR_ADD_OUTPUT_SIZE ((_FIR_INPUT_SIZE / _INPUT_COL_SIZE) * _OUTPUT_COL_SIZE)
#define _FIR_ADD_OUTPUT_SIZE_ALIGNED MAKE_ALIGN_2(_FIR_ADD_OUTPUT_SIZE)

#define _DEINTERLEAVE_REMAIN_SIZE (_FIR_OUTPUT_REMAIN_SIZE_ALIGNED / _STRIDE_COL_SIZE)
#define _DEINTERLEAVE_REMAIN_SIZE_ALIGNED MAKE_ALIGN_2(_DEINTERLEAVE_REMAIN_SIZE)
#define _DEINTERLEAVE_SIZE (_FIR_OUTPUT_SIZE_ALIGNED / _STRIDE_COL_SIZE)
#define _DEINTERLEAVE_SIZE_ALIGNED MAKE_ALIGN_2(_DEINTERLEAVE_SIZE)


void conv_exp(mat_t* input, mat_t* output, mat_t* weight, mat_t* bias) {
  uint16_t in_channels = input->dims[1];
  uint16_t out_channels = output->dims[1];
  uint16_t output_len = output->strides[1];
  uint16_t input_len = input->strides[1];
  uint16_t kernel_row_size = _KERNEL_ROW_SIZE;
  uint16_t kernel_col_size = _KERNEL_COL_SIZE;

  uintptr_t src_lea_addr = (uintptr_t)lea_src;
  uintptr_t dst_lea_addr = (uintptr_t)lea_dst;
  uintptr_t tmp_lea_addr = (uintptr_t)lea_tmp;

  uintptr_t flt_lea_addr = (uintptr_t)lea_flt;
  uintptr_t flt_fram_addr = (uintptr_t)(weight->data);
  uintptr_t input_lea_addr = src_lea_addr;
  uintptr_t input_fram_addr = (uintptr_t)(input->data);
  uintptr_t output_fram_addr = (uintptr_t)(output->data);
  uintptr_t input_channel_addr = input_fram_addr;
  uintptr_t input_channel_offset = input_len * sizeof(int16_t);
  uintptr_t output_addr_offset = output_len * sizeof(int16_t);
  
  uintptr_t lea_skip_addr = dst_lea_addr;
  uintptr_t lea_skip_addr_before_offset = (_OUTPUT_COL_SIZE + _FIR_OVERLAP_SIZE) * sizeof(int16_t);
  uintptr_t lea_skip_addr_before_st = lea_skip_addr + lea_skip_addr_before_offset;
  uintptr_t lea_skip_addr_before = lea_skip_addr_before_st;
  uintptr_t lea_skip_addr_after_offset = _OUTPUT_COL_SIZE * sizeof(int16_t);
  uintptr_t lea_skip_addr_after_st = lea_skip_addr + lea_skip_addr_after_offset;
  uintptr_t lea_skip_addr_after = lea_skip_addr_after_st;
  uintptr_t input_offset = _FIR_INPUT_SIZE * _STRIDE_ROW_SIZE * sizeof(int16_t);
  uintptr_t output_offset = _FIR_ADD_OUTPUT_SIZE * sizeof(int16_t);

  int16_t* conv_flt = lea_flt;
  uint16_t input_line_size = input->dims[3];
  uint16_t input_line_size_offset = input_line_size * sizeof(int16_t);
  
  uint16_t flt_len = kernel_row_size * kernel_col_size;
  uintptr_t flt_addr_offset = flt_len * sizeof(int16_t);

  uint16_t lea_remain_size = _LEA_REMAIN_SIZE;
  uintptr_t output_remain_size_offset = lea_remain_size * sizeof(int16_t);
  uintptr_t output_lea_min_size_offset = _LEA_ADD_SIZE * sizeof(int16_t);
  uint16_t lea_remain_size_aligned = MAKE_ALIGN_2(lea_remain_size);
  
  uint16_t input_line_strided_size_offset = input_line_size_offset * _STRIDE_ROW_SIZE;
  
  msp_deinterleave_q15_params deinterleave_params = {
    .length = _DEINTERLEAVE_REMAIN_SIZE_ALIGNED,
    .channel = 0,
    .numChannels = _STRIDE_COL_SIZE
  };
  msp_fir_q15_params conv_params = {
    .length = _FIR_OUTPUT_REMAIN_SIZE_ALIGNED,
    .tapLength = MAKE_ALIGN_2(kernel_col_size),
    .coeffs = lea_flt,
    .enableCircularBuffer = false 
  };
  msp_add_q15_params add_params = { .length = _FIR_OUTPUT_REMAIN_SIZE_ALIGNED };
  msp_offset_q15_params offset_params = { .length = lea_remain_size_aligned, .offset = 0 };

  

  /* Pad the input for (1, 1, 1, 1) */
  memset(output->data, 0, GET_MAT_SIZE(input)*sizeof(int16_t));
  uint16_t input_line_num = input->dims[2] - _PADDING_TOP - _PADDING_BOTTOM;
  uint16_t input_line_size_bf = input->dims[3] - _PADDING_RIGHT - _PADDING_LEFT;
  _q15* padding_ptr_in = input->data;
  _q15* padding_ptr_out = output->data;
  for (uint16_t i = 0; i < in_channels; ++i) {
    padding_ptr_out += input_line_size;
    for (uint16_t j = 0; j < input_line_num; ++j) {
      padding_ptr_out += _PADDING_LEFT;
      DMA_makeTransfer((uintptr_t)padding_ptr_in, (uintptr_t)padding_ptr_out, input_line_size_bf);
      padding_ptr_in += input_line_size_bf;
      padding_ptr_out += (_PADDING_RIGHT + input_line_size_bf);
    }
    padding_ptr_out += input_line_size;
  }

  DMA_makeTransfer(output_fram_addr, input_fram_addr, GET_MAT_SIZE(input));
  memset(output->data, 0, GET_MAT_SIZE(output)*sizeof(int16_t));

  /* convolution */
  for (uint16_t i = 0; i < out_channels; ++i) {
    input_fram_addr = (uintptr_t)(input->data);
    for (uint16_t j = 0; j < in_channels; ++j) {
      uintptr_t tmp_output_addr = output_fram_addr;
      input_channel_addr = input_fram_addr;
      uintptr_t tmp_input_addr = input_channel_addr;
      uintptr_t strided_input_addr = tmp_input_addr;

      /* send kernel to LEA RAM */
      DMA_makeTransfer(flt_fram_addr, flt_lea_addr, flt_len);
      flt_fram_addr += flt_addr_offset;

/*************************************************************
* Do the rest of FIR
************************************************************/
      
      conv_params.length = _FIR_OUTPUT_SIZE_ALIGNED;
      
      lea_src[_FIR_INPUT_SIZE] = 0;
      lea_src[_FIR_INPUT_SIZE + 1] = 0;

      for (uint16_t n = _FIR_INPUT_REMAIN_SIZE; n < _FIR_TOTAL_SIZE; n += _FIR_INPUT_SIZE) {
        conv_flt = lea_flt;
        conv_params.coeffs = conv_flt;
        strided_input_addr = tmp_input_addr;

        /*
        * Do the FIR on the first row and save the results in lea_dst.
        * For the rest rows, save the FIR results in lea_tmp and add them to lea_dst.
        */
        deinterleave_params.length = _DEINTERLEAVE_SIZE_ALIGNED;
        add_params.length = _DEINTERLEAVE_SIZE_ALIGNED;

        /* send input to LEA RAM */
        input_lea_addr = src_lea_addr;
        for (uint16_t r = 0; r < _FIR_INPUT_SIZE; r += _INPUT_COL_SIZE) {
          DMA_makeTransfer(strided_input_addr, input_lea_addr, _INPUT_COL_SIZE);
          
          input_lea_addr += input_line_size_offset;
          strided_input_addr += input_line_strided_size_offset;
        }

        /* convolution */
        msp_fir_q15(&conv_params, lea_src, lea_tmp);

        /*
        * stride
        * Use lea deinterleave and store it in the lea_src
        */
        msp_deinterleave_q15(&deinterleave_params, lea_tmp, lea_dst);
        conv_flt += conv_params.tapLength;
        tmp_input_addr += input_line_size_offset;

        for (uint16_t k = 1; k < kernel_row_size; ++k) {
          strided_input_addr = tmp_input_addr;
          conv_params.coeffs = conv_flt;

          /* send input to LEA RAM */
          input_lea_addr = src_lea_addr;
          for (uint16_t r = 0; r < _FIR_INPUT_SIZE; r += _INPUT_COL_SIZE) {
            DMA_makeTransfer(strided_input_addr, input_lea_addr, _INPUT_COL_SIZE);
            
            input_lea_addr += input_line_size_offset;
            strided_input_addr += input_line_strided_size_offset;
          }

          /* convolution */
          msp_fir_q15(&conv_params, lea_src, lea_tmp);
          /*
          * stride
          * Use lea deinterleave and store it in the lea_src
          */
          
          msp_deinterleave_q15(&deinterleave_params, lea_tmp, lea_src);

          /* accumulate results for a 2D convolution */
          msp_add_q15(&add_params, lea_dst, lea_src, lea_dst);
          conv_flt += conv_params.tapLength;
          tmp_input_addr += input_line_size_offset;
        }

        /* accumulate results with previous outputs */
        lea_skip_addr_before = lea_skip_addr_before_st;
        lea_skip_addr_after = lea_skip_addr_after_st;
        /* skip the garbage values between two lines */
        for (uint16_t l = _OUTPUT_COL_SIZE; l < _FIR_ADD_OUTPUT_SIZE; l += _OUTPUT_COL_SIZE) {

          DMA_makeTransfer(lea_skip_addr_before, lea_skip_addr_after, _OUTPUT_COL_SIZE);
          lea_skip_addr_before += lea_skip_addr_before_offset;
          lea_skip_addr_after += lea_skip_addr_after_offset;
        }

        /* send output to LEA RAM */
        DMA_makeTransfer(tmp_output_addr, tmp_lea_addr, _FIR_ADD_OUTPUT_SIZE);
        add_params.length = _FIR_ADD_OUTPUT_SIZE_ALIGNED;
        msp_add_q15(&add_params, lea_dst, lea_tmp, lea_dst);

        /* bring back output from LEA RAM */
        DMA_makeTransfer(dst_lea_addr, tmp_output_addr, _FIR_ADD_OUTPUT_SIZE);
        input_channel_addr += input_offset;
        tmp_input_addr = input_channel_addr;
        tmp_output_addr += output_offset;
      }
      input_fram_addr += input_channel_offset;
    }
    output_fram_addr += output_addr_offset;
  }

  /* add bias and left shift */
  output_fram_addr = (uintptr_t)(output->data);

  _q15* lea_add = lea_src;
  uint16_t add_size = _LEA_ADD_SIZE;
  uintptr_t lea_add_addr = (uintptr_t)lea_add;
  for (uint16_t i = 0; i < out_channels; ++i) {
    offset_params.offset = bias->data[i];
    offset_params.length = lea_remain_size_aligned;
    add_params.length = lea_remain_size_aligned;

    DMA_makeTransfer(output_fram_addr, lea_add_addr, lea_remain_size);
    
    msp_add_q15(&add_params, lea_add, lea_add, lea_add);
    msp_offset_q15(&offset_params, lea_add, lea_add);

    DMA_makeTransfer(lea_add_addr, output_fram_addr, lea_remain_size);
    output_fram_addr += output_remain_size_offset;
    offset_params.length = add_size;
    add_params.length = add_size;

    for (uint16_t j = lea_remain_size; j < output_len; j += add_size) {
      DMA_makeTransfer(output_fram_addr, lea_add_addr, add_size);
      
      msp_add_q15(&add_params, lea_add, lea_add, lea_add);
      msp_offset_q15(&offset_params, lea_add, lea_add);

      DMA_makeTransfer(lea_add_addr, output_fram_addr, add_size);
      output_fram_addr += output_lea_min_size_offset;
    }
  }
}