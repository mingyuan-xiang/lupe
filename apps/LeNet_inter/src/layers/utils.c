#include <layers/include/utils.h>

DSPLIB_DATA(lea_flt, 2) _q15 lea_flt[LEA_FLT_SIZE];
DSPLIB_DATA(lea_src, 2) _q15 lea_src[LEA_SRC_SIZE];
DSPLIB_DATA(lea_dst, 2) _q15 lea_dst[LEA_DST_SIZE];
DSPLIB_DATA(lea_tmp, 2) _q15 lea_tmp[LEA_TMP_SIZE];
DSPLIB_DATA(lea_res, 4) _iq31 lea_res[2];

__ro_hinv int16_t intermittent_buffer[INTERMITTENT_BUFFER_SIZE];

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
    DMA_is_init = 1;
  }

  intermittent_init();
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