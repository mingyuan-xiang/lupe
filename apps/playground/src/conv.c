#include <include/utils.h>
#include <include/conv.h>
#include <include/intermittent.h>

#define _LEA_ADD_SIZE 100

#define _LEA_REMAIN_SIZE (100 % _LEA_ADD_SIZE)

#define _STRIDE_ROW_SIZE 1
#define _STRIDE_COL_SIZE 1

#define _INPUT_ROW_SIZE 14
#define _INPUT_COL_SIZE 14
#define _KERNEL_ROW_SIZE 5
#define _KERNEL_COL_SIZE 5
#define _OUTPUT_ROW_SIZE (((_INPUT_ROW_SIZE - _KERNEL_ROW_SIZE) / _STRIDE_ROW_SIZE) + 1)
#define _OUTPUT_COL_SIZE (((_INPUT_COL_SIZE - _KERNEL_COL_SIZE) / _STRIDE_COL_SIZE) + 1)
#define _KERNEL_SIZE_ALIGNED MAKE_ALIGN_2(_KERNEL_COL_SIZE)

#define _FIR_OVERLAP_SIZE ((_KERNEL_SIZE_ALIGNED - (_KERNEL_SIZE_ALIGNED - _KERNEL_COL_SIZE) - 1) / _STRIDE_COL_SIZE)


#define __LEA_SRC_SIZE 100
/* _LEA_SRC_SIZE will always be mutiple of 2 */
#define _LEA_SRC_SIZE (__LEA_SRC_SIZE - (_KERNEL_SIZE_ALIGNED - _KERNEL_COL_SIZE) * 2)
#define _FIR_TOTAL_SIZE (_OUTPUT_ROW_SIZE * _INPUT_COL_SIZE)
#define _FIR_INPUT_SIZE (_LEA_SRC_SIZE - (_LEA_SRC_SIZE % _INPUT_COL_SIZE))
#define _FIR_INPUT_REMAIN_SIZE (_FIR_TOTAL_SIZE % _FIR_INPUT_SIZE)

#define _FIR_OUTPUT_REMAIN_SIZE (_FIR_INPUT_REMAIN_SIZE - _KERNEL_COL_SIZE + 1)
#define _FIR_OUTPUT_REMAIN_SIZE_ALIGNED MAKE_ALIGN_2(_FIR_OUTPUT_REMAIN_SIZE)
#define _FIR_ADD_OUTPUT_REMAIN_SIZE ((_FIR_INPUT_REMAIN_SIZE / _INPUT_COL_SIZE) * _OUTPUT_COL_SIZE)
#define _FIR_ADD_OUTPUT_REMAIN_SIZE_ALIGNED MAKE_ALIGN_2(_FIR_ADD_OUTPUT_REMAIN_SIZE)

#define _FIR_OUTPUT_SIZE (_FIR_INPUT_SIZE - _KERNEL_COL_SIZE + 1)
#define _FIR_OUTPUT_SIZE_ALIGNED MAKE_ALIGN_2(_FIR_OUTPUT_SIZE)
#define _FIR_ADD_OUTPUT_SIZE ((_FIR_INPUT_SIZE / _INPUT_COL_SIZE) * _OUTPUT_COL_SIZE)
#define _FIR_ADD_OUTPUT_SIZE_ALIGNED MAKE_ALIGN_2(_FIR_ADD_OUTPUT_SIZE)



