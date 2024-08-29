#include <stdint.h>
#include <libmsp/mspbase.h>
#include <libmspdriver/driverlib.h>
#include <libmsptimer/timekeeper.h>
#include <libmspsyncioutils/mspsyncioutils.h>

#include <libfixedAbstract/fixed.h>
#include <libmspdriver/driverlib.h>
#include <libmatAbstract/mat.h>
#include <stdint.h>
#include <libdsp/DSPLib.h>

#define SIZE 100
#define REPEAT 10000

DSPLIB_DATA(buffer, 2) _q15 buffer_lea[SIZE];
volatile _q15 volatile_buffer_cpu[SIZE];
_q15 buffer_cpu[SIZE];

DMA_initParam dma_config;
static int DMA_is_init = 0;

void exit() {
  msp_end_printing();
  timers_stop();
}

void init_lupe() {
  if (DMA_is_init == 0) {
    DMA_disableTransferDuringReadModifyWrite();
    dma_config.channelSelect = 0;
    dma_config.transferModeSelect = DMA_TRANSFER_BLOCK;
    dma_config.transferUnitSelect = DMA_SIZE_SRCWORD_DSTWORD;
    DMA_init(&dma_config);
    DMA_enableInterrupt(dma_config.channelSelect);

    /* set DMA address direction */
    DMA0CTL &= ~(DMASRCINCR_3);
    DMA0CTL |= DMA_DIRECTION_INCREMENT;
    DMA0CTL &= ~(DMADSTINCR_3);
    DMA0CTL |= (DMA_DIRECTION_INCREMENT << 2);

    DMA1CTL = DMADT_1 | DMADSTINCR_3 | DMASRCINCR_0 | DMADSTBYTE__WORD | DMASRCBYTE__WORD | DMAIE;

    DMA_is_init = 1;
  }
}

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

// DMAEN | DMAREQ
#define DMA_EN_SIG 0x0011

#define _lupe_data16_write_addr(addr, src) ({ \
  uintptr_t __src = src; \
  __asm__  __volatile__ ("movx.a %1, %0\n\t" : "=m"(addr) : "r"(__src)); \
})

#define __lupe_data16_write_addr(addr, src) _lupe_data16_write_addr((addr), (src))

#define DMA_makeTransfer_opt(src, dst, size) { \
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

void init() {
  watchdog_disable();
  gpio_init_all_ports();
  gpio_unlock();
  gpio_clear_interrupts();
  clock_init();
  uartio_open(0);
  gpio_dir_out(1, 0);
  gpio_dir_out(1, 1);
  gpio_clear(1, 0);
  gpio_set(1, 1);
  timers_init();
  init_lupe();
}

volatile uint16_t s_start = 0;
volatile uint16_t s_end = 0;
volatile uint16_t pos = 0;

void loopcpy1() {
  s_start = 0;
  s_end = 0;
  pos = 0;
  for (uint16_t i = 0; i < REPEAT; ++i) {
    buffer_lea[0] = volatile_buffer_cpu[0];
    pos++;
  }
}

void loopcpy5() {
  s_start = 0;
  s_end = 0;
  pos = 0;
  for (uint16_t i = 0; i < REPEAT; ++i) {
    buffer_lea[0] = volatile_buffer_cpu[0];
    buffer_lea[1] = volatile_buffer_cpu[1];
    buffer_lea[2] = volatile_buffer_cpu[2];
    buffer_lea[3] = volatile_buffer_cpu[3];
    buffer_lea[4] = volatile_buffer_cpu[4];
    pos++;
  }
}



void loopcpy10() {
  s_start = 0;
  s_end = 0;
  pos = 0;
  for (uint16_t i = 0; i < REPEAT; ++i) {
    buffer_lea[0] = volatile_buffer_cpu[0];
    buffer_lea[1] = volatile_buffer_cpu[1];
    buffer_lea[2] = volatile_buffer_cpu[2];
    buffer_lea[3] = volatile_buffer_cpu[3];
    buffer_lea[4] = volatile_buffer_cpu[4];
    buffer_lea[5] = volatile_buffer_cpu[5];
    buffer_lea[6] = volatile_buffer_cpu[6];
    buffer_lea[7] = volatile_buffer_cpu[7];
    buffer_lea[8] = volatile_buffer_cpu[8];
    buffer_lea[9] = volatile_buffer_cpu[9];
    pos++;
  }
}



void loopcpy15() {
  s_start = 0;
  s_end = 0;
  pos = 0;
  for (uint16_t i = 0; i < REPEAT; ++i) {
    buffer_lea[0] = volatile_buffer_cpu[0];
    buffer_lea[1] = volatile_buffer_cpu[1];
    buffer_lea[2] = volatile_buffer_cpu[2];
    buffer_lea[3] = volatile_buffer_cpu[3];
    buffer_lea[4] = volatile_buffer_cpu[4];
    buffer_lea[5] = volatile_buffer_cpu[5];
    buffer_lea[6] = volatile_buffer_cpu[6];
    buffer_lea[7] = volatile_buffer_cpu[7];
    buffer_lea[8] = volatile_buffer_cpu[8];
    buffer_lea[9] = volatile_buffer_cpu[9];
    buffer_lea[10] = volatile_buffer_cpu[10];
    buffer_lea[11] = volatile_buffer_cpu[11];
    buffer_lea[12] = volatile_buffer_cpu[12];
    buffer_lea[13] = volatile_buffer_cpu[13];
    buffer_lea[14] = volatile_buffer_cpu[14];
    pos++;
  }
}



void loopcpy20() {
  s_start = 0;
  s_end = 0;
  pos = 0;
  for (uint16_t i = 0; i < REPEAT; ++i) {
    buffer_lea[0] = volatile_buffer_cpu[0];
    buffer_lea[1] = volatile_buffer_cpu[1];
    buffer_lea[2] = volatile_buffer_cpu[2];
    buffer_lea[3] = volatile_buffer_cpu[3];
    buffer_lea[4] = volatile_buffer_cpu[4];
    buffer_lea[5] = volatile_buffer_cpu[5];
    buffer_lea[6] = volatile_buffer_cpu[6];
    buffer_lea[7] = volatile_buffer_cpu[7];
    buffer_lea[8] = volatile_buffer_cpu[8];
    buffer_lea[9] = volatile_buffer_cpu[9];
    buffer_lea[10] = volatile_buffer_cpu[10];
    buffer_lea[11] = volatile_buffer_cpu[11];
    buffer_lea[12] = volatile_buffer_cpu[12];
    buffer_lea[13] = volatile_buffer_cpu[13];
    buffer_lea[14] = volatile_buffer_cpu[14];
    buffer_lea[15] = volatile_buffer_cpu[15];
    buffer_lea[16] = volatile_buffer_cpu[16];
    buffer_lea[17] = volatile_buffer_cpu[17];
    buffer_lea[18] = volatile_buffer_cpu[18];
    buffer_lea[19] = volatile_buffer_cpu[19];
    pos++;
  }
}



void loopcpy25() {
  s_start = 0;
  s_end = 0;
  pos = 0;
  for (uint16_t i = 0; i < REPEAT; ++i) {
    buffer_lea[0] = volatile_buffer_cpu[0];
    buffer_lea[1] = volatile_buffer_cpu[1];
    buffer_lea[2] = volatile_buffer_cpu[2];
    buffer_lea[3] = volatile_buffer_cpu[3];
    buffer_lea[4] = volatile_buffer_cpu[4];
    buffer_lea[5] = volatile_buffer_cpu[5];
    buffer_lea[6] = volatile_buffer_cpu[6];
    buffer_lea[7] = volatile_buffer_cpu[7];
    buffer_lea[8] = volatile_buffer_cpu[8];
    buffer_lea[9] = volatile_buffer_cpu[9];
    buffer_lea[10] = volatile_buffer_cpu[10];
    buffer_lea[11] = volatile_buffer_cpu[11];
    buffer_lea[12] = volatile_buffer_cpu[12];
    buffer_lea[13] = volatile_buffer_cpu[13];
    buffer_lea[14] = volatile_buffer_cpu[14];
    buffer_lea[15] = volatile_buffer_cpu[15];
    buffer_lea[16] = volatile_buffer_cpu[16];
    buffer_lea[17] = volatile_buffer_cpu[17];
    buffer_lea[18] = volatile_buffer_cpu[18];
    buffer_lea[19] = volatile_buffer_cpu[19];
    buffer_lea[20] = volatile_buffer_cpu[20];
    buffer_lea[21] = volatile_buffer_cpu[21];
    buffer_lea[22] = volatile_buffer_cpu[22];
    buffer_lea[23] = volatile_buffer_cpu[23];
    buffer_lea[24] = volatile_buffer_cpu[24];
    pos++;
  }
}



