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
volatile __ro_hinv _q15 buffer_fram[SIZE];

volatile __ro_hinv mat_t buffer_meta = {
	.dims = {1, 1, 32, 32},
	.len_dims = 4,
	.strides = {1024, 1024, 32, 1},
	.data = (fixed *)buffer_fram
};

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

#define DMA_makeTransfer_opt(src, dst, size) {\
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

#define DMA_makeTransfer(src, dst, size) {\
  __lupe_data16_write_addr((DMA0SA), (src)); \
  __lupe_data16_write_addr((DMA0DA), (dst)); \
  DMA0SZ = size; \
  DMA0CTL |= DMA_EN_SIG; \
}

#define DMA_setWord(dst, word, size) {\
  __lupe_data16_write_addr((DMA1SA), ((uint32_t)(&word))); \
  __lupe_data16_write_addr((DMA1DA), (dst)); \
  DMA1SZ = size; \
  DMA1CTL |= DMA_EN_SIG; \
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
volatile uint16_t output_len = 1;

void fram_matcpy1() {
  for (uint16_t i = 0; i < REPEAT; ++i) {
    for (uint16_t j = 0; j < output_len; ++j) {
      buffer_meta.data[0] = buffer_meta.data[SIZE - 1];
      ++pos;
    }
  }
}

void fram_matcpy5() {
  for (uint16_t i = 0; i < REPEAT; ++i) {
    #pragma GCC unroll 1
    for (uint16_t j = 0; j < output_len; ++j) {
      buffer_meta.data[0] = buffer_meta.data[SIZE - 1];
      ++pos;
    }
  }
}

void fram_matcpy10() {
  for (uint16_t i = 0; i < REPEAT; ++i) {
    buffer_meta.data[0] = buffer_meta.data[SIZE - 0 - 1];
    buffer_meta.data[1] = buffer_meta.data[SIZE - 1 - 1];
    buffer_meta.data[2] = buffer_meta.data[SIZE - 2 - 1];
    buffer_meta.data[3] = buffer_meta.data[SIZE - 3 - 1];
    buffer_meta.data[4] = buffer_meta.data[SIZE - 4 - 1];
    buffer_meta.data[5] = buffer_meta.data[SIZE - 5 - 1];
    buffer_meta.data[6] = buffer_meta.data[SIZE - 6 - 1];
    buffer_meta.data[7] = buffer_meta.data[SIZE - 7 - 1];
    buffer_meta.data[8] = buffer_meta.data[SIZE - 8 - 1];
    buffer_meta.data[9] = buffer_meta.data[SIZE - 9 - 1];
  }
}

void framcpy1() {
  for (uint16_t i = 0; i < REPEAT; ++i) {
    buffer_fram[0] = buffer_fram[SIZE - 1];
  }
}

void framcpy5() {
  for (uint16_t i = 0; i < REPEAT; ++i) {
    buffer_fram[0] = buffer_fram[SIZE - 0 - 1];
    buffer_fram[1] = buffer_fram[SIZE - 1 - 1];
    buffer_fram[2] = buffer_fram[SIZE - 2 - 1];
    buffer_fram[3] = buffer_fram[SIZE - 3 - 1];
    buffer_fram[4] = buffer_fram[SIZE - 4 - 1];
  }
}

void framcpy10() {
  for (uint16_t i = 0; i < REPEAT; ++i) {
    buffer_fram[0] = buffer_fram[SIZE - 0 - 1];
    buffer_fram[1] = buffer_fram[SIZE - 1 - 1];
    buffer_fram[2] = buffer_fram[SIZE - 2 - 1];
    buffer_fram[3] = buffer_fram[SIZE - 3 - 1];
    buffer_fram[4] = buffer_fram[SIZE - 4 - 1];
    buffer_fram[5] = buffer_fram[SIZE - 5 - 1];
    buffer_fram[6] = buffer_fram[SIZE - 6 - 1];
    buffer_fram[7] = buffer_fram[SIZE - 7 - 1];
    buffer_fram[8] = buffer_fram[SIZE - 8 - 1];
    buffer_fram[9] = buffer_fram[SIZE - 9 - 1];
  }
}

