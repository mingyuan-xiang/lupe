#include <include/lea_op.h>
#include <libmspsyncioutils/mspsyncioutils.h>
#include <libmsptimer/timekeeper.h>

DSPLIB_DATA(lea_flt, 2) _q15 lea_flt[LEA_SIZE];
DSPLIB_DATA(lea_src, 2) _q15 lea_src[LEA_SIZE];
DSPLIB_DATA(lea_dst, 2) _q15 lea_dst[LEA_SIZE];
DSPLIB_DATA(lea_tmp, 2) _q15 lea_tmp[LEA_SIZE];
DSPLIB_DATA(lea_res, 4) _iq31 lea_res[2];

msp_status msp_add_q15_time(const msp_add_q15_params *params, const _q15 *srcA, const _q15 *srcB, _q15 *dst, uint16_t repeat) {
    uint32_t other_time = 0;
    uint32_t invoke_time = 0;
    uint16_t length;
    msp_status status;
    MSP_LEA_ADDMATRIX_PARAMS *leaParams;

    start_timer();
    for (uint16_t r = 0; r < repeat; r++) {
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
    }
    other_time = stop_timer();

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
    invoke_time = stop_timer();

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

    msp_send_printf("invocation time: %n; other time: %n", invoke_time, other_time);

    return status;
}

msp_status msp_offset_q15_time(const msp_offset_q15_params *params, const _q15 *src, _q15 *dst, uint16_t repeat) {
    uint32_t other_time = 0;
    uint32_t invoke_time = 0;
    uint16_t length;
    _q15 q15Offset;
    int16_t *offsetVector;
    msp_status status;
    MSP_LEA_ADDMATRIX_PARAMS *leaParams;

    length = params->length;
    q15Offset = params->offset;

    start_timer();
    for (uint16_t r = 0; r < repeat; r++) {
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
    }
    other_time = stop_timer();

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
    invoke_time = stop_timer();

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

    msp_send_printf("invocation time: %n; other time: %n", invoke_time, other_time);
    return status;
}

msp_status msp_mac_q15_time(const msp_mac_q15_params *params, const _q15 *srcA, const _q15 *srcB, _iq31 *result, uint16_t repeat) {
    uint32_t other_time = 0;
    uint32_t invoke_time = 0;
    uint16_t length;
    msp_status status;
    MSP_LEA_MAC_PARAMS *leaParams;


    start_timer();
    for (uint16_t r = 0; r < repeat; r++) {
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
    }
    other_time = stop_timer();

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
    invoke_time = stop_timer();

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

    msp_send_printf("invocation time: %n; other time: %n", invoke_time, other_time);
    return status;
}

msp_status msp_fir_q15_time(const msp_fir_q15_params *params, const _q15 *src, _q15 *dst, uint16_t repeat) {
    uint32_t other_time = 0;
    uint32_t invoke_time = 0;
    uint16_t tapLength;
    uint16_t bufferMask;
    uint16_t outputLength;
    bool enableCircBuf;
    msp_status status;
    MSP_LEA_FIR_PARAMS *leaParams;

    start_timer();
    for (uint16_t r = 0; r < repeat; r++) {
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
    }
    other_time = stop_timer();

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
    invoke_time = stop_timer();

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

    msp_send_printf("invocation time: %n; other time: %n", invoke_time, other_time);
    return status;
}

