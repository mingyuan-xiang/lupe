#include <include/utils.h>
#include <include/conv.h>
#include <include/intermittent.h>

#define _LEA_ADD_SIZE 1600
#define _LEA_REMAIN_SIZE (126 % _LEA_ADD_SIZE)

/* assign the lea buffer pointer */
#define lea_src (lea_buffer)
#define lea_flt (lea_buffer + 512)
#define lea_tmp (lea_buffer + 528)
#define lea_dst (lea_buffer + 1040)

#define _STRIDE_ROW_SIZE 1
#define _STRIDE_COL_SIZE 1

#define _INPUT_ROW_SIZE 21
#define _INPUT_COL_SIZE 6
#define _KERNEL_ROW_SIZE 1
#define _KERNEL_COL_SIZE 1
#define _OUTPUT_ROW_SIZE (((_INPUT_ROW_SIZE - _KERNEL_ROW_SIZE) / _STRIDE_ROW_SIZE) + 1)
#define _OUTPUT_COL_SIZE (((_INPUT_COL_SIZE - _KERNEL_COL_SIZE) / _STRIDE_COL_SIZE) + 1)
#define _KERNEL_SIZE_ALIGNED MAKE_ALIGN_2(_KERNEL_COL_SIZE)

#define _FIR_OVERLAP_SIZE ((_KERNEL_SIZE_ALIGNED - (_KERNEL_SIZE_ALIGNED - _KERNEL_COL_SIZE) - 1) / _STRIDE_COL_SIZE)


#define __LEA_SRC_SIZE 512
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


static inline __attribute__((always_inline)) void new_add_q15(const _q15* srcA, const _q15* srcB, _q15* dst) {
  lea_add_params->input2 = MSP_LEA_CONVERT_ADDRESS(srcB);
  lea_add_params->output = MSP_LEA_CONVERT_ADDRESS(dst);

  /* Load source arguments to LEA. */
  LEAPMS0 = MSP_LEA_CONVERT_ADDRESS(srcA);
  LEAPMS1 = MSP_LEA_CONVERT_ADDRESS(lea_add_params);

  /* Invoke the LEACMD__ADDMATRIX command. */
  msp_lea_invokeCommand(LEACMD__ADDMATRIX);
}

static inline __attribute__((always_inline)) void new_fir_q15(_q15* coeffs, const _q15* src, _q15* dst) {
  /* Set MSP_LEA_FIR_PARAMS structure. */
  lea_fir_params->coeffs = MSP_LEA_CONVERT_ADDRESS(coeffs);
  lea_fir_params->output = MSP_LEA_CONVERT_ADDRESS(dst);

  /* Load source arguments to LEA. */
  LEAPMS0 = MSP_LEA_CONVERT_ADDRESS(src);
  LEAPMS1 = MSP_LEA_CONVERT_ADDRESS(lea_fir_params);

  /* Invoke the LEACMD__FIR command. */
  msp_lea_invokeCommand(LEACMD__FIR);
}

static inline __attribute__((always_inline)) void set_offset(int16_t offset) {
  offset_vector[0] = offset;
  offset_vector[1] = offset;
}

static inline __attribute__((always_inline)) void new_offset_q15(const _q15* src, _q15* dst) {
  lea_offset_params->output = MSP_LEA_CONVERT_ADDRESS(dst);

  /* Load source arguments to LEA. */
  LEAPMS0 = MSP_LEA_CONVERT_ADDRESS(src);
  LEAPMS1 = MSP_LEA_CONVERT_ADDRESS(lea_offset_params);

  /* Invoke the LEACMD__ADDMATRIX command. */
  msp_lea_invokeCommand(LEACMD__ADDMATRIX);
}


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
  
  uint16_t lea_skip_pos_before_st = _OUTPUT_COL_SIZE + _FIR_OVERLAP_SIZE;
  uint16_t lea_skip_pos_before = lea_skip_pos_before_st;
  uint16_t lea_skip_pos_after_st = _OUTPUT_COL_SIZE;
  uint16_t lea_skip_pos_after = lea_skip_pos_after_st;
  uintptr_t input_remain_offset = _FIR_INPUT_REMAIN_SIZE * _STRIDE_ROW_SIZE * sizeof(int16_t);
  uintptr_t output_remain_offset = _FIR_ADD_OUTPUT_REMAIN_SIZE * sizeof(int16_t);

  int16_t* conv_flt = lea_flt;
  uint16_t input_line_size = input->dims[3];
  uint16_t input_line_size_offset = input_line_size * sizeof(int16_t);
  

  uint16_t lea_remain_size = _LEA_REMAIN_SIZE;
  uintptr_t output_remain_size_offset = lea_remain_size * sizeof(int16_t);
  uint16_t lea_remain_size_aligned = MAKE_ALIGN_2(lea_remain_size);
  
  
  uint16_t tapLength = MAKE_ALIGN_2(kernel_col_size);
  add_init(MAKE_ALIGN_2(_FIR_OUTPUT_REMAIN_SIZE_ALIGNED));
  fir_init(_FIR_OUTPUT_REMAIN_SIZE_ALIGNED, tapLength);
  offset_init(lea_remain_size_aligned);
  uint16_t flt_lea_pos = 0;
  uint16_t flt_fram_pos = 0;
  uint16_t zero = 0;

  
  /* set the aligned position to be zeros */
  uint16_t lea_reset_pos = 0;
  for (uint16_t k = 0; k < kernel_row_size; ++k) {
    lea_flt[lea_reset_pos] = 0;
    lea_reset_pos += (kernel_col_size + 1);
  }
  DMA_setWord(output_fram_addr, zero, GET_MAT_SIZE(output));

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
      ++flt_lea_pos;

      for (uint16_t k = 0; k < kernel_row_size; ++k) {
        lea_flt[flt_lea_pos] = weight->data[flt_fram_pos];
        flt_lea_pos++;
        flt_fram_pos++;

        flt_lea_pos++;
      }
      /* restore flt_lea_addr pointer to the beginning of the array */
      flt_lea_pos = 0;