void cpy1() {
  for (uint16_t i = 0; i < REPEAT; ++i) {
    buffer_lea[0] = volatile_buffer_cpu[0];
  }
}

void cpy5() {
  for (uint16_t i = 0; i < REPEAT; ++i) {
    buffer_lea[0] = volatile_buffer_cpu[0];
    buffer_lea[1] = volatile_buffer_cpu[1];
    buffer_lea[2] = volatile_buffer_cpu[2];
    buffer_lea[3] = volatile_buffer_cpu[3];
    buffer_lea[4] = volatile_buffer_cpu[4];
  }
}

void cpy10() {
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
  }
}

void loopcpy1() {
  for (uint16_t i = 0; i < REPEAT; ++i) {
    s_start = 0;
    s_end = 0;
    pos = 0;
    #pragma GCC unroll 1
    for (uint16_t j = 0; j < 1; ++j) {
      buffer_lea[j] = volatile_buffer_cpu[j];
      pos++;
    }
  }
}

void loopcpy5() {
  for (uint16_t i = 0; i < REPEAT; ++i) {
    s_start = 0;
    s_end = 0;
    pos = 0;
    #pragma GCC unroll 5
    for (uint16_t j = 0; j < 5; ++j) {
      buffer_lea[j] = volatile_buffer_cpu[j];
      pos++;
    }
  }
}

void loopcpy10() {
  for (uint16_t i = 0; i < REPEAT; ++i) {
    s_start = 0;
    s_end = 0;
    pos = 0;
    #pragma GCC unroll 10
    for (uint16_t j = 0; j < 10; ++j) {
      buffer_lea[j] = volatile_buffer_cpu[j];
      pos++;
    }
  }
}


void dmaset_opt(uint16_t size) {
  uint32_t dst = (uint32_t)buffer_meta.data;

  for (uint16_t i = 0; i < REPEAT; ++i) {
    DMA_setWord(dst, buffer_meta.data[0], size);
  }
}

int main() {
  init();

  msp_send_printf("Test Ways to copy memory\n");

  uint32_t time = 0;

  msp_send_printf("============================= Loop Copy (FRAM MAT Manually Unrolled) =======================================\n");
  start_timer();
  fram_matcpy1();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Copy (FRAM MAT Manually Unrolled); size: 1, time: %n\n", REPEAT, time);

  start_timer();
  fram_matcpy5();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Copy (FRAM MAT Manually Unrolled); size: 5, time: %n\n", REPEAT, time);

  start_timer();
  fram_matcpy10();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Copy (FRAM MAT Manually Unrolled); size: 10, time: %n\n", REPEAT, time);

  msp_send_printf("============================= Loop Copy (FRAM Manually Unrolled) =======================================\n");
  start_timer();
  framcpy1();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Copy (FRAM Manually Unrolled); size: 1, time: %n\n", REPEAT, time);

  start_timer();
  framcpy5();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Copy (FRAM Manually Unrolled); size: 5, time: %n\n", REPEAT, time);

  start_timer();
  framcpy10();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Copy (FRAM Manually Unrolled); size: 10, time: %n\n", REPEAT, time);

  msp_send_printf("============================= Loop Copy (Manually Unrolled) =======================================\n");
  start_timer();
  cpy1();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Copy (Manually Unrolled); size: 1, time: %n\n", REPEAT, time);

  start_timer();
  cpy5();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Copy (Manually Unrolled); size: 5, time: %n\n", REPEAT, time);

  start_timer();
  cpy10();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Copy (Manually Unrolled); size: 10, time: %n\n", REPEAT, time);

  uint16_t ssizes[] = {1, 5, 10};
  uint16_t ll = 3;

  msp_send_printf("============================= DMA (Set Word) =======================================\n");
  for (uint16_t i = 0; i < ll; ++i) {
    start_timer();
    dmaset_opt(ssizes[i]);
    time = stop_timer();
    msp_send_printf("(REPEAT=%u) copy memory with DMA (Set Word); size: %u, time: %n\n", REPEAT, ssizes[i], time);
  }

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