#include <layers/include/utils.h>
#include <layers/include/pool2_AveragePool.h>
#include <buffer/include/buffer.h>
#include <libmspsyncioutils/mspsyncioutils.h>
#include <libmsptimer/timekeeper.h>

void pool2_AveragePool(mat_t* input, mat_t* output) {
  uint16_t in_ch = input->dims[1];
  uint16_t rows = input->dims[2];
  uint16_t cols = input->dims[3];
  uint16_t in_ch_stride = input->strides[1];
  uint16_t in_row_stride = input->strides[2];
  uint16_t out_ch_stride = output->strides[1];
  uint16_t out_row_stride = output->strides[2];
  
  uint16_t height = 2;
  uint16_t width = 2;
  uint16_t out_r = 0;
  uint16_t out_c = 0;

  int16_t agg;

  for (uint16_t s = 0; s < in_ch; ++s) {
    out_r = 0;
    for (uint16_t r = 0; r < rows; r += height) {
      out_c = 0;
      for (uint16_t c = 0; c < cols; c += width) {
        agg = 0;
        uint16_t h = r + height;
        uint16_t w = c + width;
        for (uint16_t i = r; i < h; ++i) {
          for (uint16_t j = c; j < w; ++j) {
            uint16_t offset = s * in_ch_stride + i * in_row_stride + j;
            int16_t in_data = *(input->data + offset);
            agg = __saturated_add_q15(agg, in_data >> 2);
          }
        }
        uint16_t offset = s * out_ch_stride + out_r * out_row_stride + out_c;
        *(output->data + offset) = agg;
        ++out_c;
      }
      ++out_r;
    }
  }
}