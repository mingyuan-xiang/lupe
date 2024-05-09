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
    DMA_is_init = 1;
  }
}

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

#define __STICH(A, B) A##B
#define STICH(A, B) __STICH(A, B)
#define __STIC3(A, B, C) A##B##C
#define STIC3(A, B, C) __STIC3(A, B, C)

#define DMA_makeTransfer_opt(CHANNEL, SRCADDR, DSTADDR, SIZE) {                \
    STIC3(DMA, CHANNEL, CTL) = DMADT_1 | DMADSTINCR_3 | DMASRCINCR_3 |         \
      DMADSTBYTE__WORD | DMASRCBYTE__WORD | DMAIE;                             \
    __data16_write_addr((uintptr_t)(&STIC3(DMA, CHANNEL, SA)), SRCADDR);       \
    __data16_write_addr((uintptr_t)(&STIC3(DMA, CHANNEL, DA)), DSTADDR);       \
    STIC3(DMA, CHANNEL, SZ) = SIZE;                                            \
    STIC3(DMA, CHANNEL, CTL) |= DMAEN | DMAREQ;                                \
    __bis_SR_register(GIE + LPM0_bits);                                        \
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

void loopcpy5() {
  for (uint16_t i = 0; i < REPEAT; ++i) {
    #pragma GCC unroll 5
    for (uint16_t j = 0; j < 5; ++j) {
      buffer_lea[j] = volatile_buffer_cpu[j];
    }
  }
}

void loopcpy10() {
  for (uint16_t i = 0; i < REPEAT; ++i) {
    #pragma GCC unroll 10
    for (uint16_t j = 0; j < 10; ++j) {
      buffer_lea[j] = volatile_buffer_cpu[j];
    }
  }
}

void loopcpy15() {
  for (uint16_t i = 0; i < REPEAT; ++i) {
    #pragma GCC unroll 15
    for (uint16_t j = 0; j < 15; ++j) {
      buffer_lea[j] = volatile_buffer_cpu[j];
    }
  }
}

void loopcpy20() {
  for (uint16_t i = 0; i < REPEAT; ++i) {
    #pragma GCC unroll 20
    for (uint16_t j = 0; j < 20; ++j) {
      buffer_lea[j] = volatile_buffer_cpu[j];
    }
  }
}

void loopcpy25() {
  for (uint16_t i = 0; i < REPEAT; ++i) {
    #pragma GCC unroll 25
    for (uint16_t j = 0; j < 25; ++j) {
      buffer_lea[j] = volatile_buffer_cpu[j];
    }
  }
}

void loopcpy30() {
  for (uint16_t i = 0; i < REPEAT; ++i) {
    #pragma GCC unroll 30
    for (uint16_t j = 0; j < 30; ++j) {
      buffer_lea[j] = volatile_buffer_cpu[j];
    }
  }
}

void loopcpy35() {
  for (uint16_t i = 0; i < REPEAT; ++i) {
    #pragma GCC unroll 35
    for (uint16_t j = 0; j < 35; ++j) {
      buffer_lea[j] = volatile_buffer_cpu[j];
    }
  }
}

void loopcpy40() {
  for (uint16_t i = 0; i < REPEAT; ++i) {
    #pragma GCC unroll 40
    for (uint16_t j = 0; j < 40; ++j) {
      buffer_lea[j] = volatile_buffer_cpu[j];
    }
  }
}

void loopcpy45() {
  for (uint16_t i = 0; i < REPEAT; ++i) {
    #pragma GCC unroll 45
    for (uint16_t j = 0; j < 45; ++j) {
      buffer_lea[j] = volatile_buffer_cpu[j];
    }
  }
}

void loopcpy50() {
  for (uint16_t i = 0; i < REPEAT; ++i) {
    #pragma GCC unroll 50
    for (uint16_t j = 0; j < 50; ++j) {
      buffer_lea[j] = volatile_buffer_cpu[j];
    }
  }
}

void loopcpy55() {
  for (uint16_t i = 0; i < REPEAT; ++i) {
    #pragma GCC unroll 55
    for (uint16_t j = 0; j < 55; ++j) {
      buffer_lea[j] = volatile_buffer_cpu[j];
    }
  }
}

void loopcpy60() {
  for (uint16_t i = 0; i < REPEAT; ++i) {
    #pragma GCC unroll 60
    for (uint16_t j = 0; j < 60; ++j) {
      buffer_lea[j] = volatile_buffer_cpu[j];
    }
  }
}

