#include <include/utils.h>
#include <include/conv.h>
#include <include/intermittent.h>

#define _LEA_ADD_SIZE 100

#define _LEA_REMAIN_SIZE (100 % _LEA_ADD_SIZE)

#define _STRIDE_ROW_SIZE 1
#define _STRIDE_COL_SIZE 1

#define _INPUT_ROW_SIZE 14
#define _INPUT_COL_SIZE 14
#define _KERNEL_ROW_SIZE 5
#define _KERNEL_COL_SIZE 5
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
  uintptr_t flt_addr_col_offset = kernel_col_size * sizeof(int16_t);
  uintptr_t flt_addr_padding_offset = (kernel_col_size + 1) * sizeof(int16_t);
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
  uintptr_t input_remain_offset = _FIR_INPUT_REMAIN_SIZE * _STRIDE_ROW_SIZE * sizeof(int16_t);
  uintptr_t output_remain_offset = _FIR_ADD_OUTPUT_REMAIN_SIZE * sizeof(int16_t);
  uintptr_t input_offset = _FIR_INPUT_SIZE * _STRIDE_ROW_SIZE * sizeof(int16_t);
  uintptr_t output_offset = _FIR_ADD_OUTPUT_SIZE * sizeof(int16_t);

  int16_t* conv_flt = lea_flt;
  uint16_t input_line_size = input->dims[3];
  uint16_t input_line_size_offset = input_line_size * sizeof(int16_t);
  

  uint16_t lea_remain_size = _LEA_REMAIN_SIZE;
  uintptr_t output_lea_min_size_offset = _LEA_ADD_SIZE * sizeof(int16_t);
  uint16_t lea_remain_size_aligned = MAKE_ALIGN_2(lea_remain_size);
  
  
  msp_fir_q15_params conv_params = {
    .length = _FIR_OUTPUT_REMAIN_SIZE_ALIGNED,
    .tapLength = MAKE_ALIGN_2(kernel_col_size),
    .coeffs = lea_flt,
    .enableCircularBuffer = false 
  };
  msp_add_q15_params add_params = { .length = _FIR_OUTPUT_REMAIN_SIZE_ALIGNED };
  msp_offset_q15_params offset_params = { .length = lea_remain_size_aligned, .offset = 0 };

  
  /* set the aligned position to be zeros */
  uint16_t lea_reset_pos = 0;
  for (uint16_t k = 0; k < kernel_row_size; ++k) {
    lea_flt[lea_reset_pos] = 0;
    lea_reset_pos += (kernel_col_size + 1);
  }
  memset(output->data, 0, GET_MAT_SIZE(output)*sizeof(int16_t));

  /* convolution */
  for (uint16_t i = 0; i < out_channels; ++i) {
    input_fram_addr = (uintptr_t)(input->data);
    for (uint16_t j = 0; j < in_channels; ++j) {
      uintptr_t tmp_output_addr = output_fram_addr;
      input_channel_addr = input_fram_addr;
      uintptr_t tmp_input_addr = input_channel_addr;

      /* send kernel to LEA RAM */
      /*
      * pad zero to the beginning of the filter if the filter's size is
      * not aligned to 2
      */
      flt_lea_addr += sizeof(uint16_t);

      for (uint16_t k = 0; k < kernel_row_size; ++k) {
        DMA_makeTransfer(flt_fram_addr, flt_lea_addr, kernel_col_size);
        flt_lea_addr += flt_addr_padding_offset;
        flt_fram_addr += flt_addr_col_offset;
      }
      /* restore flt_lea_addr pointer to the beginning of the array */
      flt_lea_addr = (uintptr_t)lea_flt;

/*************************************************************
* Do the reminder of FIR first
************************************************************/
      conv_flt = lea_flt;
      conv_params.coeffs = conv_flt;
      conv_params.length = _FIR_OUTPUT_REMAIN_SIZE_ALIGNED;
      add_params.length = _FIR_OUTPUT_REMAIN_SIZE_ALIGNED;
      
      lea_src[_FIR_INPUT_REMAIN_SIZE] = 0;
      lea_src[_FIR_INPUT_REMAIN_SIZE + 1] = 0;

      /*
       * Do the FIR on the first row and save the results in lea_dst.
       * For the rest rows, save the FIR results in lea_tmp and add them to lea_dst.
       */

      /* send input to LEA RAM */
      DMA_makeTransfer(tmp_input_addr, input_lea_addr, _FIR_INPUT_REMAIN_SIZE);

      /* convolution */
      msp_fir_q15(&conv_params, lea_src, lea_dst);
      conv_flt += conv_params.tapLength;
      tmp_input_addr += input_line_size_offset;

      for (uint16_t k = 1; k < kernel_row_size; ++k) {
        conv_params.coeffs = conv_flt;

        /* send input to LEA RAM */
        DMA_makeTransfer(tmp_input_addr, input_lea_addr, _FIR_INPUT_REMAIN_SIZE);

        /* convolution */
        msp_fir_q15(&conv_params, lea_src, lea_tmp);

        /* accumulate results for a 2D convolution */
        msp_add_q15(&add_params, lea_dst, lea_tmp, lea_dst);
        conv_flt += conv_params.tapLength;
        tmp_input_addr += input_line_size_offset;
      }

      /* accumulate results with previous outputs */
      lea_skip_addr_before = lea_skip_addr_before_st;
      lea_skip_addr_after = lea_skip_addr_after_st;
      /* skip the garbage values between two lines */
      for (uint16_t l = _OUTPUT_COL_SIZE; l < _FIR_ADD_OUTPUT_REMAIN_SIZE; l += _OUTPUT_COL_SIZE) {

        DMA_makeTransfer(lea_skip_addr_before, lea_skip_addr_after, _OUTPUT_COL_SIZE);
        lea_skip_addr_before += lea_skip_addr_before_offset;
        lea_skip_addr_after += lea_skip_addr_after_offset;
      }

      /* send output to LEA RAM */
      DMA_makeTransfer(tmp_output_addr, tmp_lea_addr, _FIR_ADD_OUTPUT_REMAIN_SIZE);
      add_params.length = _FIR_ADD_OUTPUT_REMAIN_SIZE_ALIGNED;
      msp_add_q15(&add_params, lea_dst, lea_tmp, lea_dst);

      /* bring back output from LEA RAM */
      DMA_makeTransfer(dst_lea_addr, tmp_output_addr, _FIR_ADD_OUTPUT_REMAIN_SIZE);
      input_channel_addr += input_remain_offset;
      tmp_input_addr = input_channel_addr;
      tmp_output_addr += output_remain_offset;

/*************************************************************
* Do the rest of FIR
************************************************************/
      
      conv_params.length = _FIR_OUTPUT_SIZE_ALIGNED;
      
      lea_src[_FIR_INPUT_SIZE] = 0;
      lea_src[_FIR_INPUT_SIZE + 1] = 0;

      for (uint16_t n = _FIR_INPUT_REMAIN_SIZE; n < _FIR_TOTAL_SIZE; n += _FIR_INPUT_SIZE) {
        conv_flt = lea_flt;
        conv_params.coeffs = conv_flt;

        /*
        * Do the FIR on the first row and save the results in lea_dst.
        * For the rest rows, save the FIR results in lea_tmp and add them to lea_dst.
        */
        add_params.length = _FIR_OUTPUT_SIZE_ALIGNED;

        /* send input to LEA RAM */
        DMA_makeTransfer(tmp_input_addr, input_lea_addr, _FIR_INPUT_SIZE);

        /* convolution */
        msp_fir_q15(&conv_params, lea_src, lea_dst);
        conv_flt += conv_params.tapLength;
        tmp_input_addr += input_line_size_offset;

        for (uint16_t k = 1; k < kernel_row_size; ++k) {
          conv_params.coeffs = conv_flt;

          /* send input to LEA RAM */
          DMA_makeTransfer(tmp_input_addr, input_lea_addr, _FIR_INPUT_SIZE);

          /* convolution */
          msp_fir_q15(&conv_params, lea_src, lea_tmp);

          /* accumulate results for a 2D convolution */
          msp_add_q15(&add_params, lea_dst, lea_tmp, lea_dst);
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
}