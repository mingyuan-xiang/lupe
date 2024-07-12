#include <include/utils.h>
#include <include/mac_conv.h>
#include <include/data.h>

#define _LEA_DST_SIZE 400
#define _PADDING_TOP 1
#define _PADDING_BOTTOM 1
#define _PADDING_RIGHT 1
#define _PADDING_LEFT 1

#define _INPUT_LINE_SIZE 34
#define _STRIDE_ROW_SIZE 1
#define _STRIDE_COL_SIZE 1
#define _FLT_LEN 9

static inline __attribute__((always_inline)) void new_add_q15(const _q15* srcA, const _q15* srcB, _q15* dst) {
  lea_add_params->input2 = MSP_LEA_CONVERT_ADDRESS(srcB);
  lea_add_params->output = MSP_LEA_CONVERT_ADDRESS(dst);

  /* Load source arguments to LEA. */
  LEAPMS0 = MSP_LEA_CONVERT_ADDRESS(srcA);
  LEAPMS1 = MSP_LEA_CONVERT_ADDRESS(lea_add_params);

  /* Invoke the LEACMD__ADDMATRIX command. */
  msp_lea_invokeCommand(LEACMD__ADDMATRIX);
}

static inline __attribute__((always_inline)) void new_mac_q15(const _q15 *srcA, const _q15 *srcB) {
  lea_mac_params->input2 = MSP_LEA_CONVERT_ADDRESS(srcB);

  /* Load source arguments to LEA. */
  LEAPMS0 = MSP_LEA_CONVERT_ADDRESS(srcA);
  LEAPMS1 = MSP_LEA_CONVERT_ADDRESS(lea_mac_params);

  /* Invoke the LEACMD__MAC command. */
  msp_lea_invokeCommand(LEACMD__MAC);
}

