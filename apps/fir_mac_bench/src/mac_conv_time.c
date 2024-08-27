#include <include/utils.h>
#include <include/mac_conv_time.h>
#include <include/data.h>
#include <libmspsyncioutils/mspsyncioutils.h>
#include <libmsptimer/timekeeper.h>

#define _LEA_ADD_SIZE 400
#define _LEA_BIAS_ADD_SIZE 400

static msp_status msp_add_q15_time(const msp_add_q15_params *params, const _q15 *srcA, const _q15 *srcB, _q15 *dst, uint16_t repeat, uint32_t* t) {
    uint32_t time = 0;
    uint16_t length;
    msp_status status;
    MSP_LEA_ADDMATRIX_PARAMS *leaParams;

    /* Initialize the vector length. */
    length = params->length;

    /* Check that length parameter is a multiple of two. */
    if (length & 1) {
        return MSP_SIZE_ERROR;
    }
    
    /* Check that the data arrays are aligned and in a valid memory segment. */
    if (!(MSP_LEA_VALID_ADDRESS(srcA, 4) &
          MSP_LEA_VALID_ADDRESS(srcB, 4) &
          MSP_LEA_VALID_ADDRESS(dst, 4))) {
        return MSP_LEA_INVALID_ADDRESS;
    }

    /* Acquire lock for LEA module. */
    if (!msp_lea_acquireLock()) {
        return MSP_LEA_BUSY;
    }

    /* Initialize LEA if it is not enabled. */
    if (!(LEAPMCTL & LEACMDEN)) {
        msp_lea_init();
    }
        
    /* Allocate MSP_LEA_ADDMATRIX_PARAMS structure. */
    leaParams = (MSP_LEA_ADDMATRIX_PARAMS *)msp_lea_allocMemory(sizeof(MSP_LEA_ADDMATRIX_PARAMS)/sizeof(uint32_t));

    /* Set MSP_LEA_ADDMATRIX_PARAMS structure. */
    leaParams->vectorSize = length;
    leaParams->input1Offset = 1;
    leaParams->input2Offset = 1;
    leaParams->outputOffset = 1;

    /* Load source arguments to LEA. */
    start_timer();
    for (uint16_t r = 0; r < repeat; r++) {
      leaParams->input2 = MSP_LEA_CONVERT_ADDRESS(srcB);
      leaParams->output = MSP_LEA_CONVERT_ADDRESS(dst);

      LEAPMS0 = MSP_LEA_CONVERT_ADDRESS(srcA);
      LEAPMS1 = MSP_LEA_CONVERT_ADDRESS(leaParams);

      /* Invoke the LEACMD__ADDMATRIX command. */
      msp_lea_invokeCommand(LEACMD__ADDMATRIX);
    }
    time += stop_timer();

    /* Free MSP_LEA_ADDMATRIX_PARAMS structure. */
    msp_lea_freeMemory(sizeof(MSP_LEA_ADDMATRIX_PARAMS)/sizeof(uint32_t));
    
    /* Set status flag. */
    status = MSP_SUCCESS;
        
    /* Check LEA interrupt flags for any errors. */
    if (msp_lea_ifg & LEACOVLIFG) {
        status = MSP_LEA_COMMAND_OVERFLOW;
    }
    else if (msp_lea_ifg & LEAOORIFG) {
        status = MSP_LEA_OUT_OF_RANGE;
    }
    else if (msp_lea_ifg & LEASDIIFG) {
        status = MSP_LEA_SCALAR_INCONSISTENCY;
    }

    /* Free lock for LEA module and return status. */
    msp_lea_freeLock();

    *t += time;
    return status;
}

