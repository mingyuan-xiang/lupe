#include <include/utils.h>

DSPLIB_DATA(lea_buffer, 2) _q15 lea_buffer[LEA_SIZE];
DSPLIB_DATA(lea_res, 4) _iq31 lea_res[2];

MSP_LEA_ADDMATRIX_PARAMS* lea_add_params;
MSP_LEA_FIR_PARAMS* lea_fir_params;
MSP_LEA_MAC_PARAMS* lea_mac_params;
MSP_LEA_ADDMATRIX_PARAMS* lea_fill_params;
MSP_LEA_MPYMATRIX_PARAMS* lea_mpy_params;
int16_t* fill_vector;
uint16_t fill_vector_addr;
MSP_LEA_ADDMATRIX_PARAMS* lea_offset_params;
int16_t* offset_vector;
MSP_LEA_DEINTERLEAVE_PARAMS* lea_deinterleave_params;
uint16_t deinterleave_cmdId;
uint16_t deinterleave_channel;
MSP_LEA_MPYMATRIXROW_PARAMS* lea_vector_matrix_mpy_params;

static int DMA_is_init = 0;

#define DMA_initialization(CHANNEL) { \
  uint8_t ch = DMA_CHANNEL_##CHANNEL; \
  uint16_t transferModeSelect = DMA_TRANSFER_BLOCK; \
  uint8_t transferUnitSelect = DMA_SIZE_SRCWORD_DSTWORD; \
  uint8_t triggerTypeSelect = DMA_TRIGGER_RISINGEDGE; \
  uint8_t triggerOffset = (ch >> 4); \
  uint8_t triggerSourceSelect = DMA_TRIGGERSOURCE_0; \
  HWREG16(DMA_BASE + ch + OFS_DMA0CTL) = \
    transferModeSelect + transferUnitSelect + triggerTypeSelect; \
  if(triggerOffset & 0x01) { \
    HWREG16(DMA_BASE + (triggerOffset & 0x0E)) &= 0x00FF; \
    HWREG16(DMA_BASE + (triggerOffset & 0x0E)) |= (triggerSourceSelect << 8); \
  } else { \
    HWREG16(DMA_BASE + (triggerOffset & 0x0E)) &= 0xFF00; \
    HWREG16(DMA_BASE + (triggerOffset & 0x0E)) |= triggerSourceSelect; \
  } \
  HWREG16(DMA_BASE + ch + OFS_DMA0CTL) |= DMAIE; \
}

void init_lupe() {
  if (DMA_is_init == 0) {
    DMA_disableTransferDuringReadModifyWrite();
    DMA_initialization(0);
    DMA_initialization(1);
  /* init LEA */
  if (!(LEAPMCTL & LEACMDEN)) {
    msp_lea_init();
  }
    DMA_is_init = 1;
  }
}

void add_init(uint16_t length) {
  lea_add_params = (MSP_LEA_ADDMATRIX_PARAMS*)msp_lea_allocMemory(sizeof(MSP_LEA_ADDMATRIX_PARAMS)/sizeof(uint32_t));
  lea_add_params->vectorSize = length;
  lea_add_params->input1Offset = 1;
  lea_add_params->input2Offset = 1;
  lea_add_params->outputOffset = 1;
}

void fir_init(uint16_t length, uint16_t tapLength) {
  lea_fir_params = (MSP_LEA_FIR_PARAMS*)msp_lea_allocMemory(sizeof(MSP_LEA_FIR_PARAMS)/sizeof(uint32_t));
  lea_fir_params->vectorSize = length;
  lea_fir_params->tapLength = tapLength;
  lea_fir_params->bufferMask = 0xffff;
}

void mac_init(uint16_t length) {
  lea_mac_params = (MSP_LEA_MAC_PARAMS*)msp_lea_allocMemory(sizeof(MSP_LEA_MAC_PARAMS)/sizeof(uint32_t));
  lea_mac_params->vectorSize = length;
  lea_mac_params->output = MSP_LEA_CONVERT_ADDRESS(lea_res);
}

void offset_init(uint16_t length) {
  lea_offset_params = (MSP_LEA_ADDMATRIX_PARAMS*)msp_lea_allocMemory(sizeof(MSP_LEA_ADDMATRIX_PARAMS)/sizeof(uint32_t));
  offset_vector = (int16_t*)msp_lea_allocMemory(2*sizeof(int16_t)/sizeof(uint32_t));
  lea_offset_params->input2 = MSP_LEA_CONVERT_ADDRESS(offset_vector);
  lea_offset_params->vectorSize = length;
  lea_offset_params->input1Offset = 1;
  lea_offset_params->input2Offset = 0;
  lea_offset_params->outputOffset = 1;
}

