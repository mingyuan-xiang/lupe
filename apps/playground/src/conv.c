#include <include/utils.h>
#include <include/conv.h>
#include <include/lupe_buffer.h>
#include <include/intermittent.h>

#define _LEA_DST_SIZE 120
#define _LEA_REMAIN_SIZE (400 % _LEA_DST_SIZE)

#define _INPUT_LINE_SIZE 20
#define _STRIDE_ROW_SIZE 1
#define _STRIDE_ROW_LINE_SIZE (_STRIDE_ROW_SIZE * _INPUT_LINE_SIZE)
#define _STRIDE_COL_SIZE 1

void conv(mat_t* input, mat_t* output, mat_t* weight, mat_t* bias) {
  uint16_t in_channels = input->dims[1];
  uint16_t out_channels = output->dims[1];
  uint16_t output_len = output->strides[1];
  uint16_t input_len = input->strides[1];

  uintptr_t input_fram_addr = (uintptr_t)(input->data);
  uintptr_t input_channel_addr = input_fram_addr;
  uintptr_t tmp_input_addr = input_channel_addr;
  uintptr_t output_addr_offset = output_len * sizeof(int16_t);

  uint16_t output_line_num = output->dims[2];
  uint16_t output_line_size = output->dims[3];

  uint16_t lea_remain_size = _LEA_REMAIN_SIZE;
  uint16_t lea_remain_size_aligned = MAKE_ALIGN_2(lea_remain_size);
  uintptr_t remain_size_offset = lea_remain_size * sizeof(int16_t);
  uintptr_t lea_min_size_offset = _LEA_DST_SIZE * sizeof(int16_t);

  uintptr_t lea_src_addr = (uintptr_t)lea_src;
  uintptr_t lea_dst_addr = (uintptr_t)lea_dst;
  msp_add_q15_params add_params = { .length = lea_remain_size_aligned };
  msp_fill_q15_params fill_params = { .length = lea_remain_size_aligned, .value = 0 };
  msp_mpy_q15_params mpy_params = { .length = lea_remain_size_aligned };

  uint16_t tmp_weight_pos = 0;
  
  if (intermittent_status[COMPUTE_CK] == INTERMITTENT_features_features_1_pw_conv_linear_pw_conv_linear_0_Conv_PREPARE) {
    switch (intermittent_status[COMPUTE_SWITCH]) {
    case RESHAPE_START: {
      /* add bias */
      uint16_t out_bias_pos = intermittent_status[COMPUTE_OUT_CH] * output_len + \
        intermittent_status[COMPUTE_IO_ROW];
      for (uint16_t i = intermittent_status[COMPUTE_OUT_CH]; i < out_channels; ++i) {
        for (uint16_t j = intermittent_status[COMPUTE_IO_ROW]; j < output_len; ++j) {
          output->data[out_bias_pos] = bias->data[i];

          uint16_t next_j = j + 1;
          VOLATILE_WRITE(next_j, COMPUTE_IO_ROW);
          ++out_bias_pos;
        }

        uint16_t next_i = i + 1;
        VOLATILE_WRITE(next_i, COMPUTE_OUT_CH);
      }

      VOLATILE_WRITE(RESHAPE_PREPARE_ST_RESET, COMPUTE_SWITCH);
    }
    case RESHAPE_PREPARE_ST_RESET:
      VOLATILE_WRITE(0, COMPUTE_OUT_CH);
      VOLATILE_WRITE(0, COMPUTE_IO_ROW);
      VOLATILE_WRITE(RESHAPE_MAIN, COMPUTE_SWITCH);
    case RESHAPE_MAIN: {
      uint16_t ptwise_pos = intermittent_status[COMPUTE_IN_CH] * output_len + \
        intermittent_status[COMPUTE_IO_ROW] * output_line_size + \
        intermittent_status[COMPUTE_IO_COL];
      uint16_t input_ch = intermittent_status[COMPUTE_IN_CH] * input_len;
      for (uint16_t c = intermittent_status[COMPUTE_IN_CH]; c < in_channels; ++c) {
        uint16_t input_row = input_ch + intermittent_status[COMPUTE_IO_ROW] * _STRIDE_ROW_LINE_SIZE;
        for (uint16_t r = intermittent_status[COMPUTE_IO_ROW]; r < output_line_num; ++r) {
          uint16_t input_col = input_row + intermittent_status[COMPUTE_IO_COL] * _STRIDE_COL_SIZE;
          for (uint16_t m = intermittent_status[COMPUTE_IO_COL]; m < output_line_size; ++m) {
            lupe_buffer_meta.data[ptwise_pos] = input->data[input_col];

            uint16_t next_m = m + 1;
            VOLATILE_WRITE(next_m, COMPUTE_IO_COL);

            input_col += _STRIDE_COL_SIZE;
            ++ptwise_pos;
          }

          VOLATILE_WRITE(0, COMPUTE_IO_COL);
          uint16_t next_r = r + 1;
          VOLATILE_WRITE(next_r, COMPUTE_IO_ROW);

          input_row += _STRIDE_ROW_LINE_SIZE;
        }

        VOLATILE_WRITE(0, COMPUTE_IO_ROW);
        uint16_t next_c = c + 1;
        VOLATILE_WRITE(next_c, COMPUTE_IN_CH);

        input_ch += input_len;
      }

      VOLATILE_WRITE(RESHAPE_ST_RESET, COMPUTE_SWITCH);
    }
    case RESHAPE_ST_RESET:
      VOLATILE_WRITE(0, COMPUTE_IO_COL);
      VOLATILE_WRITE(0, COMPUTE_IO_ROW);
      VOLATILE_WRITE(0, COMPUTE_IN_CH);
      VOLATILE_WRITE(RESHAPE_END, COMPUTE_SWITCH);
    default:
      break;
    }

    intermittent_status[COMPUTE_CK] = INTERMITTENT_features_features_1_pw_conv_linear_pw_conv_linear_0_Conv_MAIN;
  }

  uintptr_t intermittent_buffer_addr = (uintptr_t)intermittent_buffer;
  /*
   * We use the later half of intermittent_buffer to store the temporary MPY results.
   * This is safe as the MPY output will always be smaller than 1/3 of the entire intermittent_buffer.
   */
  uintptr_t intermittent_mpy_buffer_addr = intermittent_buffer_addr + INTERMITTENT_BUFFER_SIZE;

  /* convolution */
  if (intermittent_status[COMPUTE_CK] == INTERMITTENT_features_features_1_pw_conv_linear_pw_conv_linear_0_Conv_MAIN) {
    if (intermittent_status[COMPUTE_OUT_CH] & DOUBLE_BUFFER_WRITE) {
      uint16_t idx = intermittent_status[COMPUTE_OUT_CH] & DOUBLE_BUFFER_COMPLETE;
      VOLATILE_WRITE(intermittent_buffer[0], COMPUTE_IO_ROW);
      VOLATILE_WRITE(idx, COMPUTE_OUT_CH);
    }

    if (intermittent_status[COMPUTE_IO_ROW] & DOUBLE_BUFFER_WRITE) {
      uint16_t line = intermittent_status[COMPUTE_IO_ROW] & DOUBLE_BUFFER_COMPLETE;
      uint16_t size = _LEA_REMAIN_SIZE;
      uintptr_t offset = 0;
      if (line > _LEA_REMAIN_SIZE) {
        size = _LEA_DST_SIZE;
        offset = (line - size) * sizeof(int16_t);
      }
      uintptr_t addr = (uintptr_t)(output->data) + \
        intermittent_status[COMPUTE_OUT_CH] * output_addr_offset + offset;

      DMA_makeTransfer(intermittent_buffer_addr, addr, size);
      intermittent_status[COMPUTE_IN_CH] = 0;
      VOLATILE_WRITE(line, COMPUTE_IO_ROW);
    }

    if (intermittent_status[COMPUTE_IN_CH] & DOUBLE_BUFFER_WRITE) {
      uint16_t idx = intermittent_status[COMPUTE_IN_CH] & DOUBLE_BUFFER_COMPLETE;
      uint16_t size = _LEA_REMAIN_SIZE;
      if (intermittent_status[COMPUTE_IO_ROW] > 0) {
        size = _LEA_DST_SIZE;
      }

      DMA_makeTransfer(intermittent_buffer_addr, intermittent_mpy_buffer_addr, size);
      VOLATILE_WRITE(idx, COMPUTE_IN_CH);
    }

    if (intermittent_status[COMPUTE_IN_CH] > 0) {
      uint16_t size = _LEA_REMAIN_SIZE;
      if (intermittent_status[COMPUTE_IO_ROW] > 0) {
        size = _LEA_DST_SIZE;
      }

      DMA_makeTransfer(intermittent_mpy_buffer_addr, lea_dst_addr, size);
    }

    if (intermittent_status[COMPUTE_IO_ROW] >= output_len) {
      uint16_t next_idx = intermittent_status[COMPUTE_OUT_CH] + 1;
      DOUBLE_BUFFER_ASSIGN(next_idx, COMPUTE_OUT_CH, 0, intermittent_status[COMPUTE_IO_ROW]);
    }
  
    uintptr_t output_fram_addr = (uintptr_t)(output->data) + \
      intermittent_status[COMPUTE_OUT_CH] * output_addr_offset + \
      intermittent_status[COMPUTE_IO_ROW] * sizeof(int16_t);
    uint16_t weight_pos = intermittent_status[COMPUTE_OUT_CH] * in_channels;

    for (uint16_t i = intermittent_status[COMPUTE_OUT_CH]; i < out_channels; ++i) {
      input_channel_addr = input_fram_addr + intermittent_status[COMPUTE_IO_ROW] * sizeof(int16_t);

      if (intermittent_status[COMPUTE_IO_ROW] == 0) {
        if (intermittent_status[COMPUTE_IN_CH] == 0) {
          DMA_makeTransfer(output_fram_addr, lea_dst_addr, lea_remain_size);
        }

        add_params.length = lea_remain_size_aligned;
        fill_params.length = lea_remain_size_aligned;
        mpy_params.length = lea_remain_size_aligned;

        tmp_input_addr = input_channel_addr + \
          intermittent_status[COMPUTE_IN_CH] * output_addr_offset;
        tmp_weight_pos = weight_pos + intermittent_status[COMPUTE_IN_CH];
        for (uint16_t j = intermittent_status[COMPUTE_IN_CH]; j < in_channels; ++j) {
          DMA_makeTransfer(tmp_input_addr, lea_src_addr, lea_remain_size);

          fill_params.value = weight->data[tmp_weight_pos];

          msp_fill_q15(&fill_params, lea_flt);

          msp_mpy_q15(&mpy_params, lea_src, lea_flt, lea_flt);

          /* right shift the output to match up the fixed point format */
          msp_add_q15(&add_params, lea_flt, lea_flt, lea_flt);

          msp_add_q15(&add_params, lea_flt, lea_dst, lea_dst);

          /* save the results to intermittent_buffer */
          uint16_t next_j = j + 1;
          DOUBLE_BUFFER_TRANSFER(
            next_j, COMPUTE_IN_CH, lea_dst_addr, intermittent_mpy_buffer_addr,
            _LEA_REMAIN_SIZE
          );

          ++tmp_weight_pos;
          tmp_input_addr += output_addr_offset;
        }

        DOUBLE_BUFFER_TRANSFER_WITH_VAR_RESET(
          _LEA_REMAIN_SIZE, COMPUTE_IO_ROW, intermittent_status[COMPUTE_IN_CH],
          lea_dst_addr, output_fram_addr, _LEA_REMAIN_SIZE
        );

        input_channel_addr += remain_size_offset;
        output_fram_addr += remain_size_offset;
      }


      add_params.length = _LEA_DST_SIZE;
      fill_params.length = _LEA_DST_SIZE;
      mpy_params.length = _LEA_DST_SIZE;
      for (uint16_t m = intermittent_status[COMPUTE_IO_ROW]; m < output_len; m += _LEA_DST_SIZE) {
        if (intermittent_status[COMPUTE_IN_CH] == 0) {
          DMA_makeTransfer(output_fram_addr, lea_dst_addr, _LEA_DST_SIZE);
        }

        tmp_input_addr = input_channel_addr + \
          intermittent_status[COMPUTE_IN_CH] * output_addr_offset;
        tmp_weight_pos = weight_pos + intermittent_status[COMPUTE_IN_CH];
        for (uint16_t j = intermittent_status[COMPUTE_IN_CH]; j < in_channels; ++j) {
          DMA_makeTransfer(tmp_input_addr, lea_src_addr, _LEA_DST_SIZE);

          fill_params.value = weight->data[tmp_weight_pos];

          msp_fill_q15(&fill_params, lea_flt);

          msp_mpy_q15(&mpy_params, lea_src, lea_flt, lea_flt);

          /* right shift the output to match up the fixed point format */
          msp_add_q15(&add_params, lea_flt, lea_flt, lea_flt);

          msp_add_q15(&add_params, lea_flt, lea_dst, lea_dst);

          /* save the results to intermittent_buffer */
          uint16_t next_j = j + 1;
          DOUBLE_BUFFER_TRANSFER(
            next_j, COMPUTE_IN_CH, lea_dst_addr, intermittent_mpy_buffer_addr,
            _LEA_DST_SIZE
          );

          ++tmp_weight_pos;
          tmp_input_addr += output_addr_offset;
        }

        uint16_t next_m = m + _LEA_DST_SIZE;
        DOUBLE_BUFFER_TRANSFER_WITH_VAR_RESET(
          next_m, COMPUTE_IO_ROW, intermittent_status[COMPUTE_IN_CH],
          lea_dst_addr, output_fram_addr, _LEA_DST_SIZE
        );

        input_channel_addr += lea_min_size_offset;
        output_fram_addr += lea_min_size_offset;
      }

      uint16_t next_i = i + 1;
      DOUBLE_BUFFER_ASSIGN(next_i, COMPUTE_OUT_CH, 0, intermittent_status[COMPUTE_IO_ROW]);

      weight_pos += in_channels;
    }

    intermittent_status[COMPUTE_CK] = INTERMITTENT_features_features_1_pw_conv_linear_pw_conv_linear_0_Conv_EXIT;
  }

  if (intermittent_status[COMPUTE_CK] == INTERMITTENT_features_features_1_pw_conv_linear_pw_conv_linear_0_Conv_EXIT) {
    intermittent_status[COMPUTE_SWITCH] = PAD_START;
    intermittent_status[COMPUTE_IO_COL] = 0;
    intermittent_status[COMPUTE_IO_ROW] = 0;
    intermittent_status[COMPUTE_IN_CH] = 0;
    intermittent_status[COMPUTE_OUT_CH] = 0;
  }
}