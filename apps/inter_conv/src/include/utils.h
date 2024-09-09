#ifndef UTILS_H
#define UTILS_H

#include <libfixedAbstract/fixed.h>
#include <libmspdriver/driverlib.h>
#include <libmatAbstract/mat.h>
#include <stdint.h>
#include <libdsp/DSPLib.h>

#define LEA_FLT_SIZE 400
#define LEA_SRC_SIZE 400
#define LEA_DST_SIZE 400
#define LEA_TMP_SIZE 400

#define LUPE_MIN 0x8000
#define LUPE_Q15_MAX 32766 // 2^15 - 2

#define MAKE_ALIGN_2(s) ((s) + ((s) & 0x1))


extern _q15 lea_flt[];
extern _q15 lea_src[];
extern _q15 lea_tmp[];
extern _q15 lea_dst[];
extern _iq31 lea_res[];



#define GET_MAT_SIZE(mat) ((mat)->strides[0] * (mat)->dims[0])


#define DMA_makeTransfer(src, dst, size) {\
	DMA_setTransferSize(DMA_CHANNEL_0, size); \
	DMA_setSrcAddress(DMA_CHANNEL_0, src, DMA_DIRECTION_INCREMENT); \
  DMA_setDstAddress(DMA_CHANNEL_0, dst, DMA_DIRECTION_INCREMENT); \
	DMA_enableTransfers(DMA_CHANNEL_0); \
  uint16_t interruptState = __get_interrupt_state(); \
  __disable_interrupt(); \
  DMA_startTransfer(DMA_CHANNEL_0); \
	__bis_SR_register(GIE + LPM0_bits); \
	__set_interrupt_state(interruptState); \
}



void init_lupe();

#endif