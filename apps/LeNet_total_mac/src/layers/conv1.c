#include <layers/include/utils.h>
#include <layers/include/conv1.h>
#include <buffer/include/buffer.h>
#include <buffer/include/mac_buffer.h>
#include <libmspsyncioutils/mspsyncioutils.h>
#include <libmsptimer/timekeeper.h>

void conv1(mat_t* input, mat_t* output, mat_t* weight, mat_t* bias) {
  uint16_t in_channels = input->dims[1];
  uint16_t out_channels = output->dims[1];
  uint16_t flt_len = weight->strides[1];
  uint16_t output_len = output->strides[1];
  uint16_t input_len = input->strides[1];
  uint16_t kernel_size = weight->dims[2];

  uint32_t flt_lea_addr = (uint32_t)lea_flt;
  uint32_t flt_fram_addr = (uint32_t)(weight->data);
  uint32_t flt_addr_offset = flt_len * sizeof(int16_t);
  uint32_t flt_channel_offset = weight->strides[0] * sizeof(int16_t);
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

  uint16_t lea_remain_size = output_len % LEA_MIN_SIZE;
  uint16_t lea_remain_size_aligned = MAKE_ALIGN_2(lea_remain_size);
  uint32_t output_remain_size_offset = lea_remain_size * sizeof(int16_t);
  uint32_t output_lea_min_size_offset = LEA_MIN_SIZE * sizeof(int16_t);

  uint32_t src_lea_addr = (uint32_t)lea_src;
  uint32_t dst_lea_addr = (uint32_t)lea_dst;

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
      memcpy(padding_ptr_out, padding_ptr_in, input_line_size_bf*sizeof(uint16_t));
      padding_ptr_in += input_line_size_bf;
      padding_ptr_out += (2 + input_line_size_bf);
    }
    padding_ptr_out += input_line_size;
    padding_ptr_out += input_line_size;
  }
  memcpy(input->data, output->data, MAT_GET_SIZE(&out_buffer_meta)*sizeof(uint16_t));
  memset(output->data, 0, MAT_GET_SIZE(&out_buffer_meta)*sizeof(uint16_t)); 

  /* convolution */
  for (uint16_t i = 0; i < in_channels; ++i) {
    uint32_t output_channel_addr = output_fram_addr;
    uint32_t flt_tmp_addr = flt_fram_addr;

    /* assemble input to a matrix in mac_buffer */
    uint32_t mac_buffer_addr = (uint32_t)mac_buffer_meta.data;
    input_channel_addr = input_fram_addr;
    for (uint16_t m = 0; m < output_line_num; ++m) {
      uint32_t tmp_input_addr = input_channel_addr;
      for (uint16_t n = 0; n < output_line_size; ++n) {
        uint32_t tmp_input_row_addr = tmp_input_addr;
        for (uint16_t k = 0; k < kernel_size; ++k) {
          DMA_makeTransfer(tmp_input_row_addr, mac_buffer_addr, kernel_size);

          tmp_input_row_addr += input_line_size_offset;
          mac_buffer_addr += flt_addr_row_offset;
        }
        tmp_input_addr += sizeof(int16_t);
      }
      input_channel_addr += input_line_size_offset;
    }

    for (uint16_t j = 0; j < out_channels; ++j) {
      uint32_t mac_input_addr = (uint32_t)mac_buffer_meta.data;
      uint32_t tmp_output_addr = output_channel_addr;

      /*
       * send kernel to LEA RAM
       * assume (lea_flt size >= flt_len + 1)
       */
      DMA_makeTransfer(flt_tmp_addr, flt_lea_addr, flt_len);

      /*
       * The LEA array might be smaller than ouput matrix size, so we need to
       * the computation mutiple times.
       */
      if (lea_remain_size) {
        for (uint16_t l = 0; l < lea_remain_size; ++l) {
          DMA_makeTransfer(mac_input_addr, src_lea_addr, flt_len);
          msp_mac_q15(&mac_params, lea_src, lea_flt, lea_res);
          lea_tmp[l] = (int16_t)(lea_res[0] >> 14);

          mac_input_addr += flt_addr_offset;
        }

        add_params.length = lea_remain_size_aligned;
        DMA_makeTransfer(tmp_output_addr, dst_lea_addr, lea_remain_size);
        msp_add_q15(&add_params, lea_dst, lea_tmp, lea_dst);
        DMA_makeTransfer(dst_lea_addr, tmp_output_addr, lea_remain_size);

        tmp_output_addr += output_remain_size_offset;
      }

      add_params.length = LEA_MIN_SIZE;
      for (uint16_t m = lea_remain_size; m < output_len; m += LEA_MIN_SIZE) {
        for (uint16_t l = 0; l < LEA_MIN_SIZE; ++l) {
          DMA_makeTransfer(mac_input_addr, src_lea_addr, flt_len);
          msp_mac_q15(&mac_params, lea_src, lea_flt, lea_res);
          lea_tmp[l] = (int16_t)(lea_res[0] >> 14);

          mac_input_addr += flt_addr_offset;
        }

        DMA_makeTransfer(tmp_output_addr, dst_lea_addr, LEA_MIN_SIZE);
        msp_add_q15(&add_params, lea_dst, lea_tmp, lea_dst);
        DMA_makeTransfer(dst_lea_addr, tmp_output_addr, LEA_MIN_SIZE);

        tmp_output_addr += output_lea_min_size_offset;
      }

      flt_tmp_addr += flt_channel_offset;
      output_channel_addr += output_addr_offset;
    }
    input_fram_addr += input_channel_offset;
    flt_fram_addr += flt_addr_offset;
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