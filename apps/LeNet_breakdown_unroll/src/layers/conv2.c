#include <layers/include/utils.h>
#include <layers/include/conv2.h>
#include <buffer/include/buffer.h>
#include <libmspsyncioutils/mspsyncioutils.h>
#include <libmsptimer/timekeeper.h>

void conv2(mat_t* input, mat_t* output, mat_t* weight, mat_t* bias) {
  uint16_t in_channels = input->dims[1];
  uint16_t out_channels = output->dims[1];
  uint16_t flt_len = weight->strides[1];
  uint16_t output_len = output->strides[1];
  uint16_t input_len = input->strides[1];
  uint16_t kernel_size = weight->dims[2];

  uint32_t flt_lea_addr = (uint32_t)lea_flt;
  uint32_t flt_fram_addr = (uint32_t)(weight->data);
  uint32_t flt_addr_offset = flt_len * sizeof(int16_t);
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

  uint32_t init_time = 0;
  uint32_t comp_time = 0;
  uint32_t mem_time = 0;

  msp_fir_q15_params conv_params = {
    .length = MAKE_ALIGN_2(output_line_size),
    .tapLength = MAKE_ALIGN_2(kernel_size),
    .coeffs = lea_flt,
    .enableCircularBuffer = false 
  };
  msp_add_q15_params add_params = { .length = MAKE_ALIGN_2(output_line_size) };
  msp_fill_q15_params fill_params = { .length = LEA_SRC_SIZE, .value = 0 };

  /* reset LEA's RAM */
  msp_fill_q15(&fill_params, lea_src);

  fill_params.length = LEA_FLT_SIZE;
  msp_fill_q15(&fill_params, lea_flt);

  fill_params.length = LEA_DST_SIZE;
  msp_fill_q15(&fill_params, lea_dst);

  fill_params.length = LEA_DST_SIZE;
  msp_fill_q15(&fill_params, lea_tmp); 

  init_time += stop_timer();

  /* convolution */
  uint16_t flt_pos = 0;
  for (uint16_t i = 0; i < out_channels; ++i) {
    input_fram_addr = (uint32_t)(input->data);
    for (uint16_t j = 0; j < in_channels; ++j) {
      uint32_t tmp_output_addr = output_fram_addr;
      input_channel_addr = input_fram_addr;

      /* send kernel to LEA RAM */
      start_timer();
      if (kernel_size % 2) {
        uint16_t lea_flt_pos = 0;
        #pragma GCC unroll 5
        for (uint16_t k = 0; k < 5; ++k) {
          lea_flt[lea_flt_pos] = 0;
          lea_flt_pos++;
          #pragma GCC unroll 5
          for (uint16_t s = 0; s < 5; ++s) {
            lea_flt[lea_flt_pos] = weight->data[flt_pos];
            flt_pos++;
            lea_flt_pos++;
          }
        }
      } else {
          DMA_makeTransfer(flt_fram_addr, flt_lea_addr, kernel_size);

          flt_lea_addr += flt_addr_padding_offset;
          flt_fram_addr += flt_addr_row_offset;
      }
      mem_time += stop_timer();

      for (uint16_t l = 0; l < output_line_num; ++l) {
        uint32_t tmp_input_addr = input_channel_addr;
        /* send output to LEA RAM */
        start_timer();
        DMA_makeTransfer(tmp_output_addr, output_lea_addr, output_line_size);
        mem_time += stop_timer();

        conv_flt = lea_flt;

        for (uint16_t k = 0; k < kernel_size; ++k) {
          /* send input to LEA RAM */
          start_timer();
          DMA_makeTransfer(tmp_input_addr, input_lea_addr, input_line_size);
          mem_time += stop_timer();

          conv_params.coeffs = conv_flt;

          /* convolution */
          start_timer();
          msp_fir_q15(&conv_params, lea_src, lea_tmp);
          comp_time += stop_timer();

          /* accumulate results for a 2D convolution */
          start_timer();
          msp_add_q15(&add_params, lea_dst, lea_tmp, lea_dst);
          comp_time += stop_timer();

          conv_flt += conv_params.tapLength;
          tmp_input_addr += input_line_size_offset;
        }

        /* bring back output from LEA RAM */
        start_timer();
        DMA_makeTransfer(output_lea_addr, tmp_output_addr, output_line_size);
        mem_time += stop_timer();

        tmp_output_addr += output_line_size_offset;
        input_channel_addr += input_line_size_offset; 
      }
      input_fram_addr += input_channel_offset;
    }
    output_fram_addr += output_addr_offset;
  }

  /* add bias */
  start_timer();
  uint16_t pos = 0;
  for (uint16_t i = 0; i < out_channels; ++i) {
    for (uint16_t j = 0; j < output_len; ++j) {
      output->data[pos] = __saturated_add_q15(output->data[pos], output->data[pos]);
      output->data[pos] = __saturated_add_q15(output->data[pos], output->data[pos]);
      output->data[pos] = __saturated_add_q15(output->data[pos], bias->data[i]);
      ++pos;
    }
  }
  comp_time += stop_timer();

  msp_send_printf("initialization time: %n", init_time);
  msp_send_printf("compute time: %n", comp_time);
  msp_send_printf("memory time: %n", mem_time);
}