void loopcpy30() {
  s_start = 0;
  s_end = 0;
  pos = 0;
  for (uint16_t i = 0; i < REPEAT; ++i) {
    buffer_lea[0] = volatile_buffer_cpu[0];
    buffer_lea[1] = volatile_buffer_cpu[1];
    buffer_lea[2] = volatile_buffer_cpu[2];
    buffer_lea[3] = volatile_buffer_cpu[3];
    buffer_lea[4] = volatile_buffer_cpu[4];
    buffer_lea[5] = volatile_buffer_cpu[5];
    buffer_lea[6] = volatile_buffer_cpu[6];
    buffer_lea[7] = volatile_buffer_cpu[7];
    buffer_lea[8] = volatile_buffer_cpu[8];
    buffer_lea[9] = volatile_buffer_cpu[9];
    buffer_lea[10] = volatile_buffer_cpu[10];
    buffer_lea[11] = volatile_buffer_cpu[11];
    buffer_lea[12] = volatile_buffer_cpu[12];
    buffer_lea[13] = volatile_buffer_cpu[13];
    buffer_lea[14] = volatile_buffer_cpu[14];
    buffer_lea[15] = volatile_buffer_cpu[15];
    buffer_lea[16] = volatile_buffer_cpu[16];
    buffer_lea[17] = volatile_buffer_cpu[17];
    buffer_lea[18] = volatile_buffer_cpu[18];
    buffer_lea[19] = volatile_buffer_cpu[19];
    buffer_lea[20] = volatile_buffer_cpu[20];
    buffer_lea[21] = volatile_buffer_cpu[21];
    buffer_lea[22] = volatile_buffer_cpu[22];
    buffer_lea[23] = volatile_buffer_cpu[23];
    buffer_lea[24] = volatile_buffer_cpu[24];
    buffer_lea[25] = volatile_buffer_cpu[25];
    buffer_lea[26] = volatile_buffer_cpu[26];
    buffer_lea[27] = volatile_buffer_cpu[27];
    buffer_lea[28] = volatile_buffer_cpu[28];
    buffer_lea[29] = volatile_buffer_cpu[29];
    pos++;
  }
}



void loopcpy35() {
  s_start = 0;
  s_end = 0;
  pos = 0;
  for (uint16_t i = 0; i < REPEAT; ++i) {
    buffer_lea[0] = volatile_buffer_cpu[0];
    buffer_lea[1] = volatile_buffer_cpu[1];
    buffer_lea[2] = volatile_buffer_cpu[2];
    buffer_lea[3] = volatile_buffer_cpu[3];
    buffer_lea[4] = volatile_buffer_cpu[4];
    buffer_lea[5] = volatile_buffer_cpu[5];
    buffer_lea[6] = volatile_buffer_cpu[6];
    buffer_lea[7] = volatile_buffer_cpu[7];
    buffer_lea[8] = volatile_buffer_cpu[8];
    buffer_lea[9] = volatile_buffer_cpu[9];
    buffer_lea[10] = volatile_buffer_cpu[10];
    buffer_lea[11] = volatile_buffer_cpu[11];
    buffer_lea[12] = volatile_buffer_cpu[12];
    buffer_lea[13] = volatile_buffer_cpu[13];
    buffer_lea[14] = volatile_buffer_cpu[14];
    buffer_lea[15] = volatile_buffer_cpu[15];
    buffer_lea[16] = volatile_buffer_cpu[16];
    buffer_lea[17] = volatile_buffer_cpu[17];
    buffer_lea[18] = volatile_buffer_cpu[18];
    buffer_lea[19] = volatile_buffer_cpu[19];
    buffer_lea[20] = volatile_buffer_cpu[20];
    buffer_lea[21] = volatile_buffer_cpu[21];
    buffer_lea[22] = volatile_buffer_cpu[22];
    buffer_lea[23] = volatile_buffer_cpu[23];
    buffer_lea[24] = volatile_buffer_cpu[24];
    buffer_lea[25] = volatile_buffer_cpu[25];
    buffer_lea[26] = volatile_buffer_cpu[26];
    buffer_lea[27] = volatile_buffer_cpu[27];
    buffer_lea[28] = volatile_buffer_cpu[28];
    buffer_lea[29] = volatile_buffer_cpu[29];
    buffer_lea[30] = volatile_buffer_cpu[30];
    buffer_lea[31] = volatile_buffer_cpu[31];
    buffer_lea[32] = volatile_buffer_cpu[32];
    buffer_lea[33] = volatile_buffer_cpu[33];
    buffer_lea[34] = volatile_buffer_cpu[34];
    pos++;
  }
}



void loopcpy40() {
  s_start = 0;
  s_end = 0;
  pos = 0;
  for (uint16_t i = 0; i < REPEAT; ++i) {
    buffer_lea[0] = volatile_buffer_cpu[0];
    buffer_lea[1] = volatile_buffer_cpu[1];
    buffer_lea[2] = volatile_buffer_cpu[2];
    buffer_lea[3] = volatile_buffer_cpu[3];
    buffer_lea[4] = volatile_buffer_cpu[4];
    buffer_lea[5] = volatile_buffer_cpu[5];
    buffer_lea[6] = volatile_buffer_cpu[6];
    buffer_lea[7] = volatile_buffer_cpu[7];
    buffer_lea[8] = volatile_buffer_cpu[8];
    buffer_lea[9] = volatile_buffer_cpu[9];
    buffer_lea[10] = volatile_buffer_cpu[10];
    buffer_lea[11] = volatile_buffer_cpu[11];
    buffer_lea[12] = volatile_buffer_cpu[12];
    buffer_lea[13] = volatile_buffer_cpu[13];
    buffer_lea[14] = volatile_buffer_cpu[14];
    buffer_lea[15] = volatile_buffer_cpu[15];
    buffer_lea[16] = volatile_buffer_cpu[16];
    buffer_lea[17] = volatile_buffer_cpu[17];
    buffer_lea[18] = volatile_buffer_cpu[18];
    buffer_lea[19] = volatile_buffer_cpu[19];
    buffer_lea[20] = volatile_buffer_cpu[20];
    buffer_lea[21] = volatile_buffer_cpu[21];
    buffer_lea[22] = volatile_buffer_cpu[22];
    buffer_lea[23] = volatile_buffer_cpu[23];
    buffer_lea[24] = volatile_buffer_cpu[24];
    buffer_lea[25] = volatile_buffer_cpu[25];
    buffer_lea[26] = volatile_buffer_cpu[26];
    buffer_lea[27] = volatile_buffer_cpu[27];
    buffer_lea[28] = volatile_buffer_cpu[28];
    buffer_lea[29] = volatile_buffer_cpu[29];
    buffer_lea[30] = volatile_buffer_cpu[30];
    buffer_lea[31] = volatile_buffer_cpu[31];
    buffer_lea[32] = volatile_buffer_cpu[32];
    buffer_lea[33] = volatile_buffer_cpu[33];
    buffer_lea[34] = volatile_buffer_cpu[34];
    buffer_lea[35] = volatile_buffer_cpu[35];
    buffer_lea[36] = volatile_buffer_cpu[36];
    buffer_lea[37] = volatile_buffer_cpu[37];
    buffer_lea[38] = volatile_buffer_cpu[38];
    buffer_lea[39] = volatile_buffer_cpu[39];
    pos++;
  }
}



void loopcpy45() {
  s_start = 0;
  s_end = 0;
  pos = 0;
  for (uint16_t i = 0; i < REPEAT; ++i) {
    buffer_lea[0] = volatile_buffer_cpu[0];
    buffer_lea[1] = volatile_buffer_cpu[1];
    buffer_lea[2] = volatile_buffer_cpu[2];
    buffer_lea[3] = volatile_buffer_cpu[3];
    buffer_lea[4] = volatile_buffer_cpu[4];
    buffer_lea[5] = volatile_buffer_cpu[5];
    buffer_lea[6] = volatile_buffer_cpu[6];
    buffer_lea[7] = volatile_buffer_cpu[7];
    buffer_lea[8] = volatile_buffer_cpu[8];
    buffer_lea[9] = volatile_buffer_cpu[9];
    buffer_lea[10] = volatile_buffer_cpu[10];
    buffer_lea[11] = volatile_buffer_cpu[11];
    buffer_lea[12] = volatile_buffer_cpu[12];
    buffer_lea[13] = volatile_buffer_cpu[13];
    buffer_lea[14] = volatile_buffer_cpu[14];
    buffer_lea[15] = volatile_buffer_cpu[15];
    buffer_lea[16] = volatile_buffer_cpu[16];
    buffer_lea[17] = volatile_buffer_cpu[17];
    buffer_lea[18] = volatile_buffer_cpu[18];
    buffer_lea[19] = volatile_buffer_cpu[19];
    buffer_lea[20] = volatile_buffer_cpu[20];
    buffer_lea[21] = volatile_buffer_cpu[21];
    buffer_lea[22] = volatile_buffer_cpu[22];
    buffer_lea[23] = volatile_buffer_cpu[23];
    buffer_lea[24] = volatile_buffer_cpu[24];
    buffer_lea[25] = volatile_buffer_cpu[25];
    buffer_lea[26] = volatile_buffer_cpu[26];
    buffer_lea[27] = volatile_buffer_cpu[27];
    buffer_lea[28] = volatile_buffer_cpu[28];
    buffer_lea[29] = volatile_buffer_cpu[29];
    buffer_lea[30] = volatile_buffer_cpu[30];
    buffer_lea[31] = volatile_buffer_cpu[31];
    buffer_lea[32] = volatile_buffer_cpu[32];
    buffer_lea[33] = volatile_buffer_cpu[33];
    buffer_lea[34] = volatile_buffer_cpu[34];
    buffer_lea[35] = volatile_buffer_cpu[35];
    buffer_lea[36] = volatile_buffer_cpu[36];
    buffer_lea[37] = volatile_buffer_cpu[37];
    buffer_lea[38] = volatile_buffer_cpu[38];
    buffer_lea[39] = volatile_buffer_cpu[39];
    buffer_lea[40] = volatile_buffer_cpu[40];
    buffer_lea[41] = volatile_buffer_cpu[41];
    buffer_lea[42] = volatile_buffer_cpu[42];
    buffer_lea[43] = volatile_buffer_cpu[43];
    buffer_lea[44] = volatile_buffer_cpu[44];
    pos++;
  }
}



