#include <libmsp/mspbase.h>
#include <libmspdriver/driverlib.h>
#include <libmsptimer/timekeeper.h>
#include <libmspsyncioutils/mspsyncioutils.h>
#include <msp430.h>

#include <include/random_data.h>
#include <include/lea_op.h>

#define REP 10000
#define SIZES_LEN 21

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

#define DMA_EN_SIG (DMAEN | DMAREQ)

#define _lupe_random_data16_write_addr(addr, src) ({ \
  uintptr_t __src = src; \
  __asm__  __volatile__ ("movx.a %1, %0\n\t" : "=m"(addr) : "r"(__src)); \
})

#define __lupe_random_data16_write_addr(addr, src) _lupe_random_data16_write_addr((addr), (src))

#define DMA_makeTransfer_opt(src, dst, size) { \
  DMA0CTL = DMADT_1 | DMADSTINCR_3 | DMASRCINCR_3 | DMADSTBYTE__WORD | DMASRCBYTE__WORD | DMAIE; \
  __lupe_random_data16_write_addr((DMA0SA), (src)); \
  __lupe_random_data16_write_addr((DMA0DA), (dst)); \
  DMA0SZ = size; \
  uint16_t interruptState = __get_interrupt_state(); \
  __disable_interrupt(); \
  DMA0CTL |= DMA_EN_SIG; \
  __bis_SR_register(GIE + LPM0_bits); \
  __set_interrupt_state(interruptState); \
}

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

