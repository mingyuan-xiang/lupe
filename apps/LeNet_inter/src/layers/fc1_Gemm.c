#include <layers/include/utils.h>
#include <layers/include/fc1_Gemm.h>
#include <buffer/include/buffer.h>
#include <layers/include/intermittent.h>

#define _LEA_SRC_SIZE 16
#define _LEA_TMP_SIZE 1344

/* assign the lea buffer pointer */
#define lea_src (lea_buffer)
#define lea_tmp (lea_buffer + 16)
#define lea_dst (lea_buffer + 1360)


#define _OUTPUT_SIZE 84
#define _OUTPUT_SIZE_ALIGNED MAKE_ALIGN_2(_OUTPUT_SIZE)
#define _INPUT_SIZE 120
#define _MAT_SIZE (((uint16_t)_OUTPUT_SIZE_ALIGNED) * _INPUT_SIZE)
#define __MAT_BLOCK_SIZE ((_MAT_SIZE >= _LEA_TMP_SIZE) ? _LEA_TMP_SIZE : _MAT_SIZE)
#define __MAT_BLOCK_ROW_SIZE (((uint16_t)__MAT_BLOCK_SIZE) / _OUTPUT_SIZE_ALIGNED)
#define _MAT_BLOCK_ROW_SIZE ((__MAT_BLOCK_ROW_SIZE % 2) ? (__MAT_BLOCK_ROW_SIZE - 1) : __MAT_BLOCK_ROW_SIZE)
#define _MAT_BLOCK_SIZE (_MAT_BLOCK_ROW_SIZE * _OUTPUT_SIZE)
#define _MAT_BLOCK_ROW_SIZE_ALIGNED MAKE_ALIGN_2(_MAT_BLOCK_ROW_SIZE)
#define _MAT_REMAIN_ROW_SIZE (_INPUT_SIZE % _MAT_BLOCK_ROW_SIZE)
#define _MAT_REMAIN_BLOCK_SIZE (_MAT_REMAIN_ROW_SIZE * _OUTPUT_SIZE)
#define _MAT_REMAIN_ROW_SIZE_ALIGNED MAKE_ALIGN_2(_MAT_REMAIN_ROW_SIZE)
static inline __attribute__((always_inline)) void set_vector_matrix_mpy(uint16_t mat_row, uint16_t mat_col) {
  lea_vector_matrix_mpy_params->rowSize = mat_row;
  lea_vector_matrix_mpy_params->colSize = mat_col;
}

