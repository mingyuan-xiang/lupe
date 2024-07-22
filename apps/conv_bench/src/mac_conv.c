#include <include/utils.h>
#include <include/mac_conv.h>
#include <include/data.h>
#include <stdint.h>

/*
 * _LEA_SRC_SIZE will always be mutiple of _FLT_LEN
 * so that _LEA_REMAIN_SIZE will always be multiple of _FLT_LEN.
 */
#define _FLT_LEN 9
#define __LEA_SRC_SIZE 784
#define _LEA_SRC_SIZE (__LEA_SRC_SIZE - (__LEA_SRC_SIZE % _FLT_LEN))
#define _LEA_SRC_SIZE_CHANNEL_CNT (_LEA_SRC_SIZE / _FLT_LEN)

#define _LEA_ADD_SIZE 1600
#define _LEA_ADD_REMAIN_SIZE (1024 % _LEA_ADD_SIZE)

#define _PADDING_TOP 1
#define _PADDING_BOTTOM 1
#define _PADDING_RIGHT 1
#define _PADDING_LEFT 1

/* assign the lea buffer pointer */
#define lea_src (lea_buffer)
#define lea_flt (lea_buffer + 784)
#define lea_tmp (lea_buffer + 1568)
#define lea_dst (lea_buffer + 1568)

#define _STRIDE_ROW_SIZE 1
#define _STRIDE_COL_SIZE 1
#define _STRIDE_COL_OFFSET (_STRIDE_COL_SIZE * sizeof(int16_t))

static inline __attribute__((always_inline)) void new_mac_q15(const _q15 *srcA, const _q15 *srcB) {
  lea_mac_params->input2 = MSP_LEA_CONVERT_ADDRESS(srcB);

  /* Load source arguments to LEA. */
  LEAPMS0 = MSP_LEA_CONVERT_ADDRESS(srcA);
  LEAPMS1 = MSP_LEA_CONVERT_ADDRESS(lea_mac_params);

  /* Invoke the LEACMD__MAC command. */
  msp_lea_invokeCommand(LEACMD__MAC);
}

static inline __attribute__((always_inline)) void new_add_q15(const _q15* srcA, const _q15* srcB, _q15* dst) {
  lea_add_params->input2 = MSP_LEA_CONVERT_ADDRESS(srcB);
  lea_add_params->output = MSP_LEA_CONVERT_ADDRESS(dst);

  /* Load source arguments to LEA. */
  LEAPMS0 = MSP_LEA_CONVERT_ADDRESS(srcA);
  LEAPMS1 = MSP_LEA_CONVERT_ADDRESS(lea_add_params);

  /* Invoke the LEACMD__ADDMATRIX command. */
  msp_lea_invokeCommand(LEACMD__ADDMATRIX);
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

void mac_conv(mat_t* input, mat_t* output, mat_t* weight, mat_t* bias) {
  uint16_t in_channels = input->dims[1];
  uint16_t out_channels = output->dims[1];
  uint16_t output_len = output->strides[1];
  uint16_t input_len = input->strides[1];
  uint16_t kernel_size = weight->dims[2];
  uint16_t input_line_size = input->dims[3];

  uint16_t _IN_CHANNEL_NUM = in_channels;
  uint16_t _MAC_LEN = _FLT_LEN * _IN_CHANNEL_NUM;
  uint16_t _LEA_REMAIN_SIZE = _MAC_LEN % _LEA_SRC_SIZE;
  uint16_t _LEA_REMAIN_SIZE_CHANNEL_CNT = _LEA_REMAIN_SIZE / _FLT_LEN;
  uint16_t _INPUT_LINE_SIZE = input_line_size;

  uintptr_t flt_lea_addr = (uintptr_t)lea_flt;
  uintptr_t flt_fram_addr = (uintptr_t)(weight->data);
  uintptr_t mac_size = _LEA_REMAIN_SIZE;
  
  uintptr_t flt_channel_offset = weight->strides[0] * sizeof(int16_t);
  uintptr_t flt_addr_row_offset = kernel_size * sizeof(int16_t);
  uintptr_t input_fram_addr = (uintptr_t)(input->data);
  uintptr_t input_channel_offset = input_len * sizeof(int16_t);
  uintptr_t output_fram_addr = (uintptr_t)(output->data);
  uintptr_t lea_src_addr = (uintptr_t)lea_src;

  uint16_t lea_mac_remain_size_aligned = MAKE_ALIGN_2(_LEA_REMAIN_SIZE);

  uint16_t output_line_size = output->dims[3];
  uint16_t output_line_num = output->dims[2];
  uint16_t input_line_size_offset = input_line_size * sizeof(int16_t);
  uint16_t input_line_strided_size = input_line_size * _STRIDE_ROW_SIZE;
  uint16_t input_line_size_strided_offset = input_line_strided_size * sizeof(int16_t);

  uint16_t lea_remain_size = _LEA_ADD_REMAIN_SIZE;
  uintptr_t output_remain_size_offset = lea_remain_size * sizeof(int16_t);
  uint16_t lea_remain_size_aligned = MAKE_ALIGN_2(lea_remain_size);
  mac_init(lea_mac_remain_size_aligned);
  add_init(lea_remain_size_aligned);
  offset_init(lea_remain_size_aligned);
  uint16_t dst_pos;
  uint16_t s;
  uint16_t input_channel_pos = 0;
  uint16_t input_fram_pos = 0;
  uint16_t zero = 0;

  /* convolution */
  uint16_t out_pos = 0;
  uint16_t tmp_out_pos = 0;
  for (uint16_t m = 0; m < output_line_num; ++m) {
    uint16_t tmp_input_pos = input_fram_pos;
    for (uint16_t n = 0; n < output_line_size; ++n) {
      uint16_t tmp_channel_pos = tmp_input_pos;
      uint16_t mac_buffer_pos = 0;
      uintptr_t flt_tmp_addr = flt_fram_addr;
      uintptr_t flt_mac_addr = flt_tmp_addr;
      /* set the aligned position to be zeros */
      lea_src[_LEA_REMAIN_SIZE] = 0;
      lea_flt[_LEA_REMAIN_SIZE] = 0;

      /* assemble input to a matrix in mac_buffer */
      for (uint16_t i = 0; i < _LEA_REMAIN_SIZE_CHANNEL_CNT; ++i) {
        uint16_t tmp_input_row_pos = tmp_channel_pos;
        for (uint16_t k = 0; k < kernel_size; ++k) {
          s = tmp_input_row_pos;
          dst_pos = mac_buffer_pos;
          lea_src[dst_pos] = input->data[s];
          s++;
          dst_pos++;
          lea_src[dst_pos] = input->data[s];
          s++;
          dst_pos++;
          lea_src[dst_pos] = input->data[s];
          s++;
          dst_pos++;

          tmp_input_row_pos += input_line_size;
          mac_buffer_pos += kernel_size;
        }
        tmp_channel_pos += input_len;
      }
      tmp_out_pos = out_pos;
      for (uint16_t j = 0; j < out_channels; ++j) {
        DMA_makeTransfer(flt_mac_addr, flt_lea_addr, mac_size);
        
        new_mac_q15(lea_src, lea_flt);

        int16_t mac_out = (int16_t)(lea_res[0] >> 16);
        output->data[tmp_out_pos] = mac_out;

        flt_mac_addr += flt_channel_offset;
        tmp_out_pos += output_len;
      }
      
      tmp_input_pos += _STRIDE_COL_SIZE;
      out_pos++;
    }
    input_fram_pos += input_line_strided_size;
  }

  offset_clear();
  add_clear();
  mac_clear();
}