static msp_status msp_offset_q15_time(const msp_offset_q15_params *params, const _q15 *src, _q15 *dst, uint16_t repeat, uint32_t* t) {
    uint32_t time = 0;
    uint16_t length;
    _q15 q15Offset;
    int16_t *offsetVector;
    msp_status status;
    MSP_LEA_ADDMATRIX_PARAMS *leaParams;
    
    /* Initialize the loop counter and offset. */
    length = params->length;
    q15Offset = params->offset;

    /* Check that length parameter is a multiple of two. */
    if (length & 1) {
        return MSP_SIZE_ERROR;
    }
    
    /* Check that the data arrays are aligned and in a valid memory segment. */
    if (!(MSP_LEA_VALID_ADDRESS(src, 4) &
          MSP_LEA_VALID_ADDRESS(dst, 4))) {
        return MSP_LEA_INVALID_ADDRESS;
    }

    /* Acquire lock for LEA module. */
    if (!msp_lea_acquireLock()) {
        return MSP_LEA_BUSY;
    }

    /* Initialize LEA if it is not enabled. */
    if (!(LEAPMCTL & LEACMDEN)) {
        msp_lea_init();
    }
        
    /* Allocate MSP_LEA_ADDMATRIX_PARAMS structure. */
    leaParams = (MSP_LEA_ADDMATRIX_PARAMS *)msp_lea_allocMemory(sizeof(MSP_LEA_ADDMATRIX_PARAMS)/sizeof(uint32_t));
        
    /* Allocate offset vector of length two. */
    offsetVector = (int16_t *)msp_lea_allocMemory(2*sizeof(int16_t)/sizeof(uint32_t));
    offsetVector[0] = q15Offset;
    offsetVector[1] = q15Offset;

    /* Set MSP_LEA_ADDMATRIX_PARAMS structure. */
    leaParams->input2 = MSP_LEA_CONVERT_ADDRESS(offsetVector);
    leaParams->vectorSize = length;
    leaParams->input1Offset = 1;
    leaParams->input2Offset = 0;
    leaParams->outputOffset = 1;

    start_timer();
    for (uint16_t r = 0; r < repeat; r++) {
      leaParams->output = MSP_LEA_CONVERT_ADDRESS(dst);

      /* Load source arguments to LEA. */
      LEAPMS0 = MSP_LEA_CONVERT_ADDRESS(src);
      LEAPMS1 = MSP_LEA_CONVERT_ADDRESS(leaParams);

      /* Invoke the LEACMD__ADDMATRIX command. */
      msp_lea_invokeCommand(LEACMD__ADDMATRIX);
    }
    time += stop_timer();

    /* Free MSP_LEA_ADDMATRIX_PARAMS structure and offset vector. */
    msp_lea_freeMemory(2*sizeof(int16_t)/sizeof(uint32_t));
    msp_lea_freeMemory(sizeof(MSP_LEA_ADDMATRIX_PARAMS)/sizeof(uint32_t));
    
    /* Set status flag. */
    status = MSP_SUCCESS;
        
    /* Check LEA interrupt flags for any errors. */
    if (msp_lea_ifg & LEACOVLIFG) {
        status = MSP_LEA_COMMAND_OVERFLOW;
    }
    else if (msp_lea_ifg & LEAOORIFG) {
        status = MSP_LEA_OUT_OF_RANGE;
    }
    else if (msp_lea_ifg & LEASDIIFG) {
        status = MSP_LEA_SCALAR_INCONSISTENCY;
    }

    /* Free lock for LEA module and return status. */
    msp_lea_freeLock();

    *t += time;
    return status;
}

static msp_status msp_mac_q15_time(const msp_mac_q15_params *params, const _q15 *srcA, const _q15 *srcB, _iq31 *result, uint16_t repeat, uint32_t* t) {
    uint32_t time = 0;
    uint16_t length;
    msp_status status;
    MSP_LEA_MAC_PARAMS *leaParams;
    
    /* Initialize the loop counter with the vector length. */
    length = params->length;

    /* Check that length parameter is a multiple of two. */
    if (length & 1) {
        return MSP_SIZE_ERROR;
    }

    /* Check that the data arrays are aligned and in a valid memory segment. */
    if (!(MSP_LEA_VALID_ADDRESS(srcA, 4) &
          MSP_LEA_VALID_ADDRESS(srcB, 4) &
          MSP_LEA_VALID_ADDRESS(result, 4))) {
        return MSP_LEA_INVALID_ADDRESS;
    }

    /* Acquire lock for LEA module. */
    if (!msp_lea_acquireLock()) {
        return MSP_LEA_BUSY;
    }

    /* Initialize LEA if it is not enabled. */
    if (!(LEAPMCTL & LEACMDEN)) {
        msp_lea_init();
    }
        
    /* Allocate MSP_LEA_MAC_PARAMS structure. */
    leaParams = (MSP_LEA_MAC_PARAMS *)msp_lea_allocMemory(sizeof(MSP_LEA_MAC_PARAMS)/sizeof(uint32_t));

    /* Set MSP_LEA_MAC_PARAMS structure. */
    leaParams->output = MSP_LEA_CONVERT_ADDRESS(result);
    leaParams->vectorSize = length;

    start_timer();
    for (uint16_t r = 0; r < repeat; r++) {
      leaParams->input2 = MSP_LEA_CONVERT_ADDRESS(srcB);

      /* Load source arguments to LEA. */
      LEAPMS0 = MSP_LEA_CONVERT_ADDRESS(srcA);
      LEAPMS1 = MSP_LEA_CONVERT_ADDRESS(leaParams);

      /* Invoke the LEACMD__MAC command. */
      msp_lea_invokeCommand(LEACMD__MAC);
    }
    time += stop_timer();

    /* Free MSP_LEA_MAC_PARAMS structure. */
    msp_lea_freeMemory(sizeof(MSP_LEA_MAC_PARAMS)/sizeof(uint32_t));
    
    /* Set status flag. */
    status = MSP_SUCCESS;
        
    /* Check LEA interrupt flags for any errors. */
    if (msp_lea_ifg & LEACOVLIFG) {
        status = MSP_LEA_COMMAND_OVERFLOW;
    }
    else if (msp_lea_ifg & LEAOORIFG) {
        status = MSP_LEA_OUT_OF_RANGE;
    }
    else if (msp_lea_ifg & LEASDIIFG) {
        status = MSP_LEA_SCALAR_INCONSISTENCY;
    }

    /* Free lock for LEA module and return status. */
    msp_lea_freeLock();

    *t += time;
    return status;
}

