#include <layers/include/utils.h>
#include <layers/include/conv1_Conv.h>
#include <buffer/include/buffer.h>


#define _LEA_ADD_SIZE 400

#define _OUTPUT_SIZE 784
#define _LEA_REMAIN_SIZE (_OUTPUT_SIZE % _LEA_ADD_SIZE)
#define _PADDING_TOP 2
#define _PADDING_BOTTOM 2
#define _PADDING_RIGHT 2
#define _PADDING_LEFT 2

#define _INPUT_LINE_SIZE 32

#define _KERNEL_SIZE_ALIGNED 6
#define _KERNEL_ROW_SIZE 5
#define _KERNEL_COL_SIZE 5

void conv1_Conv(mat_t* input, mat_t* output, mat_t* weight, mat_t* bias) {
  uint16_t in_channels = input->dims[1];
  uint16_t out_channels = output->dims[1];
  uint16_t output_len = output->strides[1];
  uint16_t input_len = input->strides[1];
  uint16_t kernel_row_size = _KERNEL_ROW_SIZE;
  uint16_t kernel_col_size = _KERNEL_COL_SIZE;

  uintptr_t src_lea_addr = (uintptr_t)lea_src;
  uintptr_t dst_lea_addr = (uintptr_t)lea_dst;

  uintptr_t flt_lea_addr = (uintptr_t)lea_flt;
  uintptr_t flt_addr_col_offset = kernel_col_size * sizeof(int16_t);
  uintptr_t flt_addr_padding_offset = (kernel_col_size + 1) * sizeof(int16_t);
  uintptr_t input_lea_addr = src_lea_addr;
  uintptr_t input_fram_addr = (uintptr_t)(input->data);
  uintptr_t input_channel_addr = input_fram_addr;
  uintptr_t input_channel_offset = input_len * sizeof(int16_t);
  uintptr_t output_lea_addr = dst_lea_addr;
  uintptr_t output_addr_offset = output_len * sizeof(int16_t);

  int16_t* conv_flt = lea_flt;
  uint16_t input_line_size = input->dims[3];
  uint16_t output_line_size = output->dims[3];
  uint16_t output_line_num = output->dims[2];
  uint16_t output_line_size_offset = output_line_size * sizeof(int16_t);
  uint16_t input_line_size_offset = input_line_size * sizeof(int16_t);

  uint16_t lea_remain_size = _LEA_REMAIN_SIZE;
  uintptr_t output_remain_size_offset = lea_remain_size * sizeof(int16_t);
  uintptr_t output_lea_min_size_offset = _LEA_ADD_SIZE * sizeof(int16_t);
  uint16_t lea_remain_size_aligned = MAKE_ALIGN_2(lea_remain_size);

  uintptr_t inter_buffer_addr = (uintptr_t)intermittent_buffer;
  
  msp_fir_q15_params conv_params = {
    .length = MAKE_ALIGN_2(input_line_size - kernel_col_size + 1),
    .tapLength = MAKE_ALIGN_2(kernel_col_size),
    .coeffs = lea_flt,
    .enableCircularBuffer = false 
  };
  msp_add_q15_params add_params = { .length = MAKE_ALIGN_2(output_line_size) };
  msp_offset_q15_params offset_params = { .length = lea_remain_size_aligned, .offset = 0 };

  
  /* set the aligned position to be zeros */
  uint16_t lea_reset_pos = 0;
  for (uint16_t k = 0; k < kernel_row_size; ++k) {
    lea_flt[lea_reset_pos] = 0;
    lea_reset_pos += (kernel_col_size + 1);
  }

  lea_src[_INPUT_LINE_SIZE] = 0;
  lea_src[_INPUT_LINE_SIZE + 1] = 0;

  /* Pad the input for (2, 2, 2, 2) */
  switch (intermittent_status[CONV_PAD]) {
  case 0:
    memset(output->data, 0, GET_MAT_SIZE(input)*sizeof(int16_t));
    intermittent_status[CONV_PAD]++;
  case 1:
    if (intermittent_status[CONV_ROW] >= input_line_num) {
      intermittent_status[CONV_ROW] = 0;
      intermittent_status[CONV_IN_CH]++;
    }

    uint16_t input_line_num = input->dims[2] - _PADDING_TOP - _PADDING_BOTTOM;
    uint16_t input_line_size_bf = input->dims[3] - _PADDING_RIGHT - _PADDING_LEFT;
    uint16_t input_len_bf = input_line_num * input_line_size_bf;
    _q15* padding_ptr_in = input->data + \
      intermittent_status[CONV_IN_CH] * input_len_bf + \
      intermittent_status[CONV_ROW] * input_line_size_bf;
    _q15* padding_ptr_out = output->data + \
      intermittent_status[CONV_IN_CH] * input_len + \
      intermittent_status[CONV_ROW] * input_line_size;

    for (uint16_t i = intermittent_status[CONV_IN_CH]; i < in_channels; ++i) {
      padding_ptr_out += input_line_size;
      padding_ptr_out += input_line_size;
      for (uint16_t j = intermittent_status[CONV_ROW]; j < input_line_num; ++j) {
        padding_ptr_out += _PADDING_LEFT;
        DMA_makeTransfer((uintptr_t)padding_ptr_in, (uintptr_t)padding_ptr_out, input_line_size_bf);
        padding_ptr_in += input_line_size_bf;
        padding_ptr_out += (_PADDING_RIGHT + input_line_size_bf);

        intermittent_status[CONV_ROW]++;
      }
      padding_ptr_out += input_line_size;
      padding_ptr_out += input_line_size;

      intermittent_status[CONV_ROW] = 0;
      intermittent_status[CONV_IN_CH]++;
    }
    intermittent_status[CONV_PAD]++;
  case 2:
  DMA_makeTransfer(output_fram_addr, input_fram_addr, GET_MAT_SIZE(input));
  intermittent_status[CONV_PAD]++;
  case 3:
  memset(output->data, 0, GET_MAT_SIZE(output)*sizeof(int16_t));
  case 4:
  intermittent_status[CONV_IN_CH] = 0;
  default:
  }
  
  if (intermittent_status[COMPUTE_CK] == COMPUTE_WEIGHT) {
    /* convolution */
    /* Recover loop variables */
    if (intermittent_status[CONV_K_ROW] >= kernel_row_size) {
      if (intermittent_status[BUFFER_COMMIT] == DOUBLE_BUFFER_COMPLETE) {
        /* Complete double buffering. No need for recovery */
        intermittent_status[CONV_K_ROW] = 0;
        intermittent_status[CONV_IO_ROW]++;
      } else if (intermittent_status[BUFFER_COMMIT] == DOUBLE_BUFFER_TMP) {
        /* Start transferring to tmp buffer, but get interrupted. So, we need to recompute. */
        intermittent_status[CONV_K_ROW]--;
      } else {
        /* Start transferring to output buffer. Recover from tmp buffer */
        uintptr_t addr = (uintptr_t)(output->data) + \
          intermittent_status[CONV_OUT_CH] * output_len + \
          intermittent_status[CONV_IO_ROW] * output_line_size;
        DMA_makeTransfer(inter_buffer_addr, addr, output_line_size);
        intermittent_status[CONV_K_ROW] = 0;
        intermittent_status[CONV_IO_ROW]++;
      }
    }
    if (intermittent_status[CONV_IO_ROW] >= output_line_num) {
      intermittent_status[CONV_IO_ROW] = 0;
      intermittent_status[CONV_IN_CH]++;
    }
    if (intermittent_status[CONV_IN_CH] >= in_channels) {
      intermittent_status[CONV_IN_CH] = 0;
      intermittent_status[CONV_OUT_CH]++;
    }

    uintptr_t output_fram_addr = (uintptr_t)(output->data) + \
      intermittent_status[CONV_OUT_CH] * output_addr_offset;
    uintptr_t flt_fram_addr = (uintptr_t)(weight->data) + \
      intermittent_status[CONV_OUT_CH] * weight->strides[0] + \
      intermittent_status[CONV_IN_CH] * weight->strides[1];

    for (i = intermittent_status[CONV_OUT_CH]; i < out_channels; ++i) {
      input_fram_addr = (uintptr_t)(input->data) + \
        intermittent_status[CONV_IN_CH] * input_channel_offset;

      for (uint16_t j = intermittent_status[CONV_IN_CH]; j < in_channels; ++j) {
        uintptr_t tmp_output_addr = output_fram_addr + \
          intermittent_status[CONV_IO_ROW] * output_line_size_offset;
        input_channel_addr = input_fram_addr;

        /* send kernel to LEA RAM */
        /*
        * pad zero to the beginning of the filter if the filter's size is
        * not aligned to 2
        */
        flt_lea_addr += sizeof(uint16_t);

        for (uint16_t k = 0; k < kernel_row_size; ++k) {
          DMA_makeTransfer(flt_fram_addr, flt_lea_addr, kernel_col_size);
          flt_lea_addr += flt_addr_padding_offset;
          flt_fram_addr += flt_addr_col_offset;
        }
        /* restore flt_lea_addr pointer to the beginning of the array */
        flt_lea_addr = (uintptr_t)lea_flt;

        for (uint16_t l = intermittent_status[CONV_IO_ROW]; l < output_line_num; ++l) {
          uintptr_t tmp_input_addr = input_channel_addr + \
            intermittent_status[CONV_K_ROW] * input_line_size_offset;

          /* send output to LEA RAM */
          DMA_makeTransfer(tmp_output_addr, output_lea_addr, output_line_size);

          conv_flt = lea_flt + intermittent_status[CONV_K_ROW] * conv_params.tapLength;

          for (uint16_t k = intermittent_status[CONV_K_ROW]; k < kernel_row_size; ++k) {
            /* send input to LEA RAM */
            DMA_makeTransfer(tmp_input_addr, input_lea_addr, input_line_size);
            conv_params.coeffs = conv_flt;

            /* convolution */
            msp_fir_q15(&conv_params, lea_src, lea_tmp);

            /* accumulate results for a 2D convolution */
            msp_add_q15(&add_params, lea_dst, lea_tmp, lea_dst);
            conv_flt += conv_params.tapLength;
            tmp_input_addr += input_line_size_offset;

            intermittent_status[CONV_K_ROW]++;
          }

          /* bring back output from LEA RAM */
          intermittent_status[BUFFER_COMMIT] = DOUBLE_BUFFER_TMP;
          DMA_makeTransfer(output_lea_addr, inter_buffer_addr, output_line_size);
          intermittent_status[BUFFER_COMMIT] = DOUBLE_BUFFER_FINAL;
          DMA_makeTransfer(output_lea_addr, tmp_output_addr, output_line_size);
          intermittent_status[BUFFER_COMMIT] = DOUBLE_BUFFER_COMPLETE;

          tmp_output_addr += output_line_size_offset;
          input_channel_addr += input_line_size_offset;

          intermittent_status[CONV_K_ROW] = 0;
          intermittent_status[CONV_IO_ROW]++;
        }
        input_fram_addr += input_channel_offset;

        intermittent_status[CONV_IO_ROW] = 0;
        intermittent_status[CONV_IN_CH]++;
      }
      output_fram_addr += output_addr_offset;

      intermittent_status[CONV_IN_CH] = 0;
      intermittent_status[CONV_OUT_CH]++;
    }

    intermittent_status[COMPUTE_CK] = COMPUTE_BIAS;
  }

  if (intermittent_status[COMPUTE_CK] == COMPUTE_BIAS) {
    /* add bias and left shift */
    /* Recover loop variables. We use CONV_IN_CH as it will be zero when the start of this */
    output_fram_addr = (uintptr_t)(output->data);

    _q15* lea_add = lea_src;
    uint16_t add_size = _LEA_ADD_SIZE;
    uintptr_t lea_add_addr = (uintptr_t)lea_add;
    for (uint16_t i = intermittent_status[CONV_IN_CH]; i < out_channels; ++i) {
      switch (intermittent_status[CONV_IO_ROW])
      offset_params.offset = bias->data[i];
      offset_params.length = lea_remain_size_aligned;
      add_params.length = lea_remain_size_aligned;

      DMA_makeTransfer(output_fram_addr, lea_add_addr, lea_remain_size);

      msp_add_q15(&add_params, lea_add, lea_add, lea_add);
      msp_offset_q15(&offset_params, lea_add, lea_add);

      DMA_makeTransfer(lea_add_addr, output_fram_addr, lea_remain_size);
      output_fram_addr += output_remain_size_offset;
      offset_params.length = add_size;
      add_params.length = add_size;

      for (uint16_t j = lea_remain_size; j < output_len; j += add_size) {
        DMA_makeTransfer(output_fram_addr, lea_add_addr, add_size);

        msp_add_q15(&add_params, lea_add, lea_add, lea_add);
        msp_offset_q15(&offset_params, lea_add, lea_add);

        DMA_makeTransfer(lea_add_addr, output_fram_addr, add_size);
        output_fram_addr += output_lea_min_size_offset;
      }
    }
  }

  intermittent_status[CONV_PAD] = 0;
  intermittent_status[CONV_IO_ROW] = 0;
  intermittent_status[CONV_IN_CH] = 0;
  intermittent_status[CONV_OUT_CH] = 0;
}