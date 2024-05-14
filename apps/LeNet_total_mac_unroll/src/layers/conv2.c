#include <layers/include/utils.h>
#include <layers/include/conv2.h>
#include <buffer/include/buffer.h>
#include <buffer/include/mac_buffer.h>
#include <libmspsyncioutils/mspsyncioutils.h>
#include <libmsptimer/timekeeper.h>

void conv2(mat_t* input, mat_t* output, mat_t* weight, mat_t* bias) {
  uint16_t in_channels = input->dims[1];
  uint16_t out_channels = output->dims[1];
  uint16_t flt_len = weight->strides[1];
  uint16_t output_len = output->strides[1];
  uint16_t input_len = input->strides[1];
  uint16_t kernel_size = weight->dims[2];
  uint16_t flt_channel_size = weight->strides[0];

  uint32_t flt_lea_addr = (uint32_t)lea_flt;
  uint32_t flt_fram_addr = (uint32_t)(weight->data);
  uint16_t flt_fram_pos = 0;
  uint32_t flt_addr_offset = flt_len * sizeof(int16_t);
  uint32_t flt_channel_offset = flt_channel_size * sizeof(int16_t);
  uint32_t flt_addr_row_offset = kernel_size * sizeof(int16_t);
  uint32_t flt_addr_padding_offset = (kernel_size + 1) * sizeof(int16_t);
  uint32_t input_lea_addr = (uint32_t)lea_src;
  uint32_t input_fram_addr = (uint32_t)(input->data);
  uint16_t input_fram_pos = 0;
  uint32_t input_channel_addr = input_fram_addr;
  uint16_t input_channel_pos = 0;
  uint32_t input_channel_offset = input_len * sizeof(int16_t);
  uint32_t output_lea_addr = (uint32_t)lea_dst;
  uint32_t output_fram_addr = (uint32_t)(output->data);
  uint16_t output_fram_pos = 0;
  uint32_t output_addr_offset = output_len * sizeof(int16_t);

  int16_t* conv_flt = lea_flt;
  uint16_t input_line_size = input->dims[3];
  uint16_t output_line_size = output->dims[3];
  uint16_t output_line_num = output->dims[2];
  uint16_t output_line_size_offset = output_line_size * sizeof(int16_t);
  uint16_t input_line_size_offset = input_line_size * sizeof(int16_t);

  uint16_t lea_remain_size = output_len % LEA_MIN_SIZE;
  uint16_t lea_remain_size_aligned = MAKE_ALIGN_2(lea_remain_size);
  uint32_t output_remain_size_offset = lea_remain_size * sizeof(int16_t);
  uint32_t output_lea_min_size_offset = LEA_MIN_SIZE * sizeof(int16_t);

  uint32_t src_lea_addr = (uint32_t)lea_src;
  uint32_t dst_lea_addr = (uint32_t)lea_dst;

  uint16_t dst_pos;
  uint16_t s_start, s_end;

  msp_add_q15_params add_params = { .length = MAKE_ALIGN_2(output_len) };
  msp_mac_q15_params mac_params = { .length = MAKE_ALIGN_2(flt_len) };
  msp_fill_q15_params fill_params = { .length = LEA_SRC_SIZE, .value = 0 };

  /* reset LEA's RAM */
  msp_fill_q15(&fill_params, lea_src);

  fill_params.length = LEA_FLT_SIZE;
  msp_fill_q15(&fill_params, lea_flt);

  fill_params.length = LEA_DST_SIZE;
  msp_fill_q15(&fill_params, lea_dst);

  fill_params.length = LEA_DST_SIZE;
  msp_fill_q15(&fill_params, lea_tmp); 

  /* convolution */
  for (uint16_t i = 0; i < in_channels; ++i) {
    uint16_t output_channel_pos = 0;
    uint16_t flt_tmp_pos = flt_fram_pos;

    /* assemble input to a matrix in mac_buffer */
    uint16_t mac_buffer_pos = 0;
    input_channel_pos = input_fram_pos;
    for (uint16_t m = 0; m < output_line_num; ++m) {
      uint16_t tmp_input_pos = input_channel_pos;
      for (uint16_t n = 0; n < output_line_size; ++n) {
        uint16_t tmp_input_row_pos = tmp_input_pos;
        for (uint16_t k = 0; k < kernel_size; ++k) {
          s_start = tmp_input_row_pos;
          s_end = tmp_input_row_pos + 5;
          dst_pos = mac_buffer_pos;
          #pragma GCC unroll 5
          for (uint16_t s = s_start; s < s_end; ++s) {
            mac_buffer_meta.data[dst_pos] = input->data[s];
            dst_pos++;
          }

          tmp_input_row_pos += input_line_size;
          mac_buffer_pos += kernel_size;
        }
        tmp_input_pos += 1;
      }
      input_channel_pos += input_line_size;
    }

    for (uint16_t j = 0; j < out_channels; ++j) {
      uint16_t mac_input_pos = 0;
      uint16_t tmp_output_pos = output_channel_pos;

      /*
       * send kernel to LEA RAM
       * assume (lea_flt size >= flt_len + 1)
       */
      s_start = flt_tmp_pos;
      s_end = flt_tmp_pos + 25;
      dst_pos = 0;
      #pragma GCC unroll 25
      for (uint16_t s = s_start; s < s_end; ++s) {
        lea_flt[dst_pos] = weight->data[s];
        dst_pos++;
      }

      /*
       * The LEA array might be smaller than ouput matrix size, so we need to
       * the computation mutiple times.
       */
      if (lea_remain_size) {
        for (uint16_t l = 0; l < lea_remain_size; ++l) {
          s_start = mac_input_pos;
          s_end = mac_input_pos + 25;
          dst_pos = 0;
          #pragma GCC unroll 25
          for (uint16_t s = s_start; s < s_end; ++s) {
            lea_src[dst_pos] = mac_buffer_meta.data[s];
            dst_pos++;
          }

          msp_mac_q15(&mac_params, lea_src, lea_flt, lea_res);
          lea_tmp[l] = (int16_t)(lea_res[0] >> 14);

          mac_input_pos += flt_len;
        }

        add_params.length = lea_remain_size_aligned;

        s_start = tmp_output_pos;
        s_end = tmp_output_pos + 100;
        dst_pos = 0;
        #pragma GCC unroll 100
        for (uint16_t s = s_start; s < s_end; ++s) {
          lea_dst[dst_pos] = output->data[s];
          dst_pos++;
        }

        msp_add_q15(&add_params, lea_dst, lea_tmp, lea_dst);

        s_start = 0;
        s_end = 100;
        dst_pos = tmp_output_pos;
        #pragma GCC unroll 100
        for (uint16_t s = s_start; s < s_end; ++s) {
          output->data[dst_pos] = lea_dst[s];
          dst_pos++;
        }

        tmp_output_pos += lea_remain_size;
      }

      add_params.length = LEA_MIN_SIZE;
      for (uint16_t m = lea_remain_size; m < output_len; m += LEA_MIN_SIZE) {
        for (uint16_t l = 0; l < LEA_MIN_SIZE; ++l) {
          s_start = mac_input_pos;
          s_end = mac_input_pos + 25;
          dst_pos = 0;
          #pragma GCC unroll 25
          for (uint16_t s = s_start; s < s_end; ++s) {
            lea_src[dst_pos] = mac_buffer_meta.data[s];
            dst_pos++;
          }

          msp_mac_q15(&mac_params, lea_src, lea_flt, lea_res);
          lea_tmp[l] = (int16_t)(lea_res[0] >> 14);

          mac_input_pos += flt_len;
        }

        s_start = tmp_output_pos;
        s_end = tmp_output_pos + 100;
        dst_pos = 0;
        #pragma GCC unroll 100
        for (uint16_t s = s_start; s < s_end; ++s) {
          lea_dst[dst_pos] = output->data[s];
          dst_pos++;
        }

        msp_add_q15(&add_params, lea_dst, lea_tmp, lea_dst);

        s_start = 0;
        s_end = 100;
        dst_pos = tmp_output_pos;
        #pragma GCC unroll 100
        for (uint16_t s = s_start; s < s_end; ++s) {
          output->data[dst_pos] = lea_dst[s];
          dst_pos++;
        }

        tmp_output_pos += LEA_MIN_SIZE;
      }

      flt_tmp_pos += flt_channel_size;
      output_channel_pos += output_len;
    }
    input_fram_pos += input_len;
    flt_fram_pos += flt_len;
  }

  /* add bias */
  uint16_t pos = 0;
  for (uint16_t i = 0; i < out_channels; ++i) {
    for (uint16_t j = 0; j < output_len; ++j) {
      output->data[pos] = __saturated_add_q15(output->data[pos], output->data[pos]);
      output->data[pos] = __saturated_add_q15(output->data[pos], output->data[pos]);
      output->data[pos] = __saturated_add_q15(output->data[pos], bias->data[i]);
      ++pos;
    }
  }
}