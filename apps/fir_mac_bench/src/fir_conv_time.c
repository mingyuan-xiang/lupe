#include <include/utils.h>
#include <include/fir_conv_time.h>
#include <libmspsyncioutils/mspsyncioutils.h>
#include <libmsptimer/timekeeper.h>

#define _LEA_ADD_SIZE 400

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

static msp_status msp_fir_q15_time(const msp_fir_q15_params *params, const _q15 *src, _q15 *dst, uint16_t repeat, uint32_t* t) {
    uint32_t time = 0;
    uint16_t tapLength;
    uint16_t bufferMask;
    uint16_t outputLength;
    bool enableCircBuf;
    msp_status status;
    MSP_LEA_FIR_PARAMS *leaParams;

    /* Save parameters to local variables. */
    tapLength = params->tapLength;
    outputLength = params->length;
    enableCircBuf = params->enableCircularBuffer;

    /* Check that tap and output length are a multiple of two. */
    if ((tapLength & 1) || (outputLength & 1)) {
        return MSP_SIZE_ERROR;
    }
    
    /* Check that the length is a power of two if circular buffer is enabled. */
    if (enableCircBuf && (outputLength & (outputLength-1))) {
        return MSP_SIZE_ERROR;
    }

    /* Check that the data arrays are aligned and in a valid memory segment. */
    if (!(MSP_LEA_VALID_ADDRESS(src, 4) &
          MSP_LEA_VALID_ADDRESS(dst, 4) &
          MSP_LEA_VALID_ADDRESS(params->coeffs, 4))) {
        return MSP_LEA_INVALID_ADDRESS;
    }

    /* Acquire lock for LEA module. */
    if (!msp_lea_acquireLock()) {
        return MSP_LEA_BUSY;
    }

    /* Set buffer mask parameter. */
    if (enableCircBuf) {
        bufferMask = outputLength - 1;
    }
    else {
        bufferMask = 0xffff;
    }

    /* Initialize LEA if it is not enabled. */
    if (!(LEAPMCTL & LEACMDEN)) {
        msp_lea_init();
    }

    /* Allocate MSP_LEA_FIR_PARAMS structure. */
    leaParams = (MSP_LEA_FIR_PARAMS *)msp_lea_allocMemory(sizeof(MSP_LEA_FIR_PARAMS)/sizeof(uint32_t));

    /* Set MSP_LEA_FIR_PARAMS structure. */
    leaParams->vectorSize = outputLength;
    leaParams->tapLength = tapLength;
    leaParams->bufferMask = bufferMask;

    start_timer();
    for (uint16_t r = 0; r < repeat; r++) {
      leaParams->coeffs = MSP_LEA_CONVERT_ADDRESS(params->coeffs);
      leaParams->output = MSP_LEA_CONVERT_ADDRESS(dst);

      /* Load source arguments to LEA. */
      LEAPMS0 = MSP_LEA_CONVERT_ADDRESS(src);
      LEAPMS1 = MSP_LEA_CONVERT_ADDRESS(leaParams);

      /* Invoke the LEACMD__FIR command. */
      msp_lea_invokeCommand(LEACMD__FIR);
    }
    time += stop_timer();

    /* Free MSP_LEA_FIR_PARAMS structure. */
    msp_lea_freeMemory(sizeof(MSP_LEA_FIR_PARAMS)/sizeof(uint32_t));

    /* Set status flag. */
    status = MSP_SUCCESS;

    /* Check LEA interrupt flags for any errors. */
    if (msp_lea_ifg & LEACOVLIFG) {
        status = MSP_LEA_COMMAND_OVERFLOW;
    }
    else if (msp_lea_ifg & LEAOORIFG) {
        /* SW workaround for OOR interrupt when src is start of LEA memory. */
        if ((uintptr_t)src + (tapLength+outputLength)*(sizeof(int16_t)) > LEAMT) {
            status = MSP_LEA_OUT_OF_RANGE;
        }
    }
    else if (msp_lea_ifg & LEASDIIFG) {
        status = MSP_LEA_SCALAR_INCONSISTENCY;
    }

    /* Free lock for LEA module and return status. */
    msp_lea_freeLock();

    *t += time;
    return status;
}

