#include <include/utils.h>
#include <include/cpu_conv.h>
#include <include/data.h>
#include <stdint.h>

void cpu_conv(mat_t* input, mat_t* output, mat_t* weight, mat_t* bias) {
  uint16_t in_channels = input->dims[1];
  uint16_t out_channels = output->dims[1];
  uint16_t input_len = input->strides[1];
  uint16_t kernel_row_size = weight->dims[2];
  uint16_t kernel_col_size = weight->dims[3];
  uint16_t kernel_size = weight->strides[0];

  uint16_t input_line_size = input->dims[3];
  uint16_t output_line_size = output->dims[3];
  uint16_t output_line_num = output->dims[2];

  memset(output->data, 0, GET_MAT_SIZE(output)*sizeof(int16_t));

  uint16_t weight_pos = 0;
  uint16_t output_pos = 0;

  /* convolution */
  for (uint16_t i = 0; i < out_channels; ++i) {
    uint16_t input_row_pos = 0;

    for (uint16_t r = 0; r < output_line_num; ++r) {
      uint16_t input_col_pos = input_row_pos;

      for (uint16_t c = 0; c < output_line_size; ++c) {
        uint16_t weight_tmp_pos = weight_pos;
        uint16_t input_pos = input_col_pos;
        int32_t res = 0;

        for (uint16_t j = 0; j < in_channels; ++j) {
          uint16_t input_kernel_row_pos = input_pos;

          for (uint16_t k = 0; k < kernel_row_size; ++k) {
            uint16_t input_kernel_col_pos = input_kernel_row_pos;

            for (uint16_t n = 0; n < kernel_col_size; ++n) {
              int32_t m = ((int32_t)weight->data[weight_tmp_pos]) * ((int32_t)input->data[input_kernel_col_pos]);

              res = __saturated_add_iq31(res, m);

              input_kernel_col_pos++;
              weight_tmp_pos++;
            }
            input_kernel_row_pos += input_line_size;
          }
          input_pos += input_len;
        }
        input_col_pos++;
        int16_t res_16 = (int16_t)(res >> 14);
        output->data[output_pos] = __saturated_add_q15(res_16, bias->data[i]);
        output_pos++;
      }
      input_row_pos += input_line_size;
    }
    weight_pos += kernel_size;
  }
}