#include <layers/include/utils.h>
#include <layers/include/conv1.h>
#include <buffer/include/buffer.h>


static inline void new_add_q15(const _q15* srcA, const _q15* srcB, _q15* dst) {
  lea_add_params->input2 = MSP_LEA_CONVERT_ADDRESS(srcB);
  lea_add_params->output = MSP_LEA_CONVERT_ADDRESS(dst);

  /* Load source arguments to LEA. */
  LEAPMS0 = MSP_LEA_CONVERT_ADDRESS(srcA);
  LEAPMS1 = MSP_LEA_CONVERT_ADDRESS(lea_add_params);

  /* Invoke the LEACMD__ADDMATRIX command. */
  msp_lea_invokeCommand(LEACMD__ADDMATRIX);
}

static inline void new_fir_q15(_q15* coeffs, const _q15* src, _q15* dst) {
  /* Set MSP_LEA_FIR_PARAMS structure. */
  lea_fir_params->coeffs = MSP_LEA_CONVERT_ADDRESS(coeffs);
  lea_fir_params->output = MSP_LEA_CONVERT_ADDRESS(dst);

  /* Load source arguments to LEA. */
  LEAPMS0 = MSP_LEA_CONVERT_ADDRESS(src);
  LEAPMS1 = MSP_LEA_CONVERT_ADDRESS(lea_fir_params);

  /* Invoke the LEACMD__FIR command. */
  msp_lea_invokeCommand(LEACMD__FIR);
}

void conv1(mat_t* input, mat_t* output, mat_t* weight, mat_t* bias) {
  uint16_t in_channels = input->dims[1];
  uint16_t out_channels = output->dims[1];
  uint16_t output_len = output->strides[1];
  uint16_t input_len = input->strides[1];
  uint16_t kernel_size = weight->dims[2];

  uint32_t flt_lea_addr = (uint32_t)lea_flt;
  uint32_t flt_fram_addr = (uint32_t)(weight->data);
  uint32_t flt_addr_row_offset = kernel_size * sizeof(int16_t);
  uint32_t flt_addr_padding_offset = (kernel_size + 1) * sizeof(int16_t);
  uint32_t input_lea_addr = (uint32_t)lea_src;
  uint32_t input_fram_addr = (uint32_t)(input->data);
  uint32_t input_channel_addr = input_fram_addr;
  uint32_t input_channel_offset = input_len * sizeof(int16_t);
  uint32_t output_lea_addr = (uint32_t)lea_dst;
  uint32_t output_fram_addr = (uint32_t)(output->data);
  uint32_t output_addr_offset = output_len * sizeof(int16_t);

  int16_t* conv_flt = lea_flt;
  uint16_t input_line_size = input->dims[3];
  uint16_t output_line_size = output->dims[3];
  uint16_t output_line_num = output->dims[2];
  uint16_t output_line_size_offset = output_line_size * sizeof(int16_t);
  uint16_t input_line_size_offset = input_line_size * sizeof(int16_t);
  
  uint16_t tapLength = MAKE_ALIGN_2(kernel_size);
  add_init(MAKE_ALIGN_2(output_line_size));
  fir_init(MAKE_ALIGN_2(output_line_size), tapLength);
  msp_fill_q15_params fill_params = { .length = LEA_SRC_SIZE, .value = 0 };

  /* reset LEA's RAM */
  msp_fill_q15(&fill_params, lea_src);

  fill_params.length = LEA_FLT_SIZE;
  msp_fill_q15(&fill_params, lea_flt);

  fill_params.length = LEA_DST_SIZE;
  msp_fill_q15(&fill_params, lea_dst);

  fill_params.length = LEA_DST_SIZE;
  msp_fill_q15(&fill_params, lea_tmp);

  /* Pad the input for (2, 2, 2, 2) */
  uint16_t input_line_num = input->dims[2] - 4;
  uint16_t input_line_size_bf = input->dims[3] - 4;
  _q15* padding_ptr_in = input->data;
  _q15* padding_ptr_out = output->data;
  for (uint16_t i = 0; i < in_channels; ++i) {
    padding_ptr_out += input_line_size;
    padding_ptr_out += input_line_size;
    for (uint16_t j = 0; j < input_line_num; ++j) {
      padding_ptr_out += 2;
      DMA_makeTransfer((uint32_t)padding_ptr_in, (uint32_t)padding_ptr_out, input_line_size_bf);
      memcpy(padding_ptr_out, padding_ptr_in, input_line_size_bf*sizeof(uint16_t));
      padding_ptr_in += input_line_size_bf;
      padding_ptr_out += (2 + input_line_size_bf);
    }
    padding_ptr_out += input_line_size;
    padding_ptr_out += input_line_size;
  }

  DMA_makeTransfer(output_fram_addr, input_fram_addr, MAT_GET_SIZE(&out_buffer_meta)); 
  memset(output->data, 0, MAT_GET_SIZE(&out_buffer_meta)*sizeof(uint16_t));

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
      flt_lea_addr += sizeof(uint16_t);
      fill_params.length = LEA_FLT_SIZE;
      msp_fill_q15(&fill_params, lea_flt);

      for (uint16_t k = 0; k < kernel_size; ++k) {
        DMA_makeTransfer(flt_fram_addr, flt_lea_addr, kernel_size);
        flt_lea_addr += flt_addr_padding_offset;
        flt_fram_addr += flt_addr_row_offset;
      }
      /* restore flt_lea_addr pointer to the beginning of the array */
      flt_lea_addr = (uint32_t)lea_flt;

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

  /* add bias and rescale the output */
  uint16_t pos = 0;
  for (uint16_t i = 0; i < out_channels; ++i) {
    for (uint16_t j = 0; j < output_len; ++j) {
      output->data[pos] = __saturated_add_q15(output->data[pos], output->data[pos]);
      output->data[pos] = __saturated_add_q15(output->data[pos], output->data[pos]);
      output->data[pos] = __saturated_add_q15(output->data[pos], output->data[pos]);
      output->data[pos] = __saturated_add_q15(output->data[pos], bias->data[i]);
      ++pos;
    }
  }
  
  fir_clear();
  add_clear();
}