void fill_init(uint16_t length) {
  lea_fill_params = (MSP_LEA_ADDMATRIX_PARAMS*)msp_lea_allocMemory(sizeof(MSP_LEA_ADDMATRIX_PARAMS)/sizeof(uint32_t));
  fill_vector = (int16_t*)msp_lea_allocMemory(2*sizeof(int16_t)/sizeof(uint32_t));
  fill_vector_addr =  MSP_LEA_CONVERT_ADDRESS(fill_vector);
  lea_fill_params->input2 = MSP_LEA_CONST_ZERO;
  lea_fill_params->vectorSize = length;
  lea_fill_params->input1Offset = 0;
  lea_fill_params->input2Offset = 0;
  lea_fill_params->outputOffset = 1;
}

void mpy_init(uint16_t length) {
  lea_mpy_params = (MSP_LEA_MPYMATRIX_PARAMS*)msp_lea_allocMemory(sizeof(MSP_LEA_MPYMATRIX_PARAMS)/sizeof(uint32_t));
  lea_mpy_params->vectorSize = length;
  lea_mpy_params->input1Offset = 1;
  lea_mpy_params->input2Offset = 1;
  lea_mpy_params->outputOffset = 1;
}

void deinterleave_init(uint16_t length, uint16_t channel, uint16_t numChannels) {
  lea_deinterleave_params = (MSP_LEA_DEINTERLEAVE_PARAMS*)msp_lea_allocMemory(sizeof(MSP_LEA_DEINTERLEAVE_PARAMS)/sizeof(uint32_t));
  lea_deinterleave_params->vectorSize = length;
  lea_deinterleave_params->interleaveDepth = numChannels;

  /* Determine which LEA deinterleave command to invoke. */
  if (channel & 1) {
    if (numChannels & 1) {
      /* Invoke the LEACMD__DEINTERLEAVEODDODD command. */
      deinterleave_cmdId = LEACMD__DEINTERLEAVEODDODD;
    } else {
      /* Invoke the LEACMD__DEINTERLEAVEODDEVEN command. */
      deinterleave_cmdId = LEACMD__DEINTERLEAVEODDEVEN;
    }
  } else {
    if (numChannels & 1) {
      /* Invoke the LEACMD__DEINTERLEAVEEVENODD command. */
      deinterleave_cmdId = LEACMD__DEINTERLEAVEEVENODD;
    } else {
      /* Invoke the LEACMD__DEINTERLEAVEEVENEVEN command. */
      deinterleave_cmdId = LEACMD__DEINTERLEAVEEVENEVEN;
    }
  }
  deinterleave_channel = channel;
}

void vector_matrix_mpy_init(uint16_t mat_row, uint16_t mat_col) {
  lea_vector_matrix_mpy_params = (MSP_LEA_MPYMATRIXROW_PARAMS*)msp_lea_allocMemory(sizeof(MSP_LEA_MPYMATRIXROW_PARAMS)/sizeof(uint32_t));
  lea_vector_matrix_mpy_params->rowSize = mat_row;
  lea_vector_matrix_mpy_params->colSize = mat_col;
}

void add_clear() {
  msp_lea_freeMemory(sizeof(MSP_LEA_ADDMATRIX_PARAMS)/sizeof(uint32_t));
}

void fir_clear() {
  msp_lea_freeMemory(sizeof(MSP_LEA_FIR_PARAMS)/sizeof(uint32_t));
}

void mac_clear() {
  msp_lea_freeMemory(sizeof(MSP_LEA_MAC_PARAMS)/sizeof(uint32_t));
}

void offset_clear() {
  msp_lea_freeMemory(2*sizeof(int16_t)/sizeof(uint32_t));
  msp_lea_freeMemory(sizeof(MSP_LEA_ADDMATRIX_PARAMS)/sizeof(uint32_t));
}

void fill_clear() {
  msp_lea_freeMemory(2*sizeof(int16_t)/sizeof(uint32_t));
  msp_lea_freeMemory(sizeof(MSP_LEA_ADDMATRIX_PARAMS)/sizeof(uint32_t));
}

void mpy_clear() {
  msp_lea_freeMemory(sizeof(MSP_LEA_MPYMATRIX_PARAMS)/sizeof(uint32_t));
}

void deinterleave_clear() {
  msp_lea_freeMemory(sizeof(MSP_LEA_DEINTERLEAVE_PARAMS)/sizeof(uint32_t));
}

void vector_matrix_mpy_clear() {
  msp_lea_freeMemory(sizeof(MSP_LEA_MPYMATRIXROW_PARAMS)/sizeof(uint32_t));
}

/* Wake up CPU once DMA is complete */
void __attribute__((interrupt(DMA_VECTOR))) dma_isr_handler(void) {
  switch (__even_in_range(DMAIV, DMAIV_DMA2IFG)) {
  case DMAIV_DMA0IFG:
  case DMAIV_DMA1IFG:
  case DMAIV_DMA2IFG:
    break;
  default:
    break;
  }
  __bic_SR_register_on_exit(LPM0_bits);
}