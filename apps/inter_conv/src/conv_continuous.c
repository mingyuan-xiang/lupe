#include <include/utils.h>
#include <include/conv_continuous.h>


#define _LEA_ADD_SIZE 400

#define _OUTPUT_SIZE 784
#define _LEA_REMAIN_SIZE (_OUTPUT_SIZE % _LEA_ADD_SIZE)
#define _PADDING_TOP 2
#define _PADDING_BOTTOM 2
#define _PADDING_RIGHT 2
#define _PADDING_LEFT 2

#define _INPUT_LINE_SIZE 32

#define _KERNEL_SIZE_ALIGNED 6
#define _KERNEL_ROW_SIZE 5
#define _KERNEL_COL_SIZE 5


void conv_continuous(mat_t* input, mat_t* output, mat_t* weight) {
  uint16_t in_channels = input->dims[1];
  uint16_t out_channels = output->dims[1];
  uint16_t output_len = output->strides[1];
  uint16_t input_len = input->strides[1];
  uint16_t kernel_row_size = _KERNEL_ROW_SIZE;
  uint16_t kernel_col_size = _KERNEL_COL_SIZE;

  uintptr_t src_lea_addr = (uintptr_t)lea_src;
  uintptr_t dst_lea_addr = (uintptr_t)lea_dst;

  uintptr_t flt_lea_addr = (uintptr_t)lea_flt;
  uintptr_t flt_fram_addr = (uintptr_t)(weight->data);
  uintptr_t flt_addr_col_offset = kernel_col_size * sizeof(int16_t);
  uintptr_t flt_addr_padding_offset = (kernel_col_size + 1) * sizeof(int16_t);
  uintptr_t input_lea_addr = src_lea_addr;
  uintptr_t input_fram_addr = (uintptr_t)(input->data);
  uintptr_t input_channel_addr = input_fram_addr;
  uintptr_t input_channel_offset = input_len * sizeof(int16_t);
  uintptr_t output_lea_addr = dst_lea_addr;
  uintptr_t output_fram_addr = (uintptr_t)(output->data);
  uintptr_t output_addr_offset = output_len * sizeof(int16_t);

  int16_t* conv_flt = lea_flt;
  uint16_t input_line_size = input->dims[3];
  uint16_t output_line_size = output->dims[3];
  uint16_t output_line_num = output->dims[2];
  uint16_t output_line_size_offset = output_line_size * sizeof(int16_t);
  uint16_t input_line_size_offset = input_line_size * sizeof(int16_t);

  uint16_t lea_remain_size = _LEA_REMAIN_SIZE;
  uintptr_t output_remain_size_offset = lea_remain_size * sizeof(int16_t);
  uintptr_t output_lea_min_size_offset = _LEA_ADD_SIZE * sizeof(int16_t);
  uint16_t lea_remain_size_aligned = MAKE_ALIGN_2(lea_remain_size);
  

  
  msp_fir_q15_params conv_params = {
    .length = MAKE_ALIGN_2(input_line_size - kernel_col_size + 1),
    .tapLength = MAKE_ALIGN_2(kernel_col_size),
    .coeffs = lea_flt,
    .enableCircularBuffer = false 
  };
  msp_add_q15_params add_params = { .length = MAKE_ALIGN_2(output_line_size) };
  msp_offset_q15_params offset_params = { .length = lea_remain_size_aligned, .offset = 0 };

  
  /* set the aligned position to be zeros */
  uint16_t lea_reset_pos = 0;
  for (uint16_t k = 0; k < kernel_row_size; ++k) {
    lea_flt[lea_reset_pos] = 0;
    lea_reset_pos += (kernel_col_size + 1);
  }

  lea_src[_INPUT_LINE_SIZE] = 0;
  lea_src[_INPUT_LINE_SIZE + 1] = 0;

  memset(output->data, 0, GET_MAT_SIZE(output)*sizeof(int16_t));

  /* convolution */
  for (uint16_t i = 0; i < out_channels; ++i) {
    input_fram_addr = (uintptr_t)(input->data);
    for (uint16_t j = 0; j < in_channels; ++j) {
      uintptr_t tmp_output_addr = output_fram_addr;
      input_channel_addr = input_fram_addr;

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

      for (uint16_t l = 0; l < output_line_num; ++l) {
        uintptr_t tmp_input_addr = input_channel_addr;
        /* send output to LEA RAM */
        DMA_makeTransfer(tmp_output_addr, output_lea_addr, output_line_size);

        conv_flt = lea_flt;

        for (uint16_t k = 0; k < kernel_row_size; ++k) {
          /* send input to LEA RAM */
          DMA_makeTransfer(tmp_input_addr, input_lea_addr, input_line_size);
          conv_params.coeffs = conv_flt;

          /* convolution */
          msp_fir_q15(&conv_params, lea_src, lea_tmp);

          /* accumulate results for a 2D convolution */
          msp_add_q15(&add_params, lea_dst, lea_tmp, lea_dst);
          conv_flt += conv_params.tapLength;
          tmp_input_addr += input_line_size_offset;
        }

        /* bring back output from LEA RAM */
        DMA_makeTransfer(output_lea_addr, tmp_output_addr, output_line_size);
        tmp_output_addr += output_line_size_offset;
        input_channel_addr += input_line_size_offset;
      }
      input_fram_addr += input_channel_offset;
    }
    output_fram_addr += output_addr_offset;
  }
}