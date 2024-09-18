#include <layers/include/utils.h>
#include <layers/include/pool2_AveragePool.h>
#include <buffer/include/buffer.h>
#include <layers/include/intermittent.h>

void pool2_AveragePool(mat_t* input, mat_t* output) {
  uint16_t in_ch = input->dims[1];
  uint16_t rows = input->dims[2];
  uint16_t cols = input->dims[3];
  uint16_t in_row_stride = input->strides[2];
  uint16_t in_len = input->strides[1];
  uint16_t out_len = output->strides[1];
  uint16_t out_rows = output->dims[2];
  uint16_t out_cols = output->dims[3];
  uint16_t height = 2;
  uint16_t width = 2;
  uint16_t pool_row_offset = in_row_stride * height;
  int32_t agg;

  if (intermittent_status[COMPUTE_CK] == INTERMITTENT_pool2_AveragePool_PREPARE) {
    if (intermittent_status[COMPUTE_IO_COL] > out_cols) {
      intermittent_status[COMPUTE_IO_COL] = 0;
      intermittent_status[COMPUTE_IO_ROW]++;
    }

    if (intermittent_status[COMPUTE_IO_ROW] & DOUBLE_BUFFER_WRITE) {
      uint16_t idx = intermittent_status[COMPUTE_IO_ROW] & DOUBLE_BUFFER_COMPLETE;
      intermittent_status[COMPUTE_IO_COL] = intermittent_buffer[0];
      intermittent_status[COMPUTE_IO_ROW] = idx;
    }

    if (intermittent_status[COMPUTE_IO_ROW] > out_rows) {
      intermittent_status[COMPUTE_IO_ROW] = 0;
      intermittent_status[COMPUTE_IN_CH]++;
    }

    if (intermittent_status[COMPUTE_IN_CH] & DOUBLE_BUFFER_WRITE) {
      uint16_t idx = intermittent_status[COMPUTE_IN_CH] & DOUBLE_BUFFER_COMPLETE;
      intermittent_status[COMPUTE_IO_ROW] = intermittent_buffer[0];
      intermittent_status[COMPUTE_IN_CH] = idx;
    }

    uint16_t ch_offset = intermittent_status[COMPUTE_IN_CH] * in_len + \
      intermittent_status[COMPUTE_IO_ROW] * pool_row_offset;
    uint16_t out_offset = intermittent_status[COMPUTE_IN_CH] * out_len + \
      intermittent_status[COMPUTE_IO_ROW] * out_rows + \
      intermittent_status[COMPUTE_IO_COL];
    for (uint16_t s = intermittent_status[COMPUTE_IN_CH]; s < in_ch; ++s) {
      for (uint16_t r = intermittent_status[COMPUTE_IO_ROW]; r < rows; r += height) {
        uint16_t in_row_offset = ch_offset + intermittent_status[COMPUTE_IO_COL] * width;
        for (uint16_t c = intermittent_status[COMPUTE_IO_COL]; c < cols; c += width) {
          agg = 0;
          uint16_t h = r + height;
          uint16_t w = c + width;
          uint16_t h_offset = in_row_offset;
          for (uint16_t i = r; i < h; ++i) {
            uint16_t in_offset = h_offset;
            for (uint16_t j = c; j < w; ++j) {
              int16_t in_data = *(input->data + in_offset);
              agg = __saturated_add_iq31(agg, in_data);
              ++in_offset;
            }
            h_offset += in_row_stride;
          }
          agg /= 4;
          *(output->data + out_offset) = agg;
          ++out_offset;
          in_row_offset += width;

          intermittent_status[COMPUTE_IO_COL]++;
        }
        ch_offset += pool_row_offset;

        uint16_t next_r = r + 1;
        DOUBLE_BUFFER_ASSIGN(next_r, COMPUTE_IO_ROW, 0, intermittent_status[COMPUTE_IO_COL]);
      }

      uint16_t next_s = s + 1;
      DOUBLE_BUFFER_ASSIGN(next_s, COMPUTE_IN_CH, 0, intermittent_status[COMPUTE_IO_ROW]);
    }

    intermittent_status[COMPUTE_CK] = INTERMITTENT_pool2_AveragePool_EXIT;
  }

  if (intermittent_status[COMPUTE_CK] == INTERMITTENT_pool2_AveragePool_EXIT) {
    intermittent_status[COMPUTE_IO_COL] = 0;
    intermittent_status[COMPUTE_IO_ROW] = 0;
    intermittent_status[COMPUTE_IN_CH] = 0;
  }
}