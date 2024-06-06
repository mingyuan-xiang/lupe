#ifndef UTILS_H
#define UTILS_H

#include <libfixedAbstract/fixed.h>
#include <libmspdriver/driverlib.h>
#include <libmatAbstract/mat.h>
#include <stdint.h>
#include <libdsp/DSPLib.h>

#define LEA_FLT_SIZE 100
#define LEA_SRC_SIZE 100
#define LEA_DST_SIZE 100
#define LEA_MIN_SIZE 100

#define LUPE_MIN 0x8000
#define LUPE_Q15_MAX 32766 // 2^15 - 2

#define MAKE_ALIGN_2(s) ((s) + ((s) & 0x1))

extern _q15 lea_flt[LEA_FLT_SIZE];
extern _q15 lea_src[LEA_SRC_SIZE];
extern _q15 lea_tmp[LEA_DST_SIZE];
extern _q15 lea_dst[LEA_DST_SIZE];
extern _iq31 lea_res[2];

extern DMA_initParam dma_config;



#define DMA_makeTransfer(src, dst, size) {\
	DMA_setTransferSize(dma_config.channelSelect, size); \
	DMA_setSrcAddress(dma_config.channelSelect, src, DMA_DIRECTION_INCREMENT); \
  DMA_setDstAddress(dma_config.channelSelect, dst, DMA_DIRECTION_INCREMENT); \
	DMA_enableTransfers(dma_config.channelSelect); \
  uint16_t interruptState = __get_interrupt_state(); \
  __disable_interrupt(); \
  DMA_startTransfer(dma_config.channelSelect); \
	__bis_SR_register(GIE + LPM0_bits); \ 
	__set_interrupt_state(interruptState); \
}



void init_lupe();

#endif