void loopcpy50() {
  s_start = 0;
  s_end = 0;
  pos = 0;
  for (uint16_t i = 0; i < REPEAT; ++i) {
    buffer_lea[0] = volatile_buffer_cpu[0];
    buffer_lea[1] = volatile_buffer_cpu[1];
    buffer_lea[2] = volatile_buffer_cpu[2];
    buffer_lea[3] = volatile_buffer_cpu[3];
    buffer_lea[4] = volatile_buffer_cpu[4];
    buffer_lea[5] = volatile_buffer_cpu[5];
    buffer_lea[6] = volatile_buffer_cpu[6];
    buffer_lea[7] = volatile_buffer_cpu[7];
    buffer_lea[8] = volatile_buffer_cpu[8];
    buffer_lea[9] = volatile_buffer_cpu[9];
    buffer_lea[10] = volatile_buffer_cpu[10];
    buffer_lea[11] = volatile_buffer_cpu[11];
    buffer_lea[12] = volatile_buffer_cpu[12];
    buffer_lea[13] = volatile_buffer_cpu[13];
    buffer_lea[14] = volatile_buffer_cpu[14];
    buffer_lea[15] = volatile_buffer_cpu[15];
    buffer_lea[16] = volatile_buffer_cpu[16];
    buffer_lea[17] = volatile_buffer_cpu[17];
    buffer_lea[18] = volatile_buffer_cpu[18];
    buffer_lea[19] = volatile_buffer_cpu[19];
    buffer_lea[20] = volatile_buffer_cpu[20];
    buffer_lea[21] = volatile_buffer_cpu[21];
    buffer_lea[22] = volatile_buffer_cpu[22];
    buffer_lea[23] = volatile_buffer_cpu[23];
    buffer_lea[24] = volatile_buffer_cpu[24];
    buffer_lea[25] = volatile_buffer_cpu[25];
    buffer_lea[26] = volatile_buffer_cpu[26];
    buffer_lea[27] = volatile_buffer_cpu[27];
    buffer_lea[28] = volatile_buffer_cpu[28];
    buffer_lea[29] = volatile_buffer_cpu[29];
    buffer_lea[30] = volatile_buffer_cpu[30];
    buffer_lea[31] = volatile_buffer_cpu[31];
    buffer_lea[32] = volatile_buffer_cpu[32];
    buffer_lea[33] = volatile_buffer_cpu[33];
    buffer_lea[34] = volatile_buffer_cpu[34];
    buffer_lea[35] = volatile_buffer_cpu[35];
    buffer_lea[36] = volatile_buffer_cpu[36];
    buffer_lea[37] = volatile_buffer_cpu[37];
    buffer_lea[38] = volatile_buffer_cpu[38];
    buffer_lea[39] = volatile_buffer_cpu[39];
    buffer_lea[40] = volatile_buffer_cpu[40];
    buffer_lea[41] = volatile_buffer_cpu[41];
    buffer_lea[42] = volatile_buffer_cpu[42];
    buffer_lea[43] = volatile_buffer_cpu[43];
    buffer_lea[44] = volatile_buffer_cpu[44];
    buffer_lea[45] = volatile_buffer_cpu[45];
    buffer_lea[46] = volatile_buffer_cpu[46];
    buffer_lea[47] = volatile_buffer_cpu[47];
    buffer_lea[48] = volatile_buffer_cpu[48];
    buffer_lea[49] = volatile_buffer_cpu[49];
    pos++;
  }
}



void loopcpy55() {
  s_start = 0;
  s_end = 0;
  pos = 0;
  for (uint16_t i = 0; i < REPEAT; ++i) {
    buffer_lea[0] = volatile_buffer_cpu[0];
    buffer_lea[1] = volatile_buffer_cpu[1];
    buffer_lea[2] = volatile_buffer_cpu[2];
    buffer_lea[3] = volatile_buffer_cpu[3];
    buffer_lea[4] = volatile_buffer_cpu[4];
    buffer_lea[5] = volatile_buffer_cpu[5];
    buffer_lea[6] = volatile_buffer_cpu[6];
    buffer_lea[7] = volatile_buffer_cpu[7];
    buffer_lea[8] = volatile_buffer_cpu[8];
    buffer_lea[9] = volatile_buffer_cpu[9];
    buffer_lea[10] = volatile_buffer_cpu[10];
    buffer_lea[11] = volatile_buffer_cpu[11];
    buffer_lea[12] = volatile_buffer_cpu[12];
    buffer_lea[13] = volatile_buffer_cpu[13];
    buffer_lea[14] = volatile_buffer_cpu[14];
    buffer_lea[15] = volatile_buffer_cpu[15];
    buffer_lea[16] = volatile_buffer_cpu[16];
    buffer_lea[17] = volatile_buffer_cpu[17];
    buffer_lea[18] = volatile_buffer_cpu[18];
    buffer_lea[19] = volatile_buffer_cpu[19];
    buffer_lea[20] = volatile_buffer_cpu[20];
    buffer_lea[21] = volatile_buffer_cpu[21];
    buffer_lea[22] = volatile_buffer_cpu[22];
    buffer_lea[23] = volatile_buffer_cpu[23];
    buffer_lea[24] = volatile_buffer_cpu[24];
    buffer_lea[25] = volatile_buffer_cpu[25];
    buffer_lea[26] = volatile_buffer_cpu[26];
    buffer_lea[27] = volatile_buffer_cpu[27];
    buffer_lea[28] = volatile_buffer_cpu[28];
    buffer_lea[29] = volatile_buffer_cpu[29];
    buffer_lea[30] = volatile_buffer_cpu[30];
    buffer_lea[31] = volatile_buffer_cpu[31];
    buffer_lea[32] = volatile_buffer_cpu[32];
    buffer_lea[33] = volatile_buffer_cpu[33];
    buffer_lea[34] = volatile_buffer_cpu[34];
    buffer_lea[35] = volatile_buffer_cpu[35];
    buffer_lea[36] = volatile_buffer_cpu[36];
    buffer_lea[37] = volatile_buffer_cpu[37];
    buffer_lea[38] = volatile_buffer_cpu[38];
    buffer_lea[39] = volatile_buffer_cpu[39];
    buffer_lea[40] = volatile_buffer_cpu[40];
    buffer_lea[41] = volatile_buffer_cpu[41];
    buffer_lea[42] = volatile_buffer_cpu[42];
    buffer_lea[43] = volatile_buffer_cpu[43];
    buffer_lea[44] = volatile_buffer_cpu[44];
    buffer_lea[45] = volatile_buffer_cpu[45];
    buffer_lea[46] = volatile_buffer_cpu[46];
    buffer_lea[47] = volatile_buffer_cpu[47];
    buffer_lea[48] = volatile_buffer_cpu[48];
    buffer_lea[49] = volatile_buffer_cpu[49];
    buffer_lea[50] = volatile_buffer_cpu[50];
    buffer_lea[51] = volatile_buffer_cpu[51];
    buffer_lea[52] = volatile_buffer_cpu[52];
    buffer_lea[53] = volatile_buffer_cpu[53];
    buffer_lea[54] = volatile_buffer_cpu[54];
    pos++;
  }
}



