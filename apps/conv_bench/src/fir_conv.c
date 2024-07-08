#include <include/utils.h>
#include <include/fir_conv.h>

#define _LEA_DST_SIZE 1408
#define _LEA_REMAIN_SIZE 1024 % _LEA_DST_SIZE
#define _PADDING_TOP 1
#define _PADDING_BOTTOM 1
#define _PADDING_RIGHT 1
#define _PADDING_LEFT 1

/* assign the lea buffer pointer */
#define lea_src (lea_buffer)
#define lea_flt (lea_buffer + 64)
#define lea_tmp (lea_buffer + 128)
#define lea_dst (lea_buffer + 192)

#define _INPUT_LINE_SIZE 34

#define _FLT_SIZE_ALIGNED 4

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

void fir_conv(mat_t* input, mat_t* output, mat_t* weight, mat_t* bias) {
  uint16_t in_channels = input->dims[1];
  uint16_t out_channels = output->dims[1];
  uint16_t output_len = output->strides[1];
  uint16_t input_len = input->strides[1];
  uint16_t kernel_size = weight->dims[2];

  uint32_t src_lea_addr = (uint32_t)lea_src;
  uint32_t dst_lea_addr = (uint32_t)lea_dst;

  uint32_t flt_lea_addr = (uint32_t)lea_flt;
  uint32_t flt_fram_addr = (uint32_t)(weight->data);
  uint32_t flt_addr_row_offset = kernel_size * sizeof(int16_t);
  uint32_t flt_addr_padding_offset = (kernel_size + 1) * sizeof(int16_t);
  uint32_t input_lea_addr = src_lea_addr;
  uint32_t input_fram_addr = (uint32_t)(input->data);
  uint32_t input_channel_addr = input_fram_addr;
  uint32_t input_channel_offset = input_len * sizeof(int16_t);
  uint32_t output_lea_addr = dst_lea_addr;
  uint32_t output_fram_addr = (uint32_t)(output->data);
  uint32_t output_addr_offset = output_len * sizeof(int16_t);

  int16_t* conv_flt = lea_flt;
  uint16_t input_line_size = input->dims[3];
  uint16_t output_line_size = output->dims[3];
  uint16_t output_line_num = output->dims[2];
  uint16_t output_line_size_offset = output_line_size * sizeof(int16_t);
  uint16_t input_line_size_offset = input_line_size * sizeof(int16_t);

  uint16_t lea_remain_size = _LEA_REMAIN_SIZE;
  uint32_t output_remain_size_offset = lea_remain_size * sizeof(int16_t);
  uint16_t lea_remain_size_aligned = MAKE_ALIGN_2(lea_remain_size);
  
  
  uint16_t tapLength = MAKE_ALIGN_2(kernel_size);
  add_init(MAKE_ALIGN_2(output_line_size));
  fir_init(MAKE_ALIGN_2(input_line_size - kernel_size + 1), tapLength);
  offset_init(lea_remain_size_aligned);
  uint16_t dst_pos;
  uint16_t s;
  uint16_t flt_lea_pos = 0;
  uint16_t flt_fram_pos = 0;
  uint16_t zero = 0;

  
  /* set the aligned position to be zeros */
  uint16_t lea_reset_pos = 0;
  for (uint16_t k = 0; k < kernel_size; ++k) {
    lea_flt[lea_reset_pos] = 0;
    lea_reset_pos += (kernel_size + 1);
  }

  lea_src[_INPUT_LINE_SIZE] = 0;
  lea_src[_INPUT_LINE_SIZE + 1] = 0;

  /* Pad the input for (1, 1, 1, 1) */
  DMA_setWord(output_fram_addr, zero, GET_MAT_SIZE(input));
  uint16_t input_line_num = input->dims[2] - _PADDING_TOP - _PADDING_BOTTOM;
  uint16_t input_line_size_bf = input->dims[3] - _PADDING_RIGHT - _PADDING_LEFT;
  _q15* padding_ptr_in = input->data;
  _q15* padding_ptr_out = output->data;
  for (uint16_t i = 0; i < in_channels; ++i) {
    padding_ptr_out += input_line_size;
    for (uint16_t j = 0; j < input_line_num; ++j) {
      padding_ptr_out += _PADDING_LEFT;
      DMA_makeTransfer((uint32_t)padding_ptr_in, (uint32_t)padding_ptr_out, input_line_size_bf);
      padding_ptr_in += input_line_size_bf;
      padding_ptr_out += (_PADDING_RIGHT + input_line_size_bf);
    }
    padding_ptr_out += input_line_size;
  }

  DMA_makeTransfer(output_fram_addr, input_fram_addr, GET_MAT_SIZE(input));
  DMA_setWord(output_fram_addr, zero, GET_MAT_SIZE(output));

  /* convolution */
  for (uint16_t i = 0; i < out_channels; ++i) {
    input_fram_addr = (uint32_t)(input->data);
    for (uint16_t j = 0; j < in_channels; ++j) {
      uint32_t tmp_output_addr = output_fram_addr;
      input_channel_addr = input_fram_addr;

      /* send kernel to LEA RAM */
      /*
      * pad zero to the beginning of the filter if the filter's size is
      * not aligned to 2
      */
      ++flt_lea_pos;

      for (uint16_t k = 0; k < kernel_size; ++k) {
        s = flt_fram_pos;
        dst_pos = flt_lea_pos;
        lea_flt[dst_pos] = weight->data[s];
        dst_pos++;
        s++;
        lea_flt[dst_pos] = weight->data[s];
        dst_pos++;
        s++;
        lea_flt[dst_pos] = weight->data[s];
        dst_pos++;
        s++;

        flt_lea_pos += _FLT_SIZE_ALIGNED;
        flt_fram_pos += kernel_size;
      }
      /* restore flt_lea_addr pointer to the beginning of the array */
      flt_lea_pos = 0;

      for (uint16_t l = 0; l < output_line_num; ++l) {
        uint32_t tmp_input_addr = input_channel_addr;
        /* send output to LEA RAM */
        DMA_makeTransfer(tmp_output_addr, output_lea_addr, output_line_size);

        conv_flt = lea_flt;

        for (uint16_t k = 0; k < kernel_size; ++k) {
          /* send input to LEA RAM */
          DMA_makeTransfer(tmp_input_addr, input_lea_addr, input_line_size);

          /* convolution */
          new_fir_q15(conv_flt, lea_src, lea_tmp);

          /* accumulate results for a 2D convolution */
          new_add_q15(lea_dst, lea_tmp, lea_dst);
          conv_flt += tapLength;
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

  /* add bias and left shift */
  output_fram_addr = (uint32_t)(output->data);

  _q15* lea_add = lea_dst;
  uint32_t lea_add_addr = (uint32_t)lea_add;
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