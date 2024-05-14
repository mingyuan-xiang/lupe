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
  uint8_t channel = dma_config.channelSelect; \
  /* DMA_setTransferSize */ \
  HWREG16(DMA_BASE + channel + OFS_DMA0SZ) = size; \
  /* DMA_setSrcAddress */ \
  __data16_write_addr((unsigned short)(DMA_BASE + channel + OFS_DMA0SA), src); \
  HWREG16(DMA_BASE + channel + OFS_DMA0CTL) &= ~(DMASRCINCR_3); \
  HWREG16(DMA_BASE + channel + OFS_DMA0CTL) |= DMA_DIRECTION_INCREMENT; \
  /* DMA_setDstAddress */ \
  __data16_write_addr((unsigned short)(DMA_BASE + channel + OFS_DMA0DA), dst); \
  HWREG16(DMA_BASE + channel + OFS_DMA0CTL) &= ~(DMADSTINCR_3); \
  HWREG16(DMA_BASE + channel + OFS_DMA0CTL) |= (DMA_DIRECTION_INCREMENT << 2); \
  /* DMA_DMA_enableTransfers */ \
  HWREG16(DMA_BASE + channel + OFS_DMA0CTL) |= DMAEN; \
  /* shut off CPU and make DMA transfer */ \
  uint16_t interruptState = __get_interrupt_state(); \
  __disable_interrupt(); \
  HWREG16(DMA_BASE + channel + OFS_DMA0CTL) |= DMAREQ; \
  __bis_SR_register(GIE + LPM0_bits); \
  __set_interrupt_state(interruptState); \
}

void init_lupe();

#endif