msp_status msp_fill_q15_time(const msp_fill_q15_params *params, _q15 *dst, uint16_t repeat) {
    uint32_t other_time = 0;
    uint32_t invoke_time = 0;
    uint16_t length;
    int16_t *fillVector;
    msp_status status;
    MSP_LEA_ADDMATRIX_PARAMS *leaParams;

    start_timer();
    for (uint16_t r = 0; r < repeat; r++) {
    /* Initialize the vector length. */
    length = params->length;

    /* Check that the data array is aligned and in a valid memory segment. */
    if (!MSP_LEA_VALID_ADDRESS(dst, 4)) {
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
        
    /* Allocate fill vector of length two. */
    fillVector = (int16_t *)msp_lea_allocMemory(2*sizeof(int16_t)/sizeof(uint32_t));
    fillVector[0] = params->value;
    fillVector[1] = params->value;

    /* Set MSP_LEA_ADDMATRIX_PARAMS structure. */
    leaParams->input2 = MSP_LEA_CONST_ZERO;
    leaParams->vectorSize = length;
    leaParams->input1Offset = 0;
    leaParams->input2Offset = 0;
    leaParams->outputOffset = 1;

    /* Free MSP_LEA_ADDMATRIX_PARAMS structure and fill vector. */
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
    }
    other_time = stop_timer();

    /* Initialize the vector length. */
    length = params->length;

    /* Check that the data array is aligned and in a valid memory segment. */
    if (!MSP_LEA_VALID_ADDRESS(dst, 4)) {
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
        
    /* Allocate fill vector of length two. */
    fillVector = (int16_t *)msp_lea_allocMemory(2*sizeof(int16_t)/sizeof(uint32_t));
    fillVector[0] = params->value;
    fillVector[1] = params->value;

    /* Set MSP_LEA_ADDMATRIX_PARAMS structure. */
    leaParams->input2 = MSP_LEA_CONST_ZERO;
    leaParams->vectorSize = length;
    leaParams->input1Offset = 0;
    leaParams->input2Offset = 0;
    leaParams->outputOffset = 1;

    start_timer();
    for (uint16_t r = 0; r < repeat; r++) {
      leaParams->output = MSP_LEA_CONVERT_ADDRESS(dst);
      /* Load source arguments to LEA. */
      LEAPMS0 = MSP_LEA_CONVERT_ADDRESS(fillVector);
      LEAPMS1 = MSP_LEA_CONVERT_ADDRESS(leaParams);

      /* Invoke the LEACMD__ADDMATRIX command. */
      msp_lea_invokeCommand(LEACMD__ADDMATRIX);
    }
    invoke_time = stop_timer();

    /* Free MSP_LEA_ADDMATRIX_PARAMS structure and fill vector. */
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

    msp_send_printf("invocation time: %n; other time: %n", invoke_time, other_time);
    return status;
}

msp_status msp_mpy_q15_time(const msp_mpy_q15_params *params, const _q15 *srcA, const _q15 *srcB, _q15 *dst, uint16_t repeat) {
    uint32_t other_time = 0;
    uint32_t invoke_time = 0;
    uint16_t length;
    msp_status status;
    MSP_LEA_MPYMATRIX_PARAMS *leaParams;

    start_timer();
    for (uint16_t r = 0; r < repeat; r++) {
    /* Initialize the loop counter with the vector length. */
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
        
    /* Allocate MSP_LEA_MPYMATRIX_PARAMS structure. */
    leaParams = (MSP_LEA_MPYMATRIX_PARAMS *)msp_lea_allocMemory(sizeof(MSP_LEA_MPYMATRIX_PARAMS)/sizeof(uint32_t));

    /* Set MSP_LEA_MPYMATRIX_PARAMS structure. */
    leaParams->vectorSize = length;
    leaParams->input1Offset = 1;
    leaParams->input2Offset = 1;
    leaParams->outputOffset = 1;

    /* Free MSP_LEA_MPYMATRIX_PARAMS structure. */
    msp_lea_freeMemory(sizeof(MSP_LEA_MPYMATRIX_PARAMS)/sizeof(uint32_t));
    
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
    }
    other_time = stop_timer();

    /* Initialize the loop counter with the vector length. */
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
        
    /* Allocate MSP_LEA_MPYMATRIX_PARAMS structure. */
    leaParams = (MSP_LEA_MPYMATRIX_PARAMS *)msp_lea_allocMemory(sizeof(MSP_LEA_MPYMATRIX_PARAMS)/sizeof(uint32_t));

    /* Set MSP_LEA_MPYMATRIX_PARAMS structure. */

    leaParams->vectorSize = length;
    leaParams->input1Offset = 1;
    leaParams->input2Offset = 1;
    leaParams->outputOffset = 1;

    start_timer();
    for (uint16_t r = 0; r < repeat; r++) {
    leaParams->input2 = MSP_LEA_CONVERT_ADDRESS(srcB);
    leaParams->output = MSP_LEA_CONVERT_ADDRESS(dst);

    /* Load source arguments to LEA. */
    LEAPMS0 = MSP_LEA_CONVERT_ADDRESS(srcA);
    LEAPMS1 = MSP_LEA_CONVERT_ADDRESS(leaParams);

    /* Invoke the LEACMD__MPYMATRIX command. */
    msp_lea_invokeCommand(LEACMD__MPYMATRIX);
    }
    invoke_time = stop_timer();

    /* Free MSP_LEA_MPYMATRIX_PARAMS structure. */
    msp_lea_freeMemory(sizeof(MSP_LEA_MPYMATRIX_PARAMS)/sizeof(uint32_t));
    
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

    msp_send_printf("invocation time: %n; other time: %n", invoke_time, other_time);
    return status;
}

msp_status msp_deinterleave_q15_time(const msp_deinterleave_q15_params *params, const _q15 *src, _q15 *dst, uint16_t repeat) {
    uint32_t other_time = 0;
    uint32_t invoke_time = 0;
    uint16_t cmdId;
    uint16_t length;
    uint16_t channel;
    uint16_t numChannels;
    msp_status status;
    MSP_LEA_DEINTERLEAVE_PARAMS *leaParams;


    start_timer();
    for (uint16_t r = 0; r < repeat; r++) {
    /* Initialize local variables from parameters. */
    length = params->length;
    channel = params->channel;
    numChannels = params->numChannels;

    /* Check that the channel is less than the total number of channels. */
    if (channel > numChannels) {
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
        
    /* Allocate MSP_LEA_DEINTERLEAVE_PARAMS structure. */
    leaParams = (MSP_LEA_DEINTERLEAVE_PARAMS *)msp_lea_allocMemory(sizeof(MSP_LEA_DEINTERLEAVE_PARAMS)/sizeof(uint32_t));

    /* Set MSP_LEA_DEINTERLEAVE_PARAMS structure. */
    leaParams->vectorSize = length;
    leaParams->interleaveDepth = numChannels;

    /* Determine which LEA deinterleave command to invoke. */
    if (channel & 1) {
        if (numChannels & 1) {
            /* Invoke the LEACMD__DEINTERLEAVEODDODD command. */
            cmdId = LEACMD__DEINTERLEAVEODDODD;
        }
        else {
            /* Invoke the LEACMD__DEINTERLEAVEODDEVEN command. */
            cmdId = LEACMD__DEINTERLEAVEODDEVEN;
        }
    }
    else {
        if (numChannels & 1) {
            /* Invoke the LEACMD__DEINTERLEAVEEVENODD command. */
            cmdId = LEACMD__DEINTERLEAVEEVENODD;
        }
        else {
            /* Invoke the LEACMD__DEINTERLEAVEEVENEVEN command. */
            cmdId = LEACMD__DEINTERLEAVEEVENEVEN;
        }
    }

    /* Free MSP_LEA_DEINTERLEAVE_PARAMS structure. */
    msp_lea_freeMemory(sizeof(MSP_LEA_DEINTERLEAVE_PARAMS)/sizeof(uint32_t));

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
    }
    other_time = stop_timer();

    /* Initialize local variables from parameters. */
    length = params->length;
    channel = params->channel;
    numChannels = params->numChannels;

    /* Check that the channel is less than the total number of channels. */
    if (channel > numChannels) {
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
        
    /* Allocate MSP_LEA_DEINTERLEAVE_PARAMS structure. */
    leaParams = (MSP_LEA_DEINTERLEAVE_PARAMS *)msp_lea_allocMemory(sizeof(MSP_LEA_DEINTERLEAVE_PARAMS)/sizeof(uint32_t));

    /* Set MSP_LEA_DEINTERLEAVE_PARAMS structure. */
    leaParams->vectorSize = length;
    leaParams->interleaveDepth = numChannels;

    /* Determine which LEA deinterleave command to invoke. */
    if (channel & 1) {
        if (numChannels & 1) {
            /* Invoke the LEACMD__DEINTERLEAVEODDODD command. */
            cmdId = LEACMD__DEINTERLEAVEODDODD;
        }
        else {
            /* Invoke the LEACMD__DEINTERLEAVEODDEVEN command. */
            cmdId = LEACMD__DEINTERLEAVEODDEVEN;
        }
    }
    else {
        if (numChannels & 1) {
            /* Invoke the LEACMD__DEINTERLEAVEEVENODD command. */
            cmdId = LEACMD__DEINTERLEAVEEVENODD;
        }
        else {
            /* Invoke the LEACMD__DEINTERLEAVEEVENEVEN command. */
            cmdId = LEACMD__DEINTERLEAVEEVENEVEN;
        }
    }
    
    start_timer();
    for (uint16_t r = 0; r < repeat; r++) {
    leaParams->output = MSP_LEA_CONVERT_ADDRESS(dst);

    /* Load source arguments to LEA. */
    LEAPMS0 = MSP_LEA_CONVERT_ADDRESS(&src[channel]);
    LEAPMS1 = MSP_LEA_CONVERT_ADDRESS(leaParams);

    /* Invoke the command. */
    msp_lea_invokeCommand(cmdId);
    }
    invoke_time = stop_timer();

    /* Free MSP_LEA_DEINTERLEAVE_PARAMS structure. */
    msp_lea_freeMemory(sizeof(MSP_LEA_DEINTERLEAVE_PARAMS)/sizeof(uint32_t));

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

    msp_send_printf("invocation time: %n; other time: %n", invoke_time, other_time);
    return status;
}

msp_status msp_matrix_mpy_q15_time(const msp_matrix_mpy_q15_params *params, const _q15 *srcA, const _q15 *srcB, _q15 *dst, uint16_t repeat) {
    uint32_t other_time = 0;
    uint32_t invoke_time = 0;
    uint16_t srcARows;
    uint16_t srcACols;
    uint16_t srcBRows;
    uint16_t srcBCols;
    msp_status status;
    MSP_LEA_MPYMATRIXROW_PARAMS *leaParams;

    start_timer();
    for (uint16_t r = 0; r < repeat; r++) {
    /* Initialize the row and column sizes. */
    srcARows = params->srcARows;
    srcACols = params->srcACols;
    srcBRows = params->srcBRows;
    srcBCols = params->srcBCols;

    /* Check that column of A equals rows of B */
    if (srcACols != srcBRows) {
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

    /* Allocate MSP_LEA_MPYMATRIXROW_PARAMS structure. */
    leaParams = (MSP_LEA_MPYMATRIXROW_PARAMS *)msp_lea_allocMemory(sizeof(MSP_LEA_MPYMATRIXROW_PARAMS)/sizeof(uint32_t));

    /* Set status flag. */
    status = MSP_SUCCESS;

    /* Iterate through each row of srcA */
    while (srcARows--) {
        /* Set MSP_LEA_MPYMATRIXROW_PARAMS structure. */
        leaParams->rowSize = srcBRows;
        leaParams->colSize = srcBCols;

        /* Check LEA interrupt flags for any errors. */
        if (msp_lea_ifg & LEACOVLIFG) {
            status = MSP_LEA_COMMAND_OVERFLOW;
            break;
        }
        else if (msp_lea_ifg & LEAOORIFG) {
            status = MSP_LEA_OUT_OF_RANGE;
            break;
        }
        else if (msp_lea_ifg & LEASDIIFG) {
            status = MSP_LEA_SCALAR_INCONSISTENCY;
            break;
        }

        /* Increment srcA and dst pointers. */
        // srcA += srcACols;
        // dst += srcBCols;
    }

    /* Free MSP_LEA_MPYMATRIXROW_PARAMS structure. */
    msp_lea_freeMemory(sizeof(MSP_LEA_MPYMATRIXROW_PARAMS)/sizeof(uint32_t));

    /* Free lock for LEA module and return status. */
    msp_lea_freeLock();
    }
    other_time = stop_timer();

    /* Initialize the row and column sizes. */
    srcARows = params->srcARows;
    srcACols = params->srcACols;
    srcBRows = params->srcBRows;
    srcBCols = params->srcBCols;

    /* Check that column of A equals rows of B */
    if (srcACols != srcBRows) {
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

    /* Allocate MSP_LEA_MPYMATRIXROW_PARAMS structure. */
    leaParams = (MSP_LEA_MPYMATRIXROW_PARAMS *)msp_lea_allocMemory(sizeof(MSP_LEA_MPYMATRIXROW_PARAMS)/sizeof(uint32_t));

    /* Set status flag. */
    status = MSP_SUCCESS;

    /* Iterate through each row of srcA */
    while (srcARows--) {
        /* Set MSP_LEA_MPYMATRIXROW_PARAMS structure. */
        leaParams->rowSize = srcBRows;
        leaParams->colSize = srcBCols;

        start_timer();
        for (uint16_t r = 0; r < repeat; r++) {
        leaParams->colVector = MSP_LEA_CONVERT_ADDRESS(srcB);
        leaParams->output = MSP_LEA_CONVERT_ADDRESS(dst);

        /* Load source arguments to LEA. */
        LEAPMS0 = MSP_LEA_CONVERT_ADDRESS(srcA);
        LEAPMS1 = MSP_LEA_CONVERT_ADDRESS(leaParams);

        /* Invoke the LEACMD__MPYMATRIXROW command. */
        msp_lea_invokeCommand(LEACMD__MPYMATRIXROW);
        }
        invoke_time = stop_timer();

        /* Check LEA interrupt flags for any errors. */
        if (msp_lea_ifg & LEACOVLIFG) {
            status = MSP_LEA_COMMAND_OVERFLOW;
            break;
        }
        else if (msp_lea_ifg & LEAOORIFG) {
            status = MSP_LEA_OUT_OF_RANGE;
            break;
        }
        else if (msp_lea_ifg & LEASDIIFG) {
            status = MSP_LEA_SCALAR_INCONSISTENCY;
            break;
        }

        /* Increment srcA and dst pointers. */
        srcA += srcACols;
        dst += srcBCols;
    }

    /* Free MSP_LEA_MPYMATRIXROW_PARAMS structure. */
    msp_lea_freeMemory(sizeof(MSP_LEA_MPYMATRIXROW_PARAMS)/sizeof(uint32_t));

    /* Free lock for LEA module and return status. */
    msp_lea_freeLock();

    msp_send_printf("invocation time: %n; other time: %n", invoke_time, other_time);
    return status;
}