void fir_conv_time(mat_t* input, mat_t* output, mat_t* weight, mat_t* bias, uint16_t repeat, uint32_t* t) {
  uint16_t in_channels = input->dims[1];
  uint16_t out_channels = output->dims[1];
  uint16_t output_len = output->strides[1];
  uint16_t input_len = input->strides[1];
  uint16_t kernel_row_size = weight->dims[2];
  uint16_t kernel_col_size = weight->dims[3];

  uintptr_t src_lea_addr = (uintptr_t)lea_src;
  uintptr_t dst_lea_addr = (uintptr_t)lea_dst;

  uintptr_t flt_lea_addr = (uintptr_t)lea_flt;
  uintptr_t flt_fram_addr = (uintptr_t)(weight->data);
  uintptr_t flt_addr_col_offset = kernel_col_size * sizeof(int16_t);
  uintptr_t flt_addr_padding_offset = (kernel_col_size + 1) * sizeof(int16_t);
  uintptr_t input_lea_addr = src_lea_addr;
  uintptr_t input_fram_addr = (uintptr_t)(input->data);
  uintptr_t input_channel_addr = input_fram_addr;
  uintptr_t input_channel_offset = input_len * sizeof(int16_t);
  uintptr_t output_lea_addr = dst_lea_addr;
  uintptr_t output_fram_addr = (uintptr_t)(output->data);
  uintptr_t output_addr_offset = output_len * sizeof(int16_t);

  int16_t* conv_flt = lea_flt;
  uint16_t input_line_size = input->dims[3];
  uint16_t output_line_size = output->dims[3];
  uint16_t output_line_num = output->dims[2];
  uint16_t output_line_size_offset = output_line_size * sizeof(int16_t);
  uint16_t input_line_size_offset = input_line_size * sizeof(int16_t);

  uint16_t lea_remain_size = output_len % _LEA_ADD_SIZE;
  uintptr_t output_remain_size_offset = lea_remain_size * sizeof(int16_t);
  uintptr_t output_lea_min_size_offset = _LEA_ADD_SIZE * sizeof(int16_t);
  uint16_t lea_remain_size_aligned = MAKE_ALIGN_2(lea_remain_size);
  
  uint32_t time = 0;
  
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

  lea_src[input_line_size] = 0;
  lea_src[input_line_size + 1] = 0;
  memset(output->data, 0, GET_MAT_SIZE(output)*sizeof(int16_t));

  /* convolution */
  for (uint16_t i = 0; i < out_channels; ++i) {
    input_fram_addr = (uintptr_t)(input->data);
    for (uint16_t j = 0; j < in_channels; ++j) {
      uintptr_t tmp_output_addr = output_fram_addr;
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

      for (uint16_t l = 0; l < output_line_num; ++l) {
        uintptr_t tmp_input_addr = input_channel_addr;
        /* send output to LEA RAM */
        DMA_makeTransfer(tmp_output_addr, output_lea_addr, output_line_size);

        conv_flt = lea_flt;

        for (uint16_t k = 0; k < kernel_row_size; ++k) {
          /* send input to LEA RAM */
          DMA_makeTransfer(tmp_input_addr, input_lea_addr, input_line_size);
          conv_params.coeffs = conv_flt;

          /* convolution */
          msp_fir_q15_time(&conv_params, lea_src, lea_tmp, repeat, &time);

          /* accumulate results for a 2D convolution */
          msp_add_q15_time(&add_params, lea_dst, lea_tmp, lea_dst, repeat, &time);
          conv_flt += conv_params.tapLength;
          tmp_input_addr += input_line_size_offset;
        }

        /* bring back output from LEA RAM */
        DMA_makeTransfer(output_lea_addr, tmp_output_addr, output_line_size);
        tmp_output_addr += output_line_size_offset;
        input_channel_addr += input_line_size_offset;
      }
      input_fram_addr += input_channel_offset;
    }
    output_fram_addr += output_addr_offset;
  }

  /* add bias and left shift */
  output_fram_addr = (uintptr_t)(output->data);

  _q15* lea_add = lea_src;
  uint16_t add_size = _LEA_ADD_SIZE;
  uintptr_t lea_add_addr = (uintptr_t)lea_add;
  for (uint16_t i = 0; i < out_channels; ++i) {
    offset_params.offset = bias->data[i];
    offset_params.length = lea_remain_size_aligned;
    add_params.length = lea_remain_size_aligned;

    DMA_makeTransfer(output_fram_addr, lea_add_addr, lea_remain_size);
    
    msp_add_q15_time(&add_params, lea_add, lea_add, lea_add, repeat, &time);
    msp_add_q15_time(&add_params, lea_add, lea_add, lea_add, repeat, &time);
    msp_offset_q15_time(&offset_params, lea_add, lea_add, repeat, &time);

    DMA_makeTransfer(lea_add_addr, output_fram_addr, lea_remain_size);
    output_fram_addr += output_remain_size_offset;
    offset_params.length = add_size;
    add_params.length = add_size;

    for (uint16_t j = lea_remain_size; j < output_len; j += add_size) {
      DMA_makeTransfer(output_fram_addr, lea_add_addr, add_size);
      
      msp_add_q15_time(&add_params, lea_add, lea_add, lea_add, repeat, &time);
      msp_add_q15_time(&add_params, lea_add, lea_add, lea_add, repeat, &time);
      msp_offset_q15_time(&offset_params, lea_add, lea_add, repeat, &time);

      DMA_makeTransfer(lea_add_addr, output_fram_addr, add_size);
      output_fram_addr += output_lea_min_size_offset;
    }
  }
  *t = time;
}