void loopcpy65() {
  for (uint16_t i = 0; i < REPEAT; ++i) {
    #pragma GCC unroll 65
    for (uint16_t j = 0; j < 65; ++j) {
      buffer_lea[j] = volatile_buffer_cpu[j];
    }
  }
}

void loopcpy70() {
  for (uint16_t i = 0; i < REPEAT; ++i) {
    #pragma GCC unroll 70
    for (uint16_t j = 0; j < 70; ++j) {
      buffer_lea[j] = volatile_buffer_cpu[j];
    }
  }
}

void loopcpy75() {
  for (uint16_t i = 0; i < REPEAT; ++i) {
    #pragma GCC unroll 75
    for (uint16_t j = 0; j < 75; ++j) {
      buffer_lea[j] = volatile_buffer_cpu[j];
    }
  }
}

void loopcpy80() {
  for (uint16_t i = 0; i < REPEAT; ++i) {
    #pragma GCC unroll 80
    for (uint16_t j = 0; j < 80; ++j) {
      buffer_lea[j] = volatile_buffer_cpu[j];
    }
  }
}

void loopcpy85() {
  for (uint16_t i = 0; i < REPEAT; ++i) {
    #pragma GCC unroll 85
    for (uint16_t j = 0; j < 85; ++j) {
      buffer_lea[j] = volatile_buffer_cpu[j];
    }
  }
}

void loopcpy90() {
  for (uint16_t i = 0; i < REPEAT; ++i) {
    #pragma GCC unroll 90
    for (uint16_t j = 0; j < 90; ++j) {
      buffer_lea[j] = volatile_buffer_cpu[j];
    }
  }
}

void loopcpy95() {
  for (uint16_t i = 0; i < REPEAT; ++i) {
    #pragma GCC unroll 95
    for (uint16_t j = 0; j < 95; ++j) {
      buffer_lea[j] = volatile_buffer_cpu[j];
    }
  }
}

void loopcpy100() {
  for (uint16_t i = 0; i < REPEAT; ++i) {
    #pragma GCC unroll 100
    for (uint16_t j = 0; j < 100; ++j) {
      buffer_lea[j] = volatile_buffer_cpu[j];
    }
  }
}

void loopcpy(uint16_t size) {
  for (uint16_t i = 0; i < REPEAT; ++i) {
    for (uint16_t j = 0; j < size; ++j) {
      buffer_lea[j] = volatile_buffer_cpu[j];
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
    DMA_makeTransfer_opt(0, src, dst, size);
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

  msp_send_printf("============================= Loop Assign (Unrolled) =======================================\n");
  start_timer();
  loopcpy5();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Assign (Unrolled); size: 5, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy10();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Assign (Unrolled); size: 10, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy15();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Assign (Unrolled); size: 15, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy20();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Assign (Unrolled); size: 20, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy25();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Assign (Unrolled); size: 25, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy30();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Assign (Unrolled); size: 30, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy35();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Assign (Unrolled); size: 35, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy40();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Assign (Unrolled); size: 40, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy45();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Assign (Unrolled); size: 45, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy50();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Assign (Unrolled); size: 50, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy55();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Assign (Unrolled); size: 55, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy60();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Assign (Unrolled); size: 60, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy65();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Assign (Unrolled); size: 65, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy70();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Assign (Unrolled); size: 70, time: %n\n", REPEAT, time);

    start_timer();
  loopcpy75();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Assign (Unrolled); size: 75, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy80();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Assign (Unrolled); size: 80, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy85();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Assign (Unrolled); size: 85, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy90();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Assign (Unrolled); size: 90, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy95();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Assign (Unrolled); size: 95, time: %n\n", REPEAT, time);

  start_timer();
  loopcpy100();
  time = stop_timer();
  msp_send_printf("(REPEAT=%u) copy memory with Loop Assign (Unrolled); size: 100, time: %n\n", REPEAT, time);

  uint16_t sizes[] = {5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100};
  uint16_t l = 20;

  msp_send_printf("============================= Loop Assign =======================================\n");
  for (uint16_t i = 0; i < l; ++i) {
    start_timer();
    loopcpy(sizes[i]);
    time = stop_timer();
    msp_send_printf("(REPEAT=%u) copy memory with Loop Assign; size: %u, time: %n\n", REPEAT, sizes[i], time);
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