void loopcpy60() {
  s_start = 0;
  s_end = 0;
  pos = 0;
  for (uint16_t i = 0; i < REPEAT; ++i) {
    buffer_lea[0] = volatile_buffer_cpu[0];
    buffer_lea[1] = volatile_buffer_cpu[1];
    buffer_lea[2] = volatile_buffer_cpu[2];
    buffer_lea[3] = volatile_buffer_cpu[3];
    buffer_lea[4] = volatile_buffer_cpu[4];
    buffer_lea[5] = volatile_buffer_cpu[5];
    buffer_lea[6] = volatile_buffer_cpu[6];
    buffer_lea[7] = volatile_buffer_cpu[7];
    buffer_lea[8] = volatile_buffer_cpu[8];
    buffer_lea[9] = volatile_buffer_cpu[9];
    buffer_lea[10] = volatile_buffer_cpu[10];
    buffer_lea[11] = volatile_buffer_cpu[11];
    buffer_lea[12] = volatile_buffer_cpu[12];
    buffer_lea[13] = volatile_buffer_cpu[13];
    buffer_lea[14] = volatile_buffer_cpu[14];
    buffer_lea[15] = volatile_buffer_cpu[15];
    buffer_lea[16] = volatile_buffer_cpu[16];
    buffer_lea[17] = volatile_buffer_cpu[17];
    buffer_lea[18] = volatile_buffer_cpu[18];
    buffer_lea[19] = volatile_buffer_cpu[19];
    buffer_lea[20] = volatile_buffer_cpu[20];
    buffer_lea[21] = volatile_buffer_cpu[21];
    buffer_lea[22] = volatile_buffer_cpu[22];
    buffer_lea[23] = volatile_buffer_cpu[23];
    buffer_lea[24] = volatile_buffer_cpu[24];
    buffer_lea[25] = volatile_buffer_cpu[25];
    buffer_lea[26] = volatile_buffer_cpu[26];
    buffer_lea[27] = volatile_buffer_cpu[27];
    buffer_lea[28] = volatile_buffer_cpu[28];
    buffer_lea[29] = volatile_buffer_cpu[29];
    buffer_lea[30] = volatile_buffer_cpu[30];
    buffer_lea[31] = volatile_buffer_cpu[31];
    buffer_lea[32] = volatile_buffer_cpu[32];
    buffer_lea[33] = volatile_buffer_cpu[33];
    buffer_lea[34] = volatile_buffer_cpu[34];
    buffer_lea[35] = volatile_buffer_cpu[35];
    buffer_lea[36] = volatile_buffer_cpu[36];
    buffer_lea[37] = volatile_buffer_cpu[37];
    buffer_lea[38] = volatile_buffer_cpu[38];
    buffer_lea[39] = volatile_buffer_cpu[39];
    buffer_lea[40] = volatile_buffer_cpu[40];
    buffer_lea[41] = volatile_buffer_cpu[41];
    buffer_lea[42] = volatile_buffer_cpu[42];
    buffer_lea[43] = volatile_buffer_cpu[43];
    buffer_lea[44] = volatile_buffer_cpu[44];
    buffer_lea[45] = volatile_buffer_cpu[45];
    buffer_lea[46] = volatile_buffer_cpu[46];
    buffer_lea[47] = volatile_buffer_cpu[47];
    buffer_lea[48] = volatile_buffer_cpu[48];
    buffer_lea[49] = volatile_buffer_cpu[49];
    buffer_lea[50] = volatile_buffer_cpu[50];
    buffer_lea[51] = volatile_buffer_cpu[51];
    buffer_lea[52] = volatile_buffer_cpu[52];
    buffer_lea[53] = volatile_buffer_cpu[53];
    buffer_lea[54] = volatile_buffer_cpu[54];
    buffer_lea[55] = volatile_buffer_cpu[55];
    buffer_lea[56] = volatile_buffer_cpu[56];
    buffer_lea[57] = volatile_buffer_cpu[57];
    buffer_lea[58] = volatile_buffer_cpu[58];
    buffer_lea[59] = volatile_buffer_cpu[59];
    pos++;
  }
}



void loopcpy65() {
  s_start = 0;
  s_end = 0;
  pos = 0;
  for (uint16_t i = 0; i < REPEAT; ++i) {
    buffer_lea[0] = volatile_buffer_cpu[0];
    buffer_lea[1] = volatile_buffer_cpu[1];
    buffer_lea[2] = volatile_buffer_cpu[2];
    buffer_lea[3] = volatile_buffer_cpu[3];
    buffer_lea[4] = volatile_buffer_cpu[4];
    buffer_lea[5] = volatile_buffer_cpu[5];
    buffer_lea[6] = volatile_buffer_cpu[6];
    buffer_lea[7] = volatile_buffer_cpu[7];
    buffer_lea[8] = volatile_buffer_cpu[8];
    buffer_lea[9] = volatile_buffer_cpu[9];
    buffer_lea[10] = volatile_buffer_cpu[10];
    buffer_lea[11] = volatile_buffer_cpu[11];
    buffer_lea[12] = volatile_buffer_cpu[12];
    buffer_lea[13] = volatile_buffer_cpu[13];
    buffer_lea[14] = volatile_buffer_cpu[14];
    buffer_lea[15] = volatile_buffer_cpu[15];
    buffer_lea[16] = volatile_buffer_cpu[16];
    buffer_lea[17] = volatile_buffer_cpu[17];
    buffer_lea[18] = volatile_buffer_cpu[18];
    buffer_lea[19] = volatile_buffer_cpu[19];
    buffer_lea[20] = volatile_buffer_cpu[20];
    buffer_lea[21] = volatile_buffer_cpu[21];
    buffer_lea[22] = volatile_buffer_cpu[22];
    buffer_lea[23] = volatile_buffer_cpu[23];
    buffer_lea[24] = volatile_buffer_cpu[24];
    buffer_lea[25] = volatile_buffer_cpu[25];
    buffer_lea[26] = volatile_buffer_cpu[26];
    buffer_lea[27] = volatile_buffer_cpu[27];
    buffer_lea[28] = volatile_buffer_cpu[28];
    buffer_lea[29] = volatile_buffer_cpu[29];
    buffer_lea[30] = volatile_buffer_cpu[30];
    buffer_lea[31] = volatile_buffer_cpu[31];
    buffer_lea[32] = volatile_buffer_cpu[32];
    buffer_lea[33] = volatile_buffer_cpu[33];
    buffer_lea[34] = volatile_buffer_cpu[34];
    buffer_lea[35] = volatile_buffer_cpu[35];
    buffer_lea[36] = volatile_buffer_cpu[36];
    buffer_lea[37] = volatile_buffer_cpu[37];
    buffer_lea[38] = volatile_buffer_cpu[38];
    buffer_lea[39] = volatile_buffer_cpu[39];
    buffer_lea[40] = volatile_buffer_cpu[40];
    buffer_lea[41] = volatile_buffer_cpu[41];
    buffer_lea[42] = volatile_buffer_cpu[42];
    buffer_lea[43] = volatile_buffer_cpu[43];
    buffer_lea[44] = volatile_buffer_cpu[44];
    buffer_lea[45] = volatile_buffer_cpu[45];
    buffer_lea[46] = volatile_buffer_cpu[46];
    buffer_lea[47] = volatile_buffer_cpu[47];
    buffer_lea[48] = volatile_buffer_cpu[48];
    buffer_lea[49] = volatile_buffer_cpu[49];
    buffer_lea[50] = volatile_buffer_cpu[50];
    buffer_lea[51] = volatile_buffer_cpu[51];
    buffer_lea[52] = volatile_buffer_cpu[52];
    buffer_lea[53] = volatile_buffer_cpu[53];
    buffer_lea[54] = volatile_buffer_cpu[54];
    buffer_lea[55] = volatile_buffer_cpu[55];
    buffer_lea[56] = volatile_buffer_cpu[56];
    buffer_lea[57] = volatile_buffer_cpu[57];
    buffer_lea[58] = volatile_buffer_cpu[58];
    buffer_lea[59] = volatile_buffer_cpu[59];
    buffer_lea[60] = volatile_buffer_cpu[60];
    buffer_lea[61] = volatile_buffer_cpu[61];
    buffer_lea[62] = volatile_buffer_cpu[62];
    buffer_lea[63] = volatile_buffer_cpu[63];
    buffer_lea[64] = volatile_buffer_cpu[64];
    pos++;
  }
}