void init_lupe() {
  DMA_disableTransferDuringReadModifyWrite();
  DMA_initialization(0);
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

void exit() {
  msp_end_printing();
  timers_stop();
}

int main() {
  init();

  uint32_t total_time = 0;
  uint16_t sizes[] = {4, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160, 170, 180, 190, 200};

  msp_send_printf("\n=========================== benchmark msp_add_q15 ============================\n");
  msp_add_q15_params add_params = { .length = 10 };
  for (uint16_t i = 0; i < SIZES_LEN; ++i) {
    memcpy(lea_flt, random_data, sizeof(int16_t) * LEA_SIZE);
    memcpy(lea_src, random_data, sizeof(int16_t) * LEA_SIZE);
    memcpy(lea_tmp, random_data, sizeof(int16_t) * LEA_SIZE);
    memcpy(lea_dst, random_data, sizeof(int16_t) * LEA_SIZE);

    msp_send_printf("msp_add_q15 size of %u", sizes[i]);
    add_params.length = sizes[i];
    msp_add_q15_time(&add_params, lea_src, lea_tmp, lea_dst, REP);
    start_timer();
    for (uint16_t r = 0; r < REP; r++) {
      msp_add_q15(&add_params, lea_src, lea_tmp, lea_dst);
    }
    total_time = stop_timer();
    msp_send_printf("total time: %n", total_time);
  }

  msp_send_printf("\n=========================== benchmark msp_mac_q15 ============================\n");
  msp_mac_q15_params mac_params = { .length = 10 };
  for (uint16_t i = 0; i < SIZES_LEN; ++i) {
    memcpy(lea_flt, random_data, sizeof(int16_t) * LEA_SIZE);
    memcpy(lea_src, random_data, sizeof(int16_t) * LEA_SIZE);
    memcpy(lea_tmp, random_data, sizeof(int16_t) * LEA_SIZE);
    memcpy(lea_dst, random_data, sizeof(int16_t) * LEA_SIZE);

    msp_send_printf("msp_mac_q15 size of %u", sizes[i]);
    mac_params.length = sizes[i];
    msp_mac_q15_time(&mac_params, lea_src, lea_tmp, lea_res, REP);
    start_timer();
    for (uint16_t r = 0; r < REP; r++) {
      msp_mac_q15(&mac_params, lea_src, lea_tmp, lea_res);
    }
    total_time = stop_timer();
    msp_send_printf("total time: %n", total_time);
  }

  msp_send_printf("\n=========================== benchmark msp_offset_q15 ============================\n");
  msp_offset_q15_params offset_params = { .length = 10, .offset = 1 };
  for (uint16_t i = 0; i < SIZES_LEN; ++i) {
    memcpy(lea_flt, random_data, sizeof(int16_t) * LEA_SIZE);
    memcpy(lea_src, random_data, sizeof(int16_t) * LEA_SIZE);
    memcpy(lea_tmp, random_data, sizeof(int16_t) * LEA_SIZE);
    memcpy(lea_dst, random_data, sizeof(int16_t) * LEA_SIZE);

    msp_send_printf("msp_offset_q15 size of %u", sizes[i]);
    offset_params.length = sizes[i];
    msp_offset_q15_time(&offset_params, lea_src, lea_dst, REP);
    start_timer();
    for (uint16_t r = 0; r < REP; r++) {
      msp_offset_q15(&offset_params, lea_src, lea_dst);
    }
    total_time = stop_timer();
    msp_send_printf("total time: %n", total_time);
  }

  msp_send_printf("\n=========================== benchmark msp_fir_q15 ============================\n");
  uint16_t k = 3;
  msp_fir_q15_params conv_params = {
    .length = 10 - k + 1,
    .tapLength = k + 1,
    .coeffs = lea_flt,
    .enableCircularBuffer = false 
  };
  for (uint16_t i = 0; i < SIZES_LEN; ++i) {
    memcpy(lea_flt, random_data, sizeof(int16_t) * LEA_SIZE);
    memcpy(lea_src, random_data, sizeof(int16_t) * LEA_SIZE);
    memcpy(lea_tmp, random_data, sizeof(int16_t) * LEA_SIZE);
    memcpy(lea_dst, random_data, sizeof(int16_t) * LEA_SIZE);

    msp_send_printf("msp_fir_q15 size of %u", sizes[i]);
    conv_params.length = sizes[i] - k + 1;
    msp_fir_q15_time(&conv_params, lea_src, lea_dst, REP);
    start_timer();
    for (uint16_t r = 0; r < REP; r++) {
      msp_fir_q15(&conv_params, lea_src, lea_dst);
    }
    total_time = stop_timer();
    msp_send_printf("total time: %n", total_time);
  }

  msp_send_printf("\n=========================== benchmark msp_fill_q15 ============================\n");
  msp_fill_q15_params fill_params = { .length = 10, .value = 1 };
  for (uint16_t i = 0; i < SIZES_LEN; ++i) {
    memcpy(lea_flt, random_data, sizeof(int16_t) * LEA_SIZE);
    memcpy(lea_src, random_data, sizeof(int16_t) * LEA_SIZE);
    memcpy(lea_tmp, random_data, sizeof(int16_t) * LEA_SIZE);
    memcpy(lea_dst, random_data, sizeof(int16_t) * LEA_SIZE);

    msp_send_printf("msp_fill_q15 size of %u", sizes[i]);
    fill_params.length = sizes[i];
    msp_fill_q15_time(&fill_params, lea_dst, REP);
    start_timer();
    for (uint16_t r = 0; r < REP; r++) {
      msp_fill_q15(&fill_params, lea_dst);
    }
    total_time = stop_timer();
    msp_send_printf("total time: %n", total_time);
  }

  msp_send_printf("\n=========================== benchmark msp_mpy_q15 ============================\n");
  msp_mpy_q15_params mpy_params = { .length = 10 };
  for (uint16_t i = 0; i < SIZES_LEN; ++i) {
    memcpy(lea_flt, random_data, sizeof(int16_t) * LEA_SIZE);
    memcpy(lea_src, random_data, sizeof(int16_t) * LEA_SIZE);
    memcpy(lea_tmp, random_data, sizeof(int16_t) * LEA_SIZE);
    memcpy(lea_dst, random_data, sizeof(int16_t) * LEA_SIZE);

    msp_send_printf("msp_mpy_q15 size of %u", sizes[i]);
    mpy_params.length = sizes[i];
    msp_mpy_q15_time(&mpy_params, lea_src, lea_tmp, lea_dst, REP);
    start_timer();
    for (uint16_t r = 0; r < REP; r++) {
      msp_mpy_q15(&mpy_params, lea_src, lea_tmp, lea_dst);
    }
    total_time = stop_timer();
    msp_send_printf("total time: %n", total_time);
  }

  msp_send_printf("\n=========================== benchmark msp_deinterleave_q15 ============================\n");
  msp_deinterleave_q15_params deinterleave_params = {
    .length = 5,
    .channel = 0,
    .numChannels = 2
  };
  for (uint16_t i = 0; i < SIZES_LEN; ++i) {
    memcpy(lea_flt, random_data, sizeof(int16_t) * LEA_SIZE);
    memcpy(lea_src, random_data, sizeof(int16_t) * LEA_SIZE);
    memcpy(lea_tmp, random_data, sizeof(int16_t) * LEA_SIZE);
    memcpy(lea_dst, random_data, sizeof(int16_t) * LEA_SIZE);

    msp_send_printf("msp_deinterleave_q15 size of %u", sizes[i]);
    deinterleave_params.length = sizes[i] / 2;
    msp_deinterleave_q15_time(&deinterleave_params, lea_src, lea_dst, REP);
    start_timer();
    for (uint16_t r = 0; r < REP; r++) {
      msp_deinterleave_q15(&deinterleave_params, lea_src, lea_dst);
    }
    total_time = stop_timer();
    msp_send_printf("total time: %n", total_time);
  }

  msp_send_printf("\n=========================== benchmark msp_matrix_mpy_q15 ============================\n");
  msp_matrix_mpy_q15_params vector_matrix_mpy_params = {
    .srcARows = 1,
    .srcACols = 10,
    .srcBRows = 10,
    .srcBCols = 2
  };
  for (uint16_t i = 0; i < SIZES_LEN; ++i) {
    memcpy(lea_flt, random_data, sizeof(int16_t) * LEA_SIZE);
    memcpy(lea_src, random_data, sizeof(int16_t) * LEA_SIZE);
    memcpy(lea_tmp, random_data, sizeof(int16_t) * LEA_SIZE);
    memcpy(lea_dst, random_data, sizeof(int16_t) * LEA_SIZE);

    msp_send_printf("msp_matrix_mpy_q15 size of %u", sizes[i]);
    vector_matrix_mpy_params.srcACols = sizes[i];
    vector_matrix_mpy_params.srcBRows = sizes[i];
    msp_matrix_mpy_q15_time(&vector_matrix_mpy_params, lea_src, lea_tmp, lea_dst, REP);
    start_timer();
    for (uint16_t r = 0; r < REP; r++) {
      msp_matrix_mpy_q15(&vector_matrix_mpy_params, lea_src, lea_tmp, lea_dst);
    }
    total_time = stop_timer();
    msp_send_printf("total time: %n", total_time);
  }

  msp_send_printf("\n=========================== benchmark DMA ============================\n");
  uintptr_t src = (uintptr_t)lea_src;
  uintptr_t dst = (uintptr_t)lea_dst;

  for (uint16_t i = 0; i < SIZES_LEN; ++i) {
    msp_send_printf("DMA size of %u", sizes[i]);
    start_timer();
    for (uint16_t r = 0; r < REP; r++) {
      DMA_makeTransfer_opt(src, dst, sizes[i]);
    }
    total_time = stop_timer();
    msp_send_printf("DMA time: %n", total_time);

    start_timer();
    for (uint16_t r = 0; r < REP; r++) {
      DMA_makeTransfer(src, dst, sizes[i]);
    }
    total_time = stop_timer();
    msp_send_printf("total time: %n", total_time);
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