void mac_conv(mat_t* input, mat_t* output, mat_t* weight, mat_t* bias) {
  uint16_t in_channels = input->dims[1];
  uint16_t out_channels = output->dims[1];
  uint16_t output_len = output->strides[1];
  uint16_t input_len = input->strides[1];
  uint16_t kernel_size = weight->dims[2];
  uint16_t _LEA_REMAIN_SIZE = output_len;

  uintptr_t flt_lea_addr = (uintptr_t)lea_flt;
  uintptr_t flt_fram_addr = (uintptr_t)(weight->data);
  uintptr_t flt_addr_offset = _FLT_LEN * sizeof(int16_t);
  uintptr_t flt_channel_offset = weight->strides[0] * sizeof(int16_t);
  uintptr_t flt_addr_row_offset = kernel_size * sizeof(int16_t);
  uintptr_t input_fram_addr = (uintptr_t)(input->data);
  uintptr_t input_channel_addr = input_fram_addr;
  uintptr_t input_channel_offset = input_len * sizeof(int16_t);
  uintptr_t output_fram_addr = (uintptr_t)(output->data);
  uintptr_t output_addr_offset = output_len * sizeof(int16_t);

  uint16_t input_line_size = input->dims[3];
  uint16_t output_line_size = output->dims[3];
  uint16_t output_line_num = output->dims[2];
  uint16_t input_line_size_offset = input_line_size * sizeof(int16_t);
  uint16_t input_line_strided_size = input_line_size * _STRIDE_ROW_SIZE;
  uint16_t input_line_size_strided_offset = input_line_strided_size * sizeof(int16_t);

  uint16_t lea_remain_size = _LEA_REMAIN_SIZE;
  uint16_t lea_remain_size_aligned = MAKE_ALIGN_2(lea_remain_size);
  uintptr_t output_remain_size_offset = lea_remain_size * sizeof(int16_t);
  uintptr_t output_lea_min_size_offset = _LEA_DST_SIZE * sizeof(int16_t);

  uintptr_t src_lea_addr = (uintptr_t)lea_src;
  uintptr_t dst_lea_addr = (uintptr_t)lea_dst;
  
  /* set the aligned position to be zeros */
  lea_src[_FLT_LEN] = 0;
  lea_flt[_FLT_LEN] = 0;
  mac_init(MAKE_ALIGN_2(_FLT_LEN));
  add_init(MAKE_ALIGN_2(_LEA_DST_SIZE));
  uint16_t dst_pos;
  uint16_t s;
  uint16_t input_channel_pos = 0;
  uint16_t input_fram_pos = 0;

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
  uint16_t out_bias_pos = 0;
  /* add bias */
  for (uint16_t i = 0; i < out_channels; ++i) {
    for (uint16_t j = 0; j < output_len; ++j) {
      output->data[out_bias_pos] = bias->data[i];
      ++out_bias_pos;
    }
  }

  /* convolution */
  for (uint16_t i = 0; i < in_channels; ++i) {
    uintptr_t output_channel_addr = output_fram_addr;
    uintptr_t flt_tmp_addr = flt_fram_addr;
    uint16_t tmp_input_addr_offset = sizeof(int16_t) * _STRIDE_COL_SIZE;

    /* assemble input to a matrix in mac_buffer */
    uint16_t mac_buffer_pos = 0;
    input_channel_pos = input_fram_pos;
    for (uint16_t m = 0; m < output_line_num; ++m) {
      uint16_t tmp_input_pos = input_channel_pos;
      for (uint16_t n = 0; n < output_line_size; ++n) {
        uint16_t tmp_input_row_pos = tmp_input_pos;
        for (uint16_t k = 0; k < kernel_size; ++k) {
          s = tmp_input_row_pos;
          dst_pos = mac_buffer_pos;
          data_meta.data[dst_pos] = input->data[s];
          s++;
          dst_pos++;
          data_meta.data[dst_pos] = input->data[s];
          s++;
          dst_pos++;
          data_meta.data[dst_pos] = input->data[s];
          s++;
          dst_pos++;

          tmp_input_row_pos += input_line_size;
          mac_buffer_pos += kernel_size;
        }
        tmp_input_pos += _STRIDE_COL_SIZE;
      }
      input_channel_pos += input_line_strided_size;
    }

    for (uint16_t j = 0; j < out_channels; ++j) {
      uintptr_t mac_input_addr = (uintptr_t)data_meta.data;
      uintptr_t tmp_output_addr = output_channel_addr;

      /*
       * send kernel to LEA RAM
       * assume (lea_flt size >= flt_len + 1)
       */
      DMA_makeTransfer(flt_tmp_addr, flt_lea_addr, _FLT_LEN);

      /*
       * The LEA array might be smaller than ouput matrix size, so we need to
       * the computation mutiple times.
       */
      for (uint16_t l = 0; l < lea_remain_size; ++l) {
        DMA_makeTransfer(mac_input_addr, src_lea_addr, _FLT_LEN);
        new_mac_q15(lea_src, lea_flt);
        lea_tmp[l] = (int16_t)(lea_res[0] >> 15);

        mac_input_addr += flt_addr_offset;
      }
      lea_add_params->vectorSize = lea_remain_size_aligned;
      DMA_makeTransfer(tmp_output_addr, dst_lea_addr, lea_remain_size);
      new_add_q15(lea_dst, lea_tmp, lea_dst);
      DMA_makeTransfer(dst_lea_addr, tmp_output_addr, lea_remain_size);
      tmp_output_addr += output_remain_size_offset;
      lea_add_params->vectorSize = _LEA_DST_SIZE;

      for (uint16_t m = lea_remain_size; m < output_len; m += _LEA_DST_SIZE) {
        for (uint16_t l = 0; l < _LEA_DST_SIZE; ++l) {
          DMA_makeTransfer(mac_input_addr, src_lea_addr, _FLT_LEN);
          new_mac_q15(lea_src, lea_flt);
          lea_tmp[l] = (int16_t)(lea_res[0] >> 15);

          mac_input_addr += flt_addr_offset;
        }

        DMA_makeTransfer(tmp_output_addr, dst_lea_addr, _LEA_DST_SIZE);
        new_add_q15(lea_dst, lea_tmp, lea_dst);
        DMA_makeTransfer(dst_lea_addr, tmp_output_addr, _LEA_DST_SIZE);

        tmp_output_addr += output_lea_min_size_offset;
      }

      flt_tmp_addr += flt_channel_offset;
      output_channel_addr += output_addr_offset;
    }
    input_fram_pos += input_len;
    flt_fram_addr += flt_addr_offset;
  }
  add_clear();
  mac_clear();
}