static inline __attribute__((always_inline)) void new_vector_matrix_mpy_q15(const _q15 *srcA, const _q15 *srcB, _q15 *dst) {
  lea_vector_matrix_mpy_params->colVector = MSP_LEA_CONVERT_ADDRESS(srcB);
  lea_vector_matrix_mpy_params->output = MSP_LEA_CONVERT_ADDRESS(dst);

  LEAPMS0 = MSP_LEA_CONVERT_ADDRESS(srcA);
  LEAPMS1 = MSP_LEA_CONVERT_ADDRESS(lea_vector_matrix_mpy_params);

  /* Invoke the LEACMD__MPYMATRIXROW command. */
  msp_lea_invokeCommand(LEACMD__MPYMATRIXROW);
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

void fc1_Gemm(mat_t* input, mat_t* output, mat_t* weight, mat_t* bias) {
  uint16_t output_size = output->dims[1];
  uint16_t input_size = input->dims[1];
  uint16_t block_row_size = _MAT_BLOCK_ROW_SIZE;
  uint16_t remain_block_size = _MAT_REMAIN_BLOCK_SIZE;
  uint16_t remain_row_size = _MAT_REMAIN_ROW_SIZE;
  uint16_t block_size = _MAT_BLOCK_SIZE;

  uintptr_t input_fram_addr = (uintptr_t)input->data;
  uintptr_t lea_tmp_addr = (uintptr_t)lea_tmp;
  uintptr_t lea_src_addr = (uintptr_t)lea_src;
  uintptr_t lea_dst_addr = (uintptr_t)lea_dst;
  uintptr_t block_row_offset = block_row_size * sizeof(int16_t);
  uintptr_t remain_block_offset = remain_block_size * sizeof(int16_t);
  uintptr_t block_offset = block_size * sizeof(int16_t);
  vector_matrix_mpy_init(_MAT_REMAIN_ROW_SIZE_ALIGNED, _OUTPUT_SIZE_ALIGNED);
  add_init(_OUTPUT_SIZE_ALIGNED);
  uint16_t input_vec_pos = 0;

  if (intermittent_status[COMPUTE_CK] == INTERMITTENT_LeNet_START) {
    if (intermittent_status[COMPUTE_IO_ROW] & DOUBLE_BUFFER_WRITE) {
      /* Start transferring to output buffer. Recover from temporary buffer */
      uint16_t line = intermittent_status[COMPUTE_IO_ROW] & DOUBLE_BUFFER_COMPLETE;
      DMA_makeTransfer((uintptr_t)intermittent_buffer, (uintptr_t)(output->data), output_size);

      intermittent_status[COMPUTE_IO_ROW] = line;
    }

    if (intermittent_status[COMPUTE_IO_ROW] > 0) {
      /* recover the results from output */
      DMA_makeTransfer((uintptr_t)(output->data), lea_dst_addr, output_size);
    } else { // intermittent_status[COMPUTE_IO_ROW] == 0
      /* add bias first so that we don't need to set the input to 0 */
      DMA_makeTransfer((uintptr_t)(bias->data), lea_dst_addr, output_size);
    }
    
    if (intermittent_status[COMPUTE_IO_ROW] == 0) {

    uintptr_t weight_fram_addr = (uintptr_t)weight->data;

    /* send weight matrix (remain) block */
    DMA_makeTransfer(weight_fram_addr, lea_tmp_addr, remain_block_size);
    weight_fram_addr += remain_block_offset;

    /* send input (remain) vector */
    lea_src[0] = input->data[input_vec_pos];
    input_vec_pos++;
    lea_src[1] = input->data[input_vec_pos];
    input_vec_pos++;
    lea_src[2] = input->data[input_vec_pos];
    input_vec_pos++;
    lea_src[3] = input->data[input_vec_pos];
    input_vec_pos++;
    lea_src[4] = input->data[input_vec_pos];
    input_vec_pos++;
    lea_src[5] = input->data[input_vec_pos];
    input_vec_pos++;
    lea_src[6] = input->data[input_vec_pos];
    input_vec_pos++;
    lea_src[7] = input->data[input_vec_pos];
    input_vec_pos++;
    
    new_vector_matrix_mpy_q15(lea_src, lea_tmp, lea_tmp);
    
    /* shift the results back to the corresponding q format */
    new_add_q15(lea_tmp, lea_tmp, lea_tmp);

    /* add the output to matrix multiplication results */
    new_add_q15(lea_tmp, lea_dst, lea_dst);

    /* save the results back to output */
    DMA_makeTransfer(lea_dst_addr, (uintptr_t)(output->data), output_size);

    /* No need to double buffering as we will always overwrite the output. */
    intermittent_status[COMPUTE_IO_ROW] = remain_row_size;
    
    }

    
    /* Compute the rest MAC */
    uintptr_t block_num = (intermittent_status[COMPUTE_IO_ROW] - remain_row_size) / block_row_size;
    uintptr_t weight_fram_addr = (uintptr_t)weight->data + (remain_block_offset + block_num * block_offset);
    input_fram_addr += intermittent_status[COMPUTE_IO_ROW] * sizeof(int16_t);
    set_vector_matrix_mpy(_MAT_BLOCK_ROW_SIZE_ALIGNED, _OUTPUT_SIZE_ALIGNED);
    
    for (uint16_t l = intermittent_status[COMPUTE_IO_ROW]; l < input_size; l += block_row_size) {
      /* send weight matrix block */
      DMA_makeTransfer(weight_fram_addr, lea_tmp_addr, block_size);
      weight_fram_addr += block_offset;

      /* send input vector */
      DMA_makeTransfer(input_fram_addr, lea_src_addr, block_row_size);
      input_fram_addr += block_row_offset;
      
      new_vector_matrix_mpy_q15(lea_src, lea_tmp, lea_tmp);
      
      /* shift the results back to the corresponding q format */
      new_add_q15(lea_tmp, lea_tmp, lea_tmp);

      /* add the output to matrix multiplication results */
      new_add_q15(lea_tmp, lea_dst, lea_dst);

      uint16_t next_l = l + block_row_size;
      DOUBLE_BUFFER_TRANSFER(next_l, COMPUTE_IO_ROW, lea_dst_addr, (uintptr_t)(output->data), output_size);
    }

    intermittent_status[COMPUTE_CK] = INTERMITTENT_fc1_Gemm_EXIT;
  }

  if (intermittent_status[COMPUTE_CK] == INTERMITTENT_fc1_Gemm_EXIT) {
    intermittent_status[COMPUTE_IO_ROW] = 0;
    add_clear();
    mac_clear();
  }
}