void conv(mat_t* input, mat_t* output, mat_t* weight, mat_t* bias) {
  uint16_t in_channels = input->dims[1];
  uint16_t out_channels = output->dims[1];
  uint16_t output_len = output->strides[1];
  uint16_t input_len = input->strides[1];
  uint16_t kernel_row_size = _KERNEL_ROW_SIZE;
  uint16_t kernel_col_size = _KERNEL_COL_SIZE;

  uintptr_t src_lea_addr = (uintptr_t)lea_src;
  uintptr_t dst_lea_addr = (uintptr_t)lea_dst;
  uintptr_t tmp_lea_addr = (uintptr_t)lea_tmp;

  uintptr_t flt_lea_addr = (uintptr_t)lea_flt;
  uintptr_t flt_addr_col_offset = kernel_col_size * sizeof(int16_t);
  uintptr_t flt_addr_padding_offset = (kernel_col_size + 1) * sizeof(int16_t);
  uintptr_t input_lea_addr = src_lea_addr;
  uintptr_t input_fram_addr = (uintptr_t)(input->data);
  uintptr_t input_channel_addr = input_fram_addr;
  uintptr_t input_channel_offset = input_len * sizeof(int16_t);
  uintptr_t output_addr_offset = output_len * sizeof(int16_t);
  
  uintptr_t lea_skip_addr = dst_lea_addr;
  uintptr_t lea_skip_addr_before_offset = (_OUTPUT_COL_SIZE + _FIR_OVERLAP_SIZE) * sizeof(int16_t);
  uintptr_t lea_skip_addr_before_st = lea_skip_addr + lea_skip_addr_before_offset;
  uintptr_t lea_skip_addr_before = lea_skip_addr_before_st;
  uintptr_t lea_skip_addr_after_offset = _OUTPUT_COL_SIZE * sizeof(int16_t);
  uintptr_t lea_skip_addr_after_st = lea_skip_addr + lea_skip_addr_after_offset;
  uintptr_t lea_skip_addr_after = lea_skip_addr_after_st;
  uintptr_t input_remain_offset = _FIR_INPUT_REMAIN_SIZE * _STRIDE_ROW_SIZE * sizeof(int16_t);
  uintptr_t output_remain_offset = _FIR_ADD_OUTPUT_REMAIN_SIZE * sizeof(int16_t);
  uintptr_t input_offset = _FIR_INPUT_SIZE * _STRIDE_ROW_SIZE * sizeof(int16_t);
  uintptr_t output_offset = _FIR_ADD_OUTPUT_SIZE * sizeof(int16_t);

  uint16_t input_line_size = input->dims[3];
  uint16_t input_line_size_offset = input_line_size * sizeof(int16_t);
  

  uint16_t lea_remain_size = _LEA_REMAIN_SIZE;
  uintptr_t output_lea_min_size_offset = _LEA_ADD_SIZE * sizeof(int16_t);
  uint16_t lea_remain_size_aligned = MAKE_ALIGN_2(lea_remain_size);
  
  
  msp_fir_q15_params conv_params = {
    .length = _FIR_OUTPUT_REMAIN_SIZE_ALIGNED,
    .tapLength = MAKE_ALIGN_2(kernel_col_size),
    .coeffs = lea_flt,
    .enableCircularBuffer = false 
  };
  msp_add_q15_params add_params = { .length = _FIR_OUTPUT_REMAIN_SIZE_ALIGNED };
  msp_offset_q15_params offset_params = { .length = lea_remain_size_aligned, .offset = 0 };

  uintptr_t intermittent_buffer_addr = (uintptr_t)intermittent_buffer;
  /*
   * We use the later half of intermittent_buffer to store the temporary FIR results.
   * This is safe as the FIR output will always be smaller than 1/3 of the entire intermittent_buffer.
   */
  uintptr_t intermittent_fir_buffer_addr = intermittent_buffer_addr + INTERMITTENT_BUFFER_SIZE;

  
  /* set the aligned position to be zeros */
  uint16_t lea_reset_pos = 0;
  for (uint16_t k = 0; k < kernel_row_size; ++k) {
    lea_flt[lea_reset_pos] = 0;
    lea_reset_pos += (kernel_col_size + 1);
  }

  /* convolution */
  if (intermittent_status[COMPUTE_CK] == INTERMITTENT_LeNet_i_START) {
    /* Recover loop variables */
    if (intermittent_status[COMPUTE_IO_ROW] & DOUBLE_BUFFER_WRITE) {
      uint16_t line = intermittent_status[COMPUTE_IO_ROW] & DOUBLE_BUFFER_COMPLETE;
      uint16_t size = _FIR_ADD_OUTPUT_REMAIN_SIZE;
      uintptr_t offset = 0;
      if (line > _FIR_INPUT_REMAIN_SIZE) {
        size = _FIR_ADD_OUTPUT_SIZE;
        offset = output_remain_offset + ((line - _FIR_INPUT_REMAIN_SIZE) / _FIR_INPUT_SIZE) * output_offset;
      }
      uintptr_t addr = intermittent_status[COMPUTE_OUT_CH] * output_addr_offset + offset;

      DMA_makeTransfer(intermittent_buffer_addr, addr, size);
      intermittent_status[COMPUTE_IO_COL] = 0;
      VOLATILE_WRITE(line, COMPUTE_IO_ROW);
    }

    if (intermittent_status[COMPUTE_IO_COL] & DOUBLE_BUFFER_WRITE) {
      uint16_t idx = intermittent_status[COMPUTE_IO_COL] & DOUBLE_BUFFER_COMPLETE;
      uint16_t size = _FIR_OUTPUT_REMAIN_SIZE;
      if (intermittent_status[COMPUTE_IO_ROW] > 0) {
        size = _FIR_OUTPUT_SIZE;
      }
      DMA_makeTransfer(intermittent_buffer_addr, intermittent_fir_buffer_addr, size);
      VOLATILE_WRITE(idx, COMPUTE_IO_COL);
    }

    if (intermittent_status[COMPUTE_IO_COL] > 0) {
      uint16_t size = _FIR_OUTPUT_REMAIN_SIZE;
      if (intermittent_status[COMPUTE_IO_ROW] > 0) {
        size = _FIR_OUTPUT_SIZE;
      }
      DMA_makeTransfer(intermittent_fir_buffer_addr, dst_lea_addr, size);
    }

    if (intermittent_status[COMPUTE_IO_ROW] >= _FIR_TOTAL_SIZE) {
      uint16_t next_idx = intermittent_status[COMPUTE_IN_CH] + 1;
      DOUBLE_BUFFER_ASSIGN(next_idx, COMPUTE_IN_CH, 0, intermittent_status[COMPUTE_IO_ROW]);
    }

    if (intermittent_status[COMPUTE_IN_CH] & DOUBLE_BUFFER_WRITE) {
      uint16_t idx = intermittent_status[COMPUTE_IN_CH] & DOUBLE_BUFFER_COMPLETE;
      VOLATILE_WRITE(intermittent_buffer[0], COMPUTE_IO_ROW);
      VOLATILE_WRITE(idx, COMPUTE_IN_CH);
    }
    
    if (intermittent_status[COMPUTE_IN_CH] >= in_channels) {
      uint16_t next_idx = intermittent_status[COMPUTE_OUT_CH] + 1;
      DOUBLE_BUFFER_ASSIGN(next_idx, COMPUTE_OUT_CH, 0, intermittent_status[COMPUTE_IN_CH]);
    }

    if (intermittent_status[COMPUTE_OUT_CH] & DOUBLE_BUFFER_WRITE) {
      uint16_t idx = intermittent_status[COMPUTE_OUT_CH] & DOUBLE_BUFFER_COMPLETE;
      VOLATILE_WRITE(intermittent_buffer[0], COMPUTE_IN_CH);
      VOLATILE_WRITE(idx, COMPUTE_OUT_CH);
    }

    uintptr_t output_fram_addr = (uintptr_t)(output->data) + \
      output_addr_offset * intermittent_status[COMPUTE_OUT_CH];
    uintptr_t flt_fram_addr = (uintptr_t)(weight->data) + \
      (intermittent_status[COMPUTE_OUT_CH] * weight->strides[0] + \
        intermittent_status[COMPUTE_IN_CH] * weight->strides[1]) * sizeof(int16_t);

    for (uint16_t i = intermittent_status[COMPUTE_OUT_CH]; i < out_channels; ++i) {
      input_fram_addr = (uintptr_t)(input->data) + \
        input_channel_offset * intermittent_status[COMPUTE_IN_CH];
      for (uint16_t j = intermittent_status[COMPUTE_IN_CH]; j < in_channels; ++j) {
        uintptr_t offset = 0;
        if (intermittent_status[COMPUTE_IO_ROW] != 0) {
          offset = output_remain_offset + \
            (intermittent_status[COMPUTE_IO_ROW] / _FIR_INPUT_SIZE) * output_offset;
        }
        uintptr_t tmp_output_addr = output_fram_addr + offset;
        input_channel_addr = input_fram_addr + \
          intermittent_status[COMPUTE_IO_ROW] * _STRIDE_ROW_SIZE * sizeof(int16_t);;
        uintptr_t tmp_input_addr = input_channel_addr + \
          input_line_size_offset * intermittent_status[COMPUTE_IO_COL];

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

  /*************************************************************
  * Do the reminder of FIR first
  ************************************************************/
        if (intermittent_status[COMPUTE_IO_ROW] == 0) {
          conv_params.length = _FIR_OUTPUT_REMAIN_SIZE_ALIGNED;
          add_params.length = _FIR_OUTPUT_REMAIN_SIZE_ALIGNED;

          lea_src[_FIR_INPUT_REMAIN_SIZE] = 0;
          lea_src[_FIR_INPUT_REMAIN_SIZE + 1] = 0;

          if (intermittent_status[COMPUTE_IO_COL] == 0) {
            conv_params.coeffs = lea_flt;

            /*
             * Do the FIR on the first row and save the results in lea_dst.
             * For the rest rows, save the FIR results in lea_tmp and add them to lea_dst.
             */

            /* send input to LEA RAM */
            DMA_makeTransfer(tmp_input_addr, input_lea_addr, _FIR_INPUT_REMAIN_SIZE);

            /* convolution */
            msp_fir_q15(&conv_params, lea_src, lea_dst);

            /* save results */
            DOUBLE_BUFFER_TRANSFER(
              1, COMPUTE_IO_COL, dst_lea_addr, intermittent_fir_buffer_addr,
              _FIR_OUTPUT_REMAIN_SIZE
            );

            tmp_input_addr += input_line_size_offset;
          }

          for (uint16_t k = intermittent_status[COMPUTE_IO_COL]; k < kernel_row_size; ++k) {
            conv_params.coeffs = lea_flt + intermittent_status[COMPUTE_IO_COL] * conv_params.tapLength;

            /* send input to LEA RAM */
            DMA_makeTransfer(tmp_input_addr, input_lea_addr, _FIR_INPUT_REMAIN_SIZE);

            /* convolution */
            msp_fir_q15(&conv_params, lea_src, lea_tmp);

            /* accumulate results for a 2D convolution */
            msp_add_q15(&add_params, lea_dst, lea_tmp, lea_dst);

            /* save results */
            uint16_t next_k = k + 1;
            DOUBLE_BUFFER_TRANSFER(
              next_k, COMPUTE_IO_COL, dst_lea_addr, intermittent_fir_buffer_addr,
              _FIR_OUTPUT_REMAIN_SIZE
            );

            tmp_input_addr += input_line_size_offset;
          }

          /* accumulate results with previous outputs */
          lea_skip_addr_before = lea_skip_addr_before_st;
          lea_skip_addr_after = lea_skip_addr_after_st;
          /* skip the garbage values between two lines */
          for (uint16_t l = _OUTPUT_COL_SIZE; l < _FIR_ADD_OUTPUT_REMAIN_SIZE; l += _OUTPUT_COL_SIZE) {
            DMA_makeTransfer(lea_skip_addr_before, lea_skip_addr_after, _OUTPUT_COL_SIZE);
            lea_skip_addr_before += lea_skip_addr_before_offset;
            lea_skip_addr_after += lea_skip_addr_after_offset;
          }

          /* send output to LEA RAM */
          DMA_makeTransfer(tmp_output_addr, tmp_lea_addr, _FIR_ADD_OUTPUT_REMAIN_SIZE);
          add_params.length = _FIR_ADD_OUTPUT_REMAIN_SIZE_ALIGNED;
          msp_add_q15(&add_params, lea_dst, lea_tmp, lea_dst);

          /* bring back output from LEA RAM */
          DOUBLE_BUFFER_TRANSFER_WITH_VAR_RESET(
            _FIR_INPUT_REMAIN_SIZE, COMPUTE_IO_ROW, intermittent_status[COMPUTE_IO_COL],
            dst_lea_addr, tmp_output_addr, _FIR_ADD_OUTPUT_REMAIN_SIZE
          );

          input_channel_addr += input_remain_offset;
          tmp_input_addr = input_channel_addr;
          tmp_output_addr += output_remain_offset;
        }

  /*************************************************************
  * Do the rest of FIR
  ************************************************************/

        conv_params.length = _FIR_OUTPUT_SIZE_ALIGNED;

        lea_src[_FIR_INPUT_SIZE] = 0;
        lea_src[_FIR_INPUT_SIZE + 1] = 0;

        for (uint16_t n = intermittent_status[COMPUTE_IO_ROW]; n < _FIR_TOTAL_SIZE; n += _FIR_INPUT_SIZE) {
          add_params.length = _FIR_OUTPUT_SIZE_ALIGNED;

          if (intermittent_status[COMPUTE_IO_COL] == 0) {
            conv_params.coeffs = lea_flt;

            /*
            * Do the FIR on the first row and save the results in lea_dst.
            * For the rest rows, save the FIR results in lea_tmp and add them to lea_dst.
            */

            /* send input to LEA RAM */
            DMA_makeTransfer(tmp_input_addr, input_lea_addr, _FIR_INPUT_SIZE);

            /* convolution */
            msp_fir_q15(&conv_params, lea_src, lea_dst);

            /* save results */
            DOUBLE_BUFFER_TRANSFER(
              1, COMPUTE_IO_COL, dst_lea_addr, intermittent_fir_buffer_addr,
              _FIR_OUTPUT_SIZE
            );


            tmp_input_addr += input_line_size_offset;
          }

          for (uint16_t k = intermittent_status[COMPUTE_IO_COL]; k < kernel_row_size; ++k) {
            conv_params.coeffs = lea_flt + intermittent_status[COMPUTE_IO_COL] * conv_params.tapLength;

            /* send input to LEA RAM */
            DMA_makeTransfer(tmp_input_addr, input_lea_addr, _FIR_INPUT_SIZE);

            /* convolution */
            msp_fir_q15(&conv_params, lea_src, lea_tmp);

            /* accumulate results for a 2D convolution */
            msp_add_q15(&add_params, lea_dst, lea_tmp, lea_dst);

            /* save results */
            uint16_t next_k = k + 1;
            DOUBLE_BUFFER_TRANSFER(
              next_k, COMPUTE_IO_COL, dst_lea_addr, intermittent_fir_buffer_addr,
              _FIR_OUTPUT_SIZE
            );

            tmp_input_addr += input_line_size_offset;
          }

          /* accumulate results with previous outputs */
          lea_skip_addr_before = lea_skip_addr_before_st;
          lea_skip_addr_after = lea_skip_addr_after_st;
          /* skip the garbage values between two lines */
          for (uint16_t l = _OUTPUT_COL_SIZE; l < _FIR_ADD_OUTPUT_SIZE; l += _OUTPUT_COL_SIZE) {
            DMA_makeTransfer(lea_skip_addr_before, lea_skip_addr_after, _OUTPUT_COL_SIZE);
            lea_skip_addr_before += lea_skip_addr_before_offset;
            lea_skip_addr_after += lea_skip_addr_after_offset;
          }

          /* send output to LEA RAM */
          DMA_makeTransfer(tmp_output_addr, tmp_lea_addr, _FIR_ADD_OUTPUT_SIZE);
          add_params.length = _FIR_ADD_OUTPUT_SIZE_ALIGNED;
          msp_add_q15(&add_params, lea_dst, lea_tmp, lea_dst);

          /* bring back output from LEA RAM */
          uint16_t next_n = n + _FIR_INPUT_SIZE;
          DOUBLE_BUFFER_TRANSFER_WITH_VAR_RESET(
            next_n, COMPUTE_IO_ROW, intermittent_status[COMPUTE_IO_COL],
            dst_lea_addr, tmp_output_addr, _FIR_ADD_OUTPUT_SIZE
          );


          input_channel_addr += input_offset;
          tmp_input_addr = input_channel_addr;
          tmp_output_addr += output_offset;
        }
        uint16_t next_j = j + 1;
        DOUBLE_BUFFER_ASSIGN(next_j, COMPUTE_IN_CH, 0, intermittent_status[COMPUTE_IO_ROW]);

        input_fram_addr += input_channel_offset;
      }
      uint16_t next_i = i + 1;
      DOUBLE_BUFFER_ASSIGN(next_i, COMPUTE_OUT_CH, 0, intermittent_status[COMPUTE_IN_CH]);

      output_fram_addr += output_addr_offset;
    }

    VOLATILE_WRITE(INTERMITTENT_conv2_Conv_EXIT, COMPUTE_CK);
  }

  if (intermittent_status[COMPUTE_CK] == INTERMITTENT_conv2_Conv_EXIT) {
    intermittent_status[COMPUTE_SWITCH] = PAD_START;
    intermittent_status[COMPUTE_IO_COL] = 0;
    intermittent_status[COMPUTE_IO_ROW] = 0;
    intermittent_status[COMPUTE_IN_CH] = 0;
    intermittent_status[COMPUTE_OUT_CH] = 0;
  }
}