void loopcpy70() {
  s_start = 0;
  s_end = 0;
  pos = 0;
  for (uint16_t i = 0; i < REPEAT; ++i) {
    buffer_lea[0] = volatile_buffer_cpu[0];
    buffer_lea[1] = volatile_buffer_cpu[1];
    buffer_lea[2] = volatile_buffer_cpu[2];
    buffer_lea[3] = volatile_buffer_cpu[3];
    buffer_lea[4] = volatile_buffer_cpu[4];
    buffer_lea[5] = volatile_buffer_cpu[5];
    buffer_lea[6] = volatile_buffer_cpu[6];
    buffer_lea[7] = volatile_buffer_cpu[7];
    buffer_lea[8] = volatile_buffer_cpu[8];
    buffer_lea[9] = volatile_buffer_cpu[9];
    buffer_lea[10] = volatile_buffer_cpu[10];
    buffer_lea[11] = volatile_buffer_cpu[11];
    buffer_lea[12] = volatile_buffer_cpu[12];
    buffer_lea[13] = volatile_buffer_cpu[13];
    buffer_lea[14] = volatile_buffer_cpu[14];
    buffer_lea[15] = volatile_buffer_cpu[15];
    buffer_lea[16] = volatile_buffer_cpu[16];
    buffer_lea[17] = volatile_buffer_cpu[17];
    buffer_lea[18] = volatile_buffer_cpu[18];
    buffer_lea[19] = volatile_buffer_cpu[19];
    buffer_lea[20] = volatile_buffer_cpu[20];
    buffer_lea[21] = volatile_buffer_cpu[21];
    buffer_lea[22] = volatile_buffer_cpu[22];
    buffer_lea[23] = volatile_buffer_cpu[23];
    buffer_lea[24] = volatile_buffer_cpu[24];
    buffer_lea[25] = volatile_buffer_cpu[25];
    buffer_lea[26] = volatile_buffer_cpu[26];
    buffer_lea[27] = volatile_buffer_cpu[27];
    buffer_lea[28] = volatile_buffer_cpu[28];
    buffer_lea[29] = volatile_buffer_cpu[29];
    buffer_lea[30] = volatile_buffer_cpu[30];
    buffer_lea[31] = volatile_buffer_cpu[31];
    buffer_lea[32] = volatile_buffer_cpu[32];
    buffer_lea[33] = volatile_buffer_cpu[33];
    buffer_lea[34] = volatile_buffer_cpu[34];
    buffer_lea[35] = volatile_buffer_cpu[35];
    buffer_lea[36] = volatile_buffer_cpu[36];
    buffer_lea[37] = volatile_buffer_cpu[37];
    buffer_lea[38] = volatile_buffer_cpu[38];
    buffer_lea[39] = volatile_buffer_cpu[39];
    buffer_lea[40] = volatile_buffer_cpu[40];
    buffer_lea[41] = volatile_buffer_cpu[41];
    buffer_lea[42] = volatile_buffer_cpu[42];
    buffer_lea[43] = volatile_buffer_cpu[43];
    buffer_lea[44] = volatile_buffer_cpu[44];
    buffer_lea[45] = volatile_buffer_cpu[45];
    buffer_lea[46] = volatile_buffer_cpu[46];
    buffer_lea[47] = volatile_buffer_cpu[47];
    buffer_lea[48] = volatile_buffer_cpu[48];
    buffer_lea[49] = volatile_buffer_cpu[49];
    buffer_lea[50] = volatile_buffer_cpu[50];
    buffer_lea[51] = volatile_buffer_cpu[51];
    buffer_lea[52] = volatile_buffer_cpu[52];
    buffer_lea[53] = volatile_buffer_cpu[53];
    buffer_lea[54] = volatile_buffer_cpu[54];
    buffer_lea[55] = volatile_buffer_cpu[55];
    buffer_lea[56] = volatile_buffer_cpu[56];
    buffer_lea[57] = volatile_buffer_cpu[57];
    buffer_lea[58] = volatile_buffer_cpu[58];
    buffer_lea[59] = volatile_buffer_cpu[59];
    buffer_lea[60] = volatile_buffer_cpu[60];
    buffer_lea[61] = volatile_buffer_cpu[61];
    buffer_lea[62] = volatile_buffer_cpu[62];
    buffer_lea[63] = volatile_buffer_cpu[63];
    buffer_lea[64] = volatile_buffer_cpu[64];
    buffer_lea[65] = volatile_buffer_cpu[65];
    buffer_lea[66] = volatile_buffer_cpu[66];
    buffer_lea[67] = volatile_buffer_cpu[67];
    buffer_lea[68] = volatile_buffer_cpu[68];
    buffer_lea[69] = volatile_buffer_cpu[69];
    pos++;
  }
}



void loopcpy75() {
  s_start = 0;
  s_end = 0;
  pos = 0;
  for (uint16_t i = 0; i < REPEAT; ++i) {
    buffer_lea[0] = volatile_buffer_cpu[0];
    buffer_lea[1] = volatile_buffer_cpu[1];
    buffer_lea[2] = volatile_buffer_cpu[2];
    buffer_lea[3] = volatile_buffer_cpu[3];
    buffer_lea[4] = volatile_buffer_cpu[4];
    buffer_lea[5] = volatile_buffer_cpu[5];
    buffer_lea[6] = volatile_buffer_cpu[6];
    buffer_lea[7] = volatile_buffer_cpu[7];
    buffer_lea[8] = volatile_buffer_cpu[8];
    buffer_lea[9] = volatile_buffer_cpu[9];
    buffer_lea[10] = volatile_buffer_cpu[10];
    buffer_lea[11] = volatile_buffer_cpu[11];
    buffer_lea[12] = volatile_buffer_cpu[12];
    buffer_lea[13] = volatile_buffer_cpu[13];
    buffer_lea[14] = volatile_buffer_cpu[14];
    buffer_lea[15] = volatile_buffer_cpu[15];
    buffer_lea[16] = volatile_buffer_cpu[16];
    buffer_lea[17] = volatile_buffer_cpu[17];
    buffer_lea[18] = volatile_buffer_cpu[18];
    buffer_lea[19] = volatile_buffer_cpu[19];
    buffer_lea[20] = volatile_buffer_cpu[20];
    buffer_lea[21] = volatile_buffer_cpu[21];
    buffer_lea[22] = volatile_buffer_cpu[22];
    buffer_lea[23] = volatile_buffer_cpu[23];
    buffer_lea[24] = volatile_buffer_cpu[24];
    buffer_lea[25] = volatile_buffer_cpu[25];
    buffer_lea[26] = volatile_buffer_cpu[26];
    buffer_lea[27] = volatile_buffer_cpu[27];
    buffer_lea[28] = volatile_buffer_cpu[28];
    buffer_lea[29] = volatile_buffer_cpu[29];
    buffer_lea[30] = volatile_buffer_cpu[30];
    buffer_lea[31] = volatile_buffer_cpu[31];
    buffer_lea[32] = volatile_buffer_cpu[32];
    buffer_lea[33] = volatile_buffer_cpu[33];
    buffer_lea[34] = volatile_buffer_cpu[34];
    buffer_lea[35] = volatile_buffer_cpu[35];
    buffer_lea[36] = volatile_buffer_cpu[36];
    buffer_lea[37] = volatile_buffer_cpu[37];
    buffer_lea[38] = volatile_buffer_cpu[38];
    buffer_lea[39] = volatile_buffer_cpu[39];
    buffer_lea[40] = volatile_buffer_cpu[40];
    buffer_lea[41] = volatile_buffer_cpu[41];
    buffer_lea[42] = volatile_buffer_cpu[42];
    buffer_lea[43] = volatile_buffer_cpu[43];
    buffer_lea[44] = volatile_buffer_cpu[44];
    buffer_lea[45] = volatile_buffer_cpu[45];
    buffer_lea[46] = volatile_buffer_cpu[46];
    buffer_lea[47] = volatile_buffer_cpu[47];
    buffer_lea[48] = volatile_buffer_cpu[48];
    buffer_lea[49] = volatile_buffer_cpu[49];
    buffer_lea[50] = volatile_buffer_cpu[50];
    buffer_lea[51] = volatile_buffer_cpu[51];
    buffer_lea[52] = volatile_buffer_cpu[52];
    buffer_lea[53] = volatile_buffer_cpu[53];
    buffer_lea[54] = volatile_buffer_cpu[54];
    buffer_lea[55] = volatile_buffer_cpu[55];
    buffer_lea[56] = volatile_buffer_cpu[56];
    buffer_lea[57] = volatile_buffer_cpu[57];
    buffer_lea[58] = volatile_buffer_cpu[58];
    buffer_lea[59] = volatile_buffer_cpu[59];
    buffer_lea[60] = volatile_buffer_cpu[60];
    buffer_lea[61] = volatile_buffer_cpu[61];
    buffer_lea[62] = volatile_buffer_cpu[62];
    buffer_lea[63] = volatile_buffer_cpu[63];
    buffer_lea[64] = volatile_buffer_cpu[64];
    buffer_lea[65] = volatile_buffer_cpu[65];
    buffer_lea[66] = volatile_buffer_cpu[66];
    buffer_lea[67] = volatile_buffer_cpu[67];
    buffer_lea[68] = volatile_buffer_cpu[68];
    buffer_lea[69] = volatile_buffer_cpu[69];
    buffer_lea[70] = volatile_buffer_cpu[70];
    buffer_lea[71] = volatile_buffer_cpu[71];
    buffer_lea[72] = volatile_buffer_cpu[72];
    buffer_lea[73] = volatile_buffer_cpu[73];
    buffer_lea[74] = volatile_buffer_cpu[74];
    pos++;
  }
}



