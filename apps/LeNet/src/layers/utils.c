#include <layers/include/utils.h>

DSPLIB_DATA(lea_flt, 2) _q15 lea_flt[LEA_FLT_SIZE];
DSPLIB_DATA(lea_src, 2) _q15 lea_src[LEA_SRC_SIZE];
DSPLIB_DATA(lea_tmp, 2) _q15 lea_tmp[LEA_DST_SIZE];
DSPLIB_DATA(lea_dst, 2) _q15 lea_dst[LEA_DST_SIZE];
DSPLIB_DATA(lea_res, 4) _iq31 lea_res[2];


DMA_initParam dma_config;
static int DMA_is_init = 0;

void init_lupe() {
  if (DMA_is_init == 0) {
    DMA_disableTransferDuringReadModifyWrite();
    dma_config.channelSelect = 0;
    dma_config.transferModeSelect = DMA_TRANSFER_BLOCK;
    dma_config.transferUnitSelect = DMA_SIZE_SRCWORD_DSTWORD;
    DMA_init(&dma_config);
    DMA_enableInterrupt(dma_config.channelSelect);



    DMA_is_init = 1;
  }
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