#ifndef UTILS_H
#define UTILS_H

#include <libfixedAbstract/fixed.h>
#include <libmspdriver/driverlib.h>
#include <libmatAbstract/mat.h>
#include <libmsppoweroff/poweroff.h>
#include <stdint.h>
#include <libdsp/DSPLib.h>
#include <layers/include/intermittent.h>

#define LEA_SIZE 1600

#define LUPE_MIN 0x8000
#define LUPE_Q15_MAX 32766 // 2^15 - 2

#define MAKE_ALIGN_2(s) ((s) + ((s) & 0x1))


extern _iq31 lea_res[];

/* As long as the size is smaller than LEA buffer size, we are good. */
#define INTERMITTENT_BUFFER_SIZE 1600
extern __ro_nv int16_t intermittent_buffer[INTERMITTENT_BUFFER_SIZE];

#define WRITE_DOUBLE_BUFFER_W_VAR(var, offset) { \
  __asm__ __volatile__ ( \
      "MOVX.W %0, &intermittent_status+%c1\n\t" \
      : \
      : "r" (var), "i" (GET_OFFSET(offset)) \
      : "memory" \
  ); \
}

#define SINGLE_BUFFER_TRANSFER(val, offset, in_addr, out_addr, size) { \
  int16_t next = val; \
  DMA_makeTransfer(in_addr, (uintptr_t)intermittent_buffer, size); \
  next = next | DOUBLE_BUFFER_WRITE; \
  WRITE_DOUBLE_BUFFER_W_VAR(next, offset); \
  DMA_makeTransfer(in_addr, out_addr, size); \
  next = next & DOUBLE_BUFFER_COMPLETE; \
  WRITE_DOUBLE_BUFFER_W_VAR(next, offset); \
}

#define DOUBLE_BUFFER_TRANSFER(val, offset, in_addr, out_addr, size) { \
  int16_t next = val; \
  DMA_makeTransfer(in_addr, (uintptr_t)intermittent_buffer, size); \
  next = next | DOUBLE_BUFFER_WRITE; \
  WRITE_DOUBLE_BUFFER_W_VAR(next, offset); \
  DMA_makeTransfer(in_addr, out_addr, size); \
  next = next & DOUBLE_BUFFER_COMPLETE; \
  WRITE_DOUBLE_BUFFER_W_VAR(next, offset); \
}

#define DOUBLE_BUFFER_ASSIGN(val, offset, in, out) { \
  int16_t next = val; \
  intermittent_buffer[0] = in; \
  next = next | DOUBLE_BUFFER_WRITE; \
  WRITE_DOUBLE_BUFFER_W_VAR(next, offset); \
  out = in; \
  next = next & DOUBLE_BUFFER_COMPLETE; \
  WRITE_DOUBLE_BUFFER_W_VAR(next, offset); \
}

#define GET_MAT_SIZE(mat) ((mat)->strides[0] * (mat)->dims[0])


extern _q15 lea_buffer[];
extern _iq31 lea_res[];


extern MSP_LEA_ADDMATRIX_PARAMS* lea_add_params;
extern MSP_LEA_FIR_PARAMS* lea_fir_params;
extern MSP_LEA_MAC_PARAMS* lea_mac_params;
extern MSP_LEA_ADDMATRIX_PARAMS* lea_fill_params;
extern MSP_LEA_MPYMATRIX_PARAMS* lea_mpy_params;
extern int16_t* fill_vector;
extern uint16_t fill_vector_addr;
extern MSP_LEA_ADDMATRIX_PARAMS* lea_offset_params;
extern int16_t* offset_vector;
extern MSP_LEA_DEINTERLEAVE_PARAMS* lea_deinterleave_params;
extern uint16_t deinterleave_cmdId;
extern uint16_t deinterleave_channel;
extern MSP_LEA_MPYMATRIXROW_PARAMS* lea_vector_matrix_mpy_params;

#define GET_MAT_SIZE(mat) ((mat)->strides[0] * (mat)->dims[0])


#define DMA_EN_SIG (DMAEN | DMAREQ)

#define _lupe_data16_write_addr(addr, src) ({ \
  uintptr_t __src = src; \
  __asm__  __volatile__ ("movx.a %1, %0\n\t" : "=m"(addr) : "r"(__src)); \
})

#define __lupe_data16_write_addr(addr, src) _lupe_data16_write_addr((addr), (src))

#define DMA_makeTransfer(src, dst, size) { \
  DMA0CTL = DMADT_1 | DMADSTINCR_3 | DMASRCINCR_3 | DMADSTBYTE__WORD | DMASRCBYTE__WORD | DMAIE; \
  __lupe_data16_write_addr((DMA0SA), (src)); \
  __lupe_data16_write_addr((DMA0DA), (dst)); \
  DMA0SZ = size; \
  uint16_t interruptState = __get_interrupt_state(); \
  __disable_interrupt(); \
  DMA0CTL |= DMA_EN_SIG; \
  __bis_SR_register(GIE + LPM0_bits); \
  __set_interrupt_state(interruptState); \
}

#define DMA_setWord(dst, word, size) { \
  DMA1CTL = DMADT_1 | DMADSTINCR_3 | DMASRCINCR_0 | DMADSTBYTE__WORD | DMASRCBYTE__WORD | DMAIE; \
  __lupe_data16_write_addr((DMA1SA), ((uintptr_t)(&word))); \
  __lupe_data16_write_addr((DMA1DA), (dst)); \
  DMA1SZ = size; \
  uint16_t interruptState = __get_interrupt_state(); \
  __disable_interrupt(); \
  DMA1CTL |= DMA_EN_SIG; \
  __bis_SR_register(GIE + LPM0_bits); \
  __set_interrupt_state(interruptState); \
}



void add_init(uint16_t length);

void fir_init(uint16_t length, uint16_t tapLength);

void mac_init(uint16_t length);

void offset_init(uint16_t length);

void fill_init(uint16_t length);

void mpy_init(uint16_t length);

void deinterleave_init(uint16_t length, uint16_t channel, uint16_t numChannels);

void vector_matrix_mpy_init(uint16_t mat_row, uint16_t mat_col);

void add_clear();

void fir_clear();

void mac_clear();

void offset_clear();

void fill_clear();

void mpy_clear();

void deinterleave_clear();

void vector_matrix_mpy_clear();

void init_lupe();

#endif