void loopcpy80() {
  s_start = 0;
  s_end = 0;
  pos = 0;
  for (uint16_t i = 0; i < REPEAT; ++i) {
    buffer_lea[0] = volatile_buffer_cpu[0];
    buffer_lea[1] = volatile_buffer_cpu[1];
    buffer_lea[2] = volatile_buffer_cpu[2];
    buffer_lea[3] = volatile_buffer_cpu[3];
    buffer_lea[4] = volatile_buffer_cpu[4];
    buffer_lea[5] = volatile_buffer_cpu[5];
    buffer_lea[6] = volatile_buffer_cpu[6];
    buffer_lea[7] = volatile_buffer_cpu[7];
    buffer_lea[8] = volatile_buffer_cpu[8];
    buffer_lea[9] = volatile_buffer_cpu[9];
    buffer_lea[10] = volatile_buffer_cpu[10];
    buffer_lea[11] = volatile_buffer_cpu[11];
    buffer_lea[12] = volatile_buffer_cpu[12];
    buffer_lea[13] = volatile_buffer_cpu[13];
    buffer_lea[14] = volatile_buffer_cpu[14];
    buffer_lea[15] = volatile_buffer_cpu[15];
    buffer_lea[16] = volatile_buffer_cpu[16];
    buffer_lea[17] = volatile_buffer_cpu[17];
    buffer_lea[18] = volatile_buffer_cpu[18];
    buffer_lea[19] = volatile_buffer_cpu[19];
    buffer_lea[20] = volatile_buffer_cpu[20];
    buffer_lea[21] = volatile_buffer_cpu[21];
    buffer_lea[22] = volatile_buffer_cpu[22];
    buffer_lea[23] = volatile_buffer_cpu[23];
    buffer_lea[24] = volatile_buffer_cpu[24];
    buffer_lea[25] = volatile_buffer_cpu[25];
    buffer_lea[26] = volatile_buffer_cpu[26];
    buffer_lea[27] = volatile_buffer_cpu[27];
    buffer_lea[28] = volatile_buffer_cpu[28];
    buffer_lea[29] = volatile_buffer_cpu[29];
    buffer_lea[30] = volatile_buffer_cpu[30];
    buffer_lea[31] = volatile_buffer_cpu[31];
    buffer_lea[32] = volatile_buffer_cpu[32];
    buffer_lea[33] = volatile_buffer_cpu[33];
    buffer_lea[34] = volatile_buffer_cpu[34];
    buffer_lea[35] = volatile_buffer_cpu[35];
    buffer_lea[36] = volatile_buffer_cpu[36];
    buffer_lea[37] = volatile_buffer_cpu[37];
    buffer_lea[38] = volatile_buffer_cpu[38];
    buffer_lea[39] = volatile_buffer_cpu[39];
    buffer_lea[40] = volatile_buffer_cpu[40];
    buffer_lea[41] = volatile_buffer_cpu[41];
    buffer_lea[42] = volatile_buffer_cpu[42];
    buffer_lea[43] = volatile_buffer_cpu[43];
    buffer_lea[44] = volatile_buffer_cpu[44];
    buffer_lea[45] = volatile_buffer_cpu[45];
    buffer_lea[46] = volatile_buffer_cpu[46];
    buffer_lea[47] = volatile_buffer_cpu[47];
    buffer_lea[48] = volatile_buffer_cpu[48];
    buffer_lea[49] = volatile_buffer_cpu[49];
    buffer_lea[50] = volatile_buffer_cpu[50];
    buffer_lea[51] = volatile_buffer_cpu[51];
    buffer_lea[52] = volatile_buffer_cpu[52];
    buffer_lea[53] = volatile_buffer_cpu[53];
    buffer_lea[54] = volatile_buffer_cpu[54];
    buffer_lea[55] = volatile_buffer_cpu[55];
    buffer_lea[56] = volatile_buffer_cpu[56];
    buffer_lea[57] = volatile_buffer_cpu[57];
    buffer_lea[58] = volatile_buffer_cpu[58];
    buffer_lea[59] = volatile_buffer_cpu[59];
    buffer_lea[60] = volatile_buffer_cpu[60];
    buffer_lea[61] = volatile_buffer_cpu[61];
    buffer_lea[62] = volatile_buffer_cpu[62];
    buffer_lea[63] = volatile_buffer_cpu[63];
    buffer_lea[64] = volatile_buffer_cpu[64];
    buffer_lea[65] = volatile_buffer_cpu[65];
    buffer_lea[66] = volatile_buffer_cpu[66];
    buffer_lea[67] = volatile_buffer_cpu[67];
    buffer_lea[68] = volatile_buffer_cpu[68];
    buffer_lea[69] = volatile_buffer_cpu[69];
    buffer_lea[70] = volatile_buffer_cpu[70];
    buffer_lea[71] = volatile_buffer_cpu[71];
    buffer_lea[72] = volatile_buffer_cpu[72];
    buffer_lea[73] = volatile_buffer_cpu[73];
    buffer_lea[74] = volatile_buffer_cpu[74];
    buffer_lea[75] = volatile_buffer_cpu[75];
    buffer_lea[76] = volatile_buffer_cpu[76];
    buffer_lea[77] = volatile_buffer_cpu[77];
    buffer_lea[78] = volatile_buffer_cpu[78];
    buffer_lea[79] = volatile_buffer_cpu[79];
    pos++;
  }
}



void loopcpy85() {
  s_start = 0;
  s_end = 0;
  pos = 0;
  for (uint16_t i = 0; i < REPEAT; ++i) {
    buffer_lea[0] = volatile_buffer_cpu[0];
    buffer_lea[1] = volatile_buffer_cpu[1];
    buffer_lea[2] = volatile_buffer_cpu[2];
    buffer_lea[3] = volatile_buffer_cpu[3];
    buffer_lea[4] = volatile_buffer_cpu[4];
    buffer_lea[5] = volatile_buffer_cpu[5];
    buffer_lea[6] = volatile_buffer_cpu[6];
    buffer_lea[7] = volatile_buffer_cpu[7];
    buffer_lea[8] = volatile_buffer_cpu[8];
    buffer_lea[9] = volatile_buffer_cpu[9];
    buffer_lea[10] = volatile_buffer_cpu[10];
    buffer_lea[11] = volatile_buffer_cpu[11];
    buffer_lea[12] = volatile_buffer_cpu[12];
    buffer_lea[13] = volatile_buffer_cpu[13];
    buffer_lea[14] = volatile_buffer_cpu[14];
    buffer_lea[15] = volatile_buffer_cpu[15];
    buffer_lea[16] = volatile_buffer_cpu[16];
    buffer_lea[17] = volatile_buffer_cpu[17];
    buffer_lea[18] = volatile_buffer_cpu[18];
    buffer_lea[19] = volatile_buffer_cpu[19];
    buffer_lea[20] = volatile_buffer_cpu[20];
    buffer_lea[21] = volatile_buffer_cpu[21];
    buffer_lea[22] = volatile_buffer_cpu[22];
    buffer_lea[23] = volatile_buffer_cpu[23];
    buffer_lea[24] = volatile_buffer_cpu[24];
    buffer_lea[25] = volatile_buffer_cpu[25];
    buffer_lea[26] = volatile_buffer_cpu[26];
    buffer_lea[27] = volatile_buffer_cpu[27];
    buffer_lea[28] = volatile_buffer_cpu[28];
    buffer_lea[29] = volatile_buffer_cpu[29];
    buffer_lea[30] = volatile_buffer_cpu[30];
    buffer_lea[31] = volatile_buffer_cpu[31];
    buffer_lea[32] = volatile_buffer_cpu[32];
    buffer_lea[33] = volatile_buffer_cpu[33];
    buffer_lea[34] = volatile_buffer_cpu[34];
    buffer_lea[35] = volatile_buffer_cpu[35];
    buffer_lea[36] = volatile_buffer_cpu[36];
    buffer_lea[37] = volatile_buffer_cpu[37];
    buffer_lea[38] = volatile_buffer_cpu[38];
    buffer_lea[39] = volatile_buffer_cpu[39];
    buffer_lea[40] = volatile_buffer_cpu[40];
    buffer_lea[41] = volatile_buffer_cpu[41];
    buffer_lea[42] = volatile_buffer_cpu[42];
    buffer_lea[43] = volatile_buffer_cpu[43];
    buffer_lea[44] = volatile_buffer_cpu[44];
    buffer_lea[45] = volatile_buffer_cpu[45];
    buffer_lea[46] = volatile_buffer_cpu[46];
    buffer_lea[47] = volatile_buffer_cpu[47];
    buffer_lea[48] = volatile_buffer_cpu[48];
    buffer_lea[49] = volatile_buffer_cpu[49];
    buffer_lea[50] = volatile_buffer_cpu[50];
    buffer_lea[51] = volatile_buffer_cpu[51];
    buffer_lea[52] = volatile_buffer_cpu[52];
    buffer_lea[53] = volatile_buffer_cpu[53];
    buffer_lea[54] = volatile_buffer_cpu[54];
    buffer_lea[55] = volatile_buffer_cpu[55];
    buffer_lea[56] = volatile_buffer_cpu[56];
    buffer_lea[57] = volatile_buffer_cpu[57];
    buffer_lea[58] = volatile_buffer_cpu[58];
    buffer_lea[59] = volatile_buffer_cpu[59];
    buffer_lea[60] = volatile_buffer_cpu[60];
    buffer_lea[61] = volatile_buffer_cpu[61];
    buffer_lea[62] = volatile_buffer_cpu[62];
    buffer_lea[63] = volatile_buffer_cpu[63];
    buffer_lea[64] = volatile_buffer_cpu[64];
    buffer_lea[65] = volatile_buffer_cpu[65];
    buffer_lea[66] = volatile_buffer_cpu[66];
    buffer_lea[67] = volatile_buffer_cpu[67];
    buffer_lea[68] = volatile_buffer_cpu[68];
    buffer_lea[69] = volatile_buffer_cpu[69];
    buffer_lea[70] = volatile_buffer_cpu[70];
    buffer_lea[71] = volatile_buffer_cpu[71];
    buffer_lea[72] = volatile_buffer_cpu[72];
    buffer_lea[73] = volatile_buffer_cpu[73];
    buffer_lea[74] = volatile_buffer_cpu[74];
    buffer_lea[75] = volatile_buffer_cpu[75];
    buffer_lea[76] = volatile_buffer_cpu[76];
    buffer_lea[77] = volatile_buffer_cpu[77];
    buffer_lea[78] = volatile_buffer_cpu[78];
    buffer_lea[79] = volatile_buffer_cpu[79];
    buffer_lea[80] = volatile_buffer_cpu[80];
    buffer_lea[81] = volatile_buffer_cpu[81];
    buffer_lea[82] = volatile_buffer_cpu[82];
    buffer_lea[83] = volatile_buffer_cpu[83];
    buffer_lea[84] = volatile_buffer_cpu[84];
    pos++;
  }
}