/*************************************************************
* Do the reminder of FIR first
************************************************************/
      conv_flt = lea_flt;
      lea_fir_params->vectorSize = _FIR_OUTPUT_REMAIN_SIZE_ALIGNED;
      lea_add_params->vectorSize = _FIR_OUTPUT_REMAIN_SIZE_ALIGNED;
      
      lea_src[_FIR_INPUT_REMAIN_SIZE] = 0;
      lea_src[_FIR_INPUT_REMAIN_SIZE + 1] = 0;

      /*
       * Do the FIR on the first row and save the results in lea_dst.
       * For the rest rows, save the FIR results in lea_tmp and add them to lea_dst.
       */

      /* send input to LEA RAM */
      DMA_makeTransfer(tmp_input_addr, input_lea_addr, _FIR_INPUT_REMAIN_SIZE);

      /* convolution */
      new_fir_q15(conv_flt, lea_src, lea_dst);
      conv_flt += tapLength;
      tmp_input_addr += input_line_size_offset;

      for (uint16_t k = 1; k < kernel_row_size; ++k) {

        /* send input to LEA RAM */
        DMA_makeTransfer(tmp_input_addr, input_lea_addr, _FIR_INPUT_REMAIN_SIZE);

        /* convolution */
        new_fir_q15(conv_flt, lea_src, lea_tmp);

        /* accumulate results for a 2D convolution */
        new_add_q15(lea_dst, lea_tmp, lea_dst);
        conv_flt += tapLength;
        tmp_input_addr += input_line_size_offset;
      }

      /* accumulate results with previous outputs */
      lea_skip_pos_before = lea_skip_pos_before_st;
      lea_skip_pos_after = lea_skip_pos_after_st;
      /* skip the garbage values between two lines */
      for (uint16_t l = _OUTPUT_COL_SIZE; l < _FIR_ADD_OUTPUT_REMAIN_SIZE; l += _OUTPUT_COL_SIZE) {
        lea_dst[lea_skip_pos_after] = lea_dst[lea_skip_pos_before];
        lea_skip_pos_after++;
        lea_skip_pos_before++;
        lea_dst[lea_skip_pos_after] = lea_dst[lea_skip_pos_before];
        lea_skip_pos_after++;
        lea_skip_pos_before++;
        lea_dst[lea_skip_pos_after] = lea_dst[lea_skip_pos_before];
        lea_skip_pos_after++;
        lea_skip_pos_before++;
        lea_dst[lea_skip_pos_after] = lea_dst[lea_skip_pos_before];
        lea_skip_pos_after++;
        lea_skip_pos_before++;
        lea_dst[lea_skip_pos_after] = lea_dst[lea_skip_pos_before];
        lea_skip_pos_after++;
        lea_skip_pos_before++;
        lea_dst[lea_skip_pos_after] = lea_dst[lea_skip_pos_before];
        lea_skip_pos_after++;
        lea_skip_pos_before++;

        lea_skip_pos_before += _FIR_OVERLAP_SIZE;
      }

      /* send output to LEA RAM */
      DMA_makeTransfer(tmp_output_addr, tmp_lea_addr, _FIR_ADD_OUTPUT_REMAIN_SIZE);
      lea_add_params->vectorSize = _FIR_ADD_OUTPUT_REMAIN_SIZE_ALIGNED;
      new_add_q15(lea_dst, lea_tmp, lea_dst);

      /* bring back output from LEA RAM */
      DMA_makeTransfer(dst_lea_addr, tmp_output_addr, _FIR_ADD_OUTPUT_REMAIN_SIZE);
      input_channel_addr += input_remain_offset;
      tmp_input_addr = input_channel_addr;
      tmp_output_addr += output_remain_offset;
      input_fram_addr += input_channel_offset;
    }
    output_fram_addr += output_addr_offset;
  }

  /* add bias and left shift */
  output_fram_addr = (uintptr_t)(output->data);

  _q15* lea_add = lea_src;
  uintptr_t lea_add_addr = (uintptr_t)lea_add;
  for (uint16_t i = 0; i < out_channels; ++i) {
    set_offset(bias->data[i]);
    lea_offset_params->vectorSize = lea_remain_size_aligned;
    lea_add_params->vectorSize = lea_remain_size_aligned;

    DMA_makeTransfer(output_fram_addr, lea_add_addr, lea_remain_size);
    
    new_add_q15(lea_add, lea_add, lea_add);
    new_offset_q15(lea_add, lea_add);

    DMA_makeTransfer(lea_add_addr, output_fram_addr, lea_remain_size);
    output_fram_addr += output_remain_size_offset;
  }
  offset_clear();
  fir_clear();
  add_clear();
}