void mac_conv_time(mat_t* input, mat_t* output, mat_t* weight, mat_t* bias, uint16_t repeat, uint32_t* t) {
  uint16_t in_channels = input->dims[1];
  uint16_t out_channels = output->dims[1];
  uint16_t output_len = output->strides[1];
  uint16_t input_len = input->strides[1];

  uint16_t _OUTPUT_SIZE = output->strides[1];
  uint16_t _LEA_REMAIN_SIZE = _OUTPUT_SIZE % _LEA_ADD_SIZE;

  uint16_t _STRIDE_ROW_SIZE = 1;
  uint16_t _STRIDE_COL_SIZE = 1;
  uint16_t _KERNEL_ROW_SIZE = weight->dims[2];
  uint16_t _KERNEL_COL_SIZE = weight->dims[3];
  uint16_t _FLT_LEN = (_KERNEL_ROW_SIZE * _KERNEL_COL_SIZE);
  uint16_t _LEA_BIAS_REMAIN_SIZE = _OUTPUT_SIZE % _LEA_BIAS_ADD_SIZE;

  uint16_t kernel_row_size = _KERNEL_ROW_SIZE;
  uint16_t kernel_col_size = _KERNEL_COL_SIZE;

  uintptr_t flt_lea_addr = (uintptr_t)lea_flt;
  uintptr_t flt_fram_addr = (uintptr_t)(weight->data);
  uintptr_t flt_addr_offset = _FLT_LEN * sizeof(int16_t);
  uintptr_t flt_channel_offset = weight->strides[0] * sizeof(int16_t);
  uintptr_t flt_addr_col_offset = kernel_col_size * sizeof(int16_t);
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
  uintptr_t output_lea_min_size_offset = _LEA_ADD_SIZE * sizeof(int16_t);

  uint16_t lea_bias_remain_size = _LEA_BIAS_REMAIN_SIZE;
  uintptr_t output_bias_remain_size_offset = lea_bias_remain_size * sizeof(int16_t);
  uintptr_t output_lea_bias_min_size_offset = _LEA_BIAS_ADD_SIZE * sizeof(int16_t);
  uint16_t lea_remain_bias_size_aligned = MAKE_ALIGN_2(lea_bias_remain_size);

  uintptr_t src_lea_addr = (uintptr_t)lea_src;
  uintptr_t dst_lea_addr = (uintptr_t)lea_dst;

  uint32_t time = 0;
  
  /* set the aligned position to be zeros */
  lea_src[_FLT_LEN] = 0;
  lea_flt[_FLT_LEN] = 0;
  msp_mac_q15_params mac_params = { .length = MAKE_ALIGN_2(_FLT_LEN) };
  msp_add_q15_params add_params = { .length = MAKE_ALIGN_2(_LEA_ADD_SIZE) };
  msp_offset_q15_params offset_params = { .length = lea_remain_bias_size_aligned, .offset = 0 };

  memset(output->data, 0, GET_MAT_SIZE(output)*sizeof(int16_t));

  /* convolution */
  for (uint16_t i = 0; i < in_channels; ++i) {
    uintptr_t output_channel_addr = output_fram_addr;
    uintptr_t flt_tmp_addr = flt_fram_addr;
    uint16_t tmp_input_addr_offset = sizeof(int16_t) * _STRIDE_COL_SIZE;

    /* assemble input to a matrix in mac_buffer */
    uintptr_t mac_buffer_addr = (uintptr_t)data_meta.data;
    input_channel_addr = input_fram_addr;
    for (uint16_t m = 0; m < output_line_num; ++m) {
      uintptr_t tmp_input_addr = input_channel_addr;
      for (uint16_t n = 0; n < output_line_size; ++n) {
        uintptr_t tmp_input_row_addr = tmp_input_addr;
        for (uint16_t k = 0; k < kernel_row_size; ++k) {
          DMA_makeTransfer(tmp_input_row_addr, mac_buffer_addr, kernel_col_size);

          tmp_input_row_addr += input_line_size_offset;
          mac_buffer_addr += flt_addr_col_offset;
        }
        tmp_input_addr += tmp_input_addr_offset;
      }
      input_channel_addr += input_line_size_strided_offset;
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
        msp_mac_q15_time(&mac_params, lea_src, lea_flt, lea_res, repeat, &time);
        lea_tmp[l] = (int16_t)(lea_res[0] >> 16);

        mac_input_addr += flt_addr_offset;
      }
      add_params.length = lea_remain_size_aligned;
      DMA_makeTransfer(tmp_output_addr, dst_lea_addr, lea_remain_size);
      msp_add_q15_time(&add_params, lea_dst, lea_tmp, lea_dst, repeat, &time);
      DMA_makeTransfer(dst_lea_addr, tmp_output_addr, lea_remain_size);
      tmp_output_addr += output_remain_size_offset;
      add_params.length = _LEA_ADD_SIZE;

      for (uint16_t m = lea_remain_size; m < output_len; m += _LEA_ADD_SIZE) {
        for (uint16_t l = 0; l < _LEA_ADD_SIZE; ++l) {
          DMA_makeTransfer(mac_input_addr, src_lea_addr, _FLT_LEN);
          msp_mac_q15_time(&mac_params, lea_src, lea_flt, lea_res, repeat, &time);
          lea_tmp[l] = (int16_t)(lea_res[0] >> 16);

          mac_input_addr += flt_addr_offset;
        }

        DMA_makeTransfer(tmp_output_addr, dst_lea_addr, _LEA_ADD_SIZE);
        msp_add_q15_time(&add_params, lea_dst, lea_tmp, lea_dst, repeat, &time);

        DMA_makeTransfer(dst_lea_addr, tmp_output_addr, _LEA_ADD_SIZE);

        tmp_output_addr += output_lea_min_size_offset;
      }

      flt_tmp_addr += flt_channel_offset;
      output_channel_addr += output_addr_offset;
    }
    input_fram_addr += input_channel_offset;
    flt_fram_addr += flt_addr_offset;
  }

  /* add bias and left shift */
  output_fram_addr = (uintptr_t)(output->data);

  _q15* lea_add = lea_src;
  uint16_t add_size = _LEA_BIAS_ADD_SIZE;
  uintptr_t lea_add_addr = (uintptr_t)lea_add;
  for (uint16_t i = 0; i < out_channels; ++i) {
    offset_params.offset = bias->data[i];
    offset_params.length = lea_remain_bias_size_aligned;
    add_params.length = lea_remain_bias_size_aligned;

    DMA_makeTransfer(output_fram_addr, lea_add_addr, lea_bias_remain_size);
    
    msp_add_q15_time(&add_params, lea_add, lea_add, lea_add, repeat, &time);
    msp_add_q15_time(&add_params, lea_add, lea_add, lea_add, repeat, &time);
    msp_offset_q15_time(&offset_params, lea_add, lea_add, repeat, &time);

    DMA_makeTransfer(lea_add_addr, output_fram_addr, lea_bias_remain_size);
    output_fram_addr += output_bias_remain_size_offset;
    offset_params.length = add_size;
    add_params.length = add_size;

    for (uint16_t j = lea_bias_remain_size; j < output_len; j += add_size) {
      DMA_makeTransfer(output_fram_addr, lea_add_addr, add_size);
      
      msp_add_q15_time(&add_params, lea_add, lea_add, lea_add, repeat, &time);
      msp_add_q15_time(&add_params, lea_add, lea_add, lea_add, repeat, &time);
      msp_offset_q15_time(&offset_params, lea_add, lea_add, repeat, &time);

      DMA_makeTransfer(lea_add_addr, output_fram_addr, add_size);
      output_fram_addr += output_lea_bias_min_size_offset;
    }
  }

  *t = time;
}