void loopcpy90() {
  s_start = 0;
  s_end = 0;
  pos = 0;
  for (uint16_t i = 0; i < REPEAT; ++i) {
    buffer_lea[0] = volatile_buffer_cpu[0];
    buffer_lea[1] = volatile_buffer_cpu[1];
    buffer_lea[2] = volatile_buffer_cpu[2];
    buffer_lea[3] = volatile_buffer_cpu[3];
    buffer_lea[4] = volatile_buffer_cpu[4];
    buffer_lea[5] = volatile_buffer_cpu[5];
    buffer_lea[6] = volatile_buffer_cpu[6];
    buffer_lea[7] = volatile_buffer_cpu[7];
    buffer_lea[8] = volatile_buffer_cpu[8];
    buffer_lea[9] = volatile_buffer_cpu[9];
    buffer_lea[10] = volatile_buffer_cpu[10];
    buffer_lea[11] = volatile_buffer_cpu[11];
    buffer_lea[12] = volatile_buffer_cpu[12];
    buffer_lea[13] = volatile_buffer_cpu[13];
    buffer_lea[14] = volatile_buffer_cpu[14];
    buffer_lea[15] = volatile_buffer_cpu[15];
    buffer_lea[16] = volatile_buffer_cpu[16];
    buffer_lea[17] = volatile_buffer_cpu[17];
    buffer_lea[18] = volatile_buffer_cpu[18];
    buffer_lea[19] = volatile_buffer_cpu[19];
    buffer_lea[20] = volatile_buffer_cpu[20];
    buffer_lea[21] = volatile_buffer_cpu[21];
    buffer_lea[22] = volatile_buffer_cpu[22];
    buffer_lea[23] = volatile_buffer_cpu[23];
    buffer_lea[24] = volatile_buffer_cpu[24];
    buffer_lea[25] = volatile_buffer_cpu[25];
    buffer_lea[26] = volatile_buffer_cpu[26];
    buffer_lea[27] = volatile_buffer_cpu[27];
    buffer_lea[28] = volatile_buffer_cpu[28];
    buffer_lea[29] = volatile_buffer_cpu[29];
    buffer_lea[30] = volatile_buffer_cpu[30];
    buffer_lea[31] = volatile_buffer_cpu[31];
    buffer_lea[32] = volatile_buffer_cpu[32];
    buffer_lea[33] = volatile_buffer_cpu[33];
    buffer_lea[34] = volatile_buffer_cpu[34];
    buffer_lea[35] = volatile_buffer_cpu[35];
    buffer_lea[36] = volatile_buffer_cpu[36];
    buffer_lea[37] = volatile_buffer_cpu[37];
    buffer_lea[38] = volatile_buffer_cpu[38];
    buffer_lea[39] = volatile_buffer_cpu[39];
    buffer_lea[40] = volatile_buffer_cpu[40];
    buffer_lea[41] = volatile_buffer_cpu[41];
    buffer_lea[42] = volatile_buffer_cpu[42];
    buffer_lea[43] = volatile_buffer_cpu[43];
    buffer_lea[44] = volatile_buffer_cpu[44];
    buffer_lea[45] = volatile_buffer_cpu[45];
    buffer_lea[46] = volatile_buffer_cpu[46];
    buffer_lea[47] = volatile_buffer_cpu[47];
    buffer_lea[48] = volatile_buffer_cpu[48];
    buffer_lea[49] = volatile_buffer_cpu[49];
    buffer_lea[50] = volatile_buffer_cpu[50];
    buffer_lea[51] = volatile_buffer_cpu[51];
    buffer_lea[52] = volatile_buffer_cpu[52];
    buffer_lea[53] = volatile_buffer_cpu[53];
    buffer_lea[54] = volatile_buffer_cpu[54];
    buffer_lea[55] = volatile_buffer_cpu[55];
    buffer_lea[56] = volatile_buffer_cpu[56];
    buffer_lea[57] = volatile_buffer_cpu[57];
    buffer_lea[58] = volatile_buffer_cpu[58];
    buffer_lea[59] = volatile_buffer_cpu[59];
    buffer_lea[60] = volatile_buffer_cpu[60];
    buffer_lea[61] = volatile_buffer_cpu[61];
    buffer_lea[62] = volatile_buffer_cpu[62];
    buffer_lea[63] = volatile_buffer_cpu[63];
    buffer_lea[64] = volatile_buffer_cpu[64];
    buffer_lea[65] = volatile_buffer_cpu[65];
    buffer_lea[66] = volatile_buffer_cpu[66];
    buffer_lea[67] = volatile_buffer_cpu[67];
    buffer_lea[68] = volatile_buffer_cpu[68];
    buffer_lea[69] = volatile_buffer_cpu[69];
    buffer_lea[70] = volatile_buffer_cpu[70];
    buffer_lea[71] = volatile_buffer_cpu[71];
    buffer_lea[72] = volatile_buffer_cpu[72];
    buffer_lea[73] = volatile_buffer_cpu[73];
    buffer_lea[74] = volatile_buffer_cpu[74];
    buffer_lea[75] = volatile_buffer_cpu[75];
    buffer_lea[76] = volatile_buffer_cpu[76];
    buffer_lea[77] = volatile_buffer_cpu[77];
    buffer_lea[78] = volatile_buffer_cpu[78];
    buffer_lea[79] = volatile_buffer_cpu[79];
    buffer_lea[80] = volatile_buffer_cpu[80];
    buffer_lea[81] = volatile_buffer_cpu[81];
    buffer_lea[82] = volatile_buffer_cpu[82];
    buffer_lea[83] = volatile_buffer_cpu[83];
    buffer_lea[84] = volatile_buffer_cpu[84];
    buffer_lea[85] = volatile_buffer_cpu[85];
    buffer_lea[86] = volatile_buffer_cpu[86];
    buffer_lea[87] = volatile_buffer_cpu[87];
    buffer_lea[88] = volatile_buffer_cpu[88];
    buffer_lea[89] = volatile_buffer_cpu[89];
    pos++;
  }
}



void loopcpy95() {
  s_start = 0;
  s_end = 0;
  pos = 0;
  for (uint16_t i = 0; i < REPEAT; ++i) {
    buffer_lea[0] = volatile_buffer_cpu[0];
    buffer_lea[1] = volatile_buffer_cpu[1];
    buffer_lea[2] = volatile_buffer_cpu[2];
    buffer_lea[3] = volatile_buffer_cpu[3];
    buffer_lea[4] = volatile_buffer_cpu[4];
    buffer_lea[5] = volatile_buffer_cpu[5];
    buffer_lea[6] = volatile_buffer_cpu[6];
    buffer_lea[7] = volatile_buffer_cpu[7];
    buffer_lea[8] = volatile_buffer_cpu[8];
    buffer_lea[9] = volatile_buffer_cpu[9];
    buffer_lea[10] = volatile_buffer_cpu[10];
    buffer_lea[11] = volatile_buffer_cpu[11];
    buffer_lea[12] = volatile_buffer_cpu[12];
    buffer_lea[13] = volatile_buffer_cpu[13];
    buffer_lea[14] = volatile_buffer_cpu[14];
    buffer_lea[15] = volatile_buffer_cpu[15];
    buffer_lea[16] = volatile_buffer_cpu[16];
    buffer_lea[17] = volatile_buffer_cpu[17];
    buffer_lea[18] = volatile_buffer_cpu[18];
    buffer_lea[19] = volatile_buffer_cpu[19];
    buffer_lea[20] = volatile_buffer_cpu[20];
    buffer_lea[21] = volatile_buffer_cpu[21];
    buffer_lea[22] = volatile_buffer_cpu[22];
    buffer_lea[23] = volatile_buffer_cpu[23];
    buffer_lea[24] = volatile_buffer_cpu[24];
    buffer_lea[25] = volatile_buffer_cpu[25];
    buffer_lea[26] = volatile_buffer_cpu[26];
    buffer_lea[27] = volatile_buffer_cpu[27];
    buffer_lea[28] = volatile_buffer_cpu[28];
    buffer_lea[29] = volatile_buffer_cpu[29];
    buffer_lea[30] = volatile_buffer_cpu[30];
    buffer_lea[31] = volatile_buffer_cpu[31];
    buffer_lea[32] = volatile_buffer_cpu[32];
    buffer_lea[33] = volatile_buffer_cpu[33];
    buffer_lea[34] = volatile_buffer_cpu[34];
    buffer_lea[35] = volatile_buffer_cpu[35];
    buffer_lea[36] = volatile_buffer_cpu[36];
    buffer_lea[37] = volatile_buffer_cpu[37];
    buffer_lea[38] = volatile_buffer_cpu[38];
    buffer_lea[39] = volatile_buffer_cpu[39];
    buffer_lea[40] = volatile_buffer_cpu[40];
    buffer_lea[41] = volatile_buffer_cpu[41];
    buffer_lea[42] = volatile_buffer_cpu[42];
    buffer_lea[43] = volatile_buffer_cpu[43];
    buffer_lea[44] = volatile_buffer_cpu[44];
    buffer_lea[45] = volatile_buffer_cpu[45];
    buffer_lea[46] = volatile_buffer_cpu[46];
    buffer_lea[47] = volatile_buffer_cpu[47];
    buffer_lea[48] = volatile_buffer_cpu[48];
    buffer_lea[49] = volatile_buffer_cpu[49];
    buffer_lea[50] = volatile_buffer_cpu[50];
    buffer_lea[51] = volatile_buffer_cpu[51];
    buffer_lea[52] = volatile_buffer_cpu[52];
    buffer_lea[53] = volatile_buffer_cpu[53];
    buffer_lea[54] = volatile_buffer_cpu[54];
    buffer_lea[55] = volatile_buffer_cpu[55];
    buffer_lea[56] = volatile_buffer_cpu[56];
    buffer_lea[57] = volatile_buffer_cpu[57];
    buffer_lea[58] = volatile_buffer_cpu[58];
    buffer_lea[59] = volatile_buffer_cpu[59];
    buffer_lea[60] = volatile_buffer_cpu[60];
    buffer_lea[61] = volatile_buffer_cpu[61];
    buffer_lea[62] = volatile_buffer_cpu[62];
    buffer_lea[63] = volatile_buffer_cpu[63];
    buffer_lea[64] = volatile_buffer_cpu[64];
    buffer_lea[65] = volatile_buffer_cpu[65];
    buffer_lea[66] = volatile_buffer_cpu[66];
    buffer_lea[67] = volatile_buffer_cpu[67];
    buffer_lea[68] = volatile_buffer_cpu[68];
    buffer_lea[69] = volatile_buffer_cpu[69];
    buffer_lea[70] = volatile_buffer_cpu[70];
    buffer_lea[71] = volatile_buffer_cpu[71];
    buffer_lea[72] = volatile_buffer_cpu[72];
    buffer_lea[73] = volatile_buffer_cpu[73];
    buffer_lea[74] = volatile_buffer_cpu[74];
    buffer_lea[75] = volatile_buffer_cpu[75];
    buffer_lea[76] = volatile_buffer_cpu[76];
    buffer_lea[77] = volatile_buffer_cpu[77];
    buffer_lea[78] = volatile_buffer_cpu[78];
    buffer_lea[79] = volatile_buffer_cpu[79];
    buffer_lea[80] = volatile_buffer_cpu[80];
    buffer_lea[81] = volatile_buffer_cpu[81];
    buffer_lea[82] = volatile_buffer_cpu[82];
    buffer_lea[83] = volatile_buffer_cpu[83];
    buffer_lea[84] = volatile_buffer_cpu[84];
    buffer_lea[85] = volatile_buffer_cpu[85];
    buffer_lea[86] = volatile_buffer_cpu[86];
    buffer_lea[87] = volatile_buffer_cpu[87];
    buffer_lea[88] = volatile_buffer_cpu[88];
    buffer_lea[89] = volatile_buffer_cpu[89];
    buffer_lea[90] = volatile_buffer_cpu[90];
    buffer_lea[91] = volatile_buffer_cpu[91];
    buffer_lea[92] = volatile_buffer_cpu[92];
    buffer_lea[93] = volatile_buffer_cpu[93];
    buffer_lea[94] = volatile_buffer_cpu[94];
    pos++;
  }
}

void loopcpy(uint16_t size) {
  for (uint16_t i = 0; i < REPEAT; ++i) {
    s_start = 0;
    s_end = 0;
    pos = 0;
    for (uint16_t j = 0; j < size; ++j) {
      buffer_lea[j] = volatile_buffer_cpu[j];
      pos++;
    }
  }
}

void dmacpy(uint16_t size) {
  uint32_t src = (uint32_t)buffer_cpu;
  uint32_t dst = (uint32_t)buffer_lea;

  for (uint16_t i = 0; i < REPEAT; ++i) {
    DMA_makeTransfer(src, dst, size);
  }
}

void dmacpy_opt(uint16_t size) {
  uint32_t src = (uint32_t)buffer_cpu;
  uint32_t dst = (uint32_t)buffer_lea;

  for (uint16_t i = 0; i < REPEAT; ++i) {
    DMA_makeTransfer_opt(src, dst, size);
  }
}

void clibcpy(uint16_t size) {
  for (uint16_t i = 0; i < REPEAT; ++i) {
    memcpy(buffer_lea, buffer_cpu, size);
  }
}

int main() {
  init();

  msp_send_printf("Test Ways to copy memory\n");

  uint32_t time = 0;

  msp_send_printf("============================= Loop Copy (Unrolled) =======================================\n");
  start_timer();
  loopcpy1();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Copy (Unrolled); size: 1, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy5();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Copy (Unrolled); size: 5, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy10();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Copy (Unrolled); size: 10, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy15();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Copy (Unrolled); size: 15, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy20();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Copy (Unrolled); size: 20, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy25();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Copy (Unrolled); size: 25, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy30();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Copy (Unrolled); size: 30, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy35();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Copy (Unrolled); size: 35, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy40();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Copy (Unrolled); size: 40, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy45();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Copy (Unrolled); size: 45, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy50();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Copy (Unrolled); size: 50, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy55();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Copy (Unrolled); size: 55, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy60();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Copy (Unrolled); size: 60, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy65();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Copy (Unrolled); size: 65, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy70();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Copy (Unrolled); size: 70, time: %n\n", REPEAT, time);

    start_timer();
  loopcpy75();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Copy (Unrolled); size: 75, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy80();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Copy (Unrolled); size: 80, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy85();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Copy (Unrolled); size: 85, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy90();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Copy (Unrolled); size: 90, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy95();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Copy (Unrolled); size: 95, time: %n\n", REPEAT, time);

  uint16_t sizes[] = {1, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95};
  uint16_t l = 20;

  msp_send_printf("============================= Loop Copy =======================================\n");
  for (uint16_t i = 0; i < l; ++i) {
    start_timer();
    loopcpy(sizes[i]);
    time = stop_timer();
    msp_send_printf("(REPEAT=%u) copy memory with Loop Copy; size: %u, time: %n\n", REPEAT, sizes[i], time);
  }

  msp_send_printf("============================= memcpy() =======================================\n");
  for (uint16_t i = 0; i < l; ++i) {
    start_timer();
    clibcpy(sizes[i]);
    time = stop_timer();
    msp_send_printf("(REPEAT=%u) copy memory with memcpy(); size: %u, time: %n\n", REPEAT, sizes[i], time);
  }

  msp_send_printf("============================= DMA =======================================\n");
  for (uint16_t i = 0; i < l; ++i) {
    start_timer();
    dmacpy(sizes[i]);
    time = stop_timer();
    msp_send_printf("(REPEAT=%u) copy memory with DMA; size: %u, time: %n\n", REPEAT, sizes[i], time);
  }

  msp_send_printf("============================= DMA (Optimized) =======================================\n");
  for (uint16_t i = 0; i < l; ++i) {
    start_timer();
    dmacpy_opt(sizes[i]);
    time = stop_timer();
    msp_send_printf("(REPEAT=%u) copy memory with DMA (Optimized); size: %u, time: %n\n", REPEAT, sizes[i], time);
  }

  exit();
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