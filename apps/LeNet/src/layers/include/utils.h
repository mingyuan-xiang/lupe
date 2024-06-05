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

extern MSP_LEA_ADDMATRIX_PARAMS* lea_add_params;
extern MSP_LEA_FIR_PARAMS* lea_fir_params;
extern MSP_LEA_MAC_PARAMS* lea_mac_params;


// DMAEN | DMAREQ
#define DMA_EN_SIG 0x0011

#define DMA_makeTransfer(src, dst, size) {\
  __disable_interrupt(); \
  __data16_write_addr((uintptr_t)(&DMA0SA), src); \
  __data16_write_addr((uintptr_t)(&DMA0DA), dst); \
  DMA0SZ = size; \
  DMA0CTL |= DMA_EN_SIG; \
  __bis_SR_register(GIE + LPM0_bits); \
}



void add_init(uint16_t length);

void fir_init(uint16_t length, uint16_t tapLength);

void mac_init(uint16_t length);

void add_clear();

void fir_clear();

void mac_clear();

void init_lupe();

#endif