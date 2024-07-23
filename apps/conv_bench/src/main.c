#include <libmsp/mspbase.h>
#include <libmspdriver/driverlib.h>
#include <libmsptimer/timekeeper.h>
#include <libmspsyncioutils/mspsyncioutils.h>
#include <msp430.h>

#include <include/utils.h>
#include <include/data.h>
#include <include/fir_conv.h>
#include <include/mac_conv.h>

#define FIR_CONV(N) fir_conv(&input_##N##_meta, &output_##N##_meta, &weight_##N##_meta, &bias_##N##_meta);
#define MAC_CONV(N) mac_conv(&input_##N##_meta, &output_##N##_meta, &weight_##N##_meta, &bias_##N##_meta);
#define CONV_BENCH(FUNC, N, FREQ) { \
  total_time = 0; \
  for (uint16_t i = 0; i < 10; ++i) { \
    start_timer(); \
    FUNC(N) \
    uint32_t t = stop_timer(); \
    total_time += t; \
  } \
  uint16_t flt_size = weight_##N##_meta.dims[2]; \
  uint16_t in_ch = input_##N##_meta.dims[1]; \
  uint16_t out_ch = output_##N##_meta.dims[1]; \
  uint16_t in_size = input_##N##_meta.dims[2]; \
  msp_send_printf("total cycles (32767 Hz), repeat for %u times: %n (kernel size: %u, in_channels: %u, out_channels: %u, input size: %u)", FREQ, total_time, flt_size, in_ch, out_ch, in_size); \
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
  
  msp_send_printf("============================ benchmark fir ============================");

  CONV_BENCH(FIR_CONV, 0, 10);
  CONV_BENCH(FIR_CONV, 1, 10);
  CONV_BENCH(FIR_CONV, 2, 10);
  CONV_BENCH(FIR_CONV, 3, 10);
  CONV_BENCH(FIR_CONV, 4, 10);
  CONV_BENCH(FIR_CONV, 5, 10);
  CONV_BENCH(FIR_CONV, 6, 10);
  CONV_BENCH(FIR_CONV, 7, 10);
  CONV_BENCH(FIR_CONV, 8, 10);
  CONV_BENCH(FIR_CONV, 9, 10);
  CONV_BENCH(FIR_CONV, 10, 10);
  // CONV_BENCH(FIR_CONV, 11, 10);
  // CONV_BENCH(FIR_CONV, 12, 10);
  // CONV_BENCH(FIR_CONV, 13, 10);
  // CONV_BENCH(FIR_CONV, 14, 10);
  // CONV_BENCH(FIR_CONV, 15, 10);
  // CONV_BENCH(FIR_CONV, 16, 10);
  // CONV_BENCH(FIR_CONV, 17, 10);
  // CONV_BENCH(FIR_CONV, 18, 10);
  // CONV_BENCH(FIR_CONV, 19, 10);
  // CONV_BENCH(FIR_CONV, 20, 10);
  // CONV_BENCH(FIR_CONV, 21, 10);
  // CONV_BENCH(FIR_CONV, 22, 10);
  // CONV_BENCH(FIR_CONV, 23, 10);
  // CONV_BENCH(FIR_CONV, 24, 10);
  // CONV_BENCH(FIR_CONV, 25, 10);
  // CONV_BENCH(FIR_CONV, 26, 10);
  // CONV_BENCH(FIR_CONV, 27, 10);
  // CONV_BENCH(FIR_CONV, 28, 10);
  // CONV_BENCH(FIR_CONV, 29, 10);
  // CONV_BENCH(FIR_CONV, 30, 10);
  // CONV_BENCH(FIR_CONV, 31, 10);
  // CONV_BENCH(FIR_CONV, 32, 10);
  // CONV_BENCH(FIR_CONV, 33, 10);
  // CONV_BENCH(FIR_CONV, 34, 10);
  // CONV_BENCH(FIR_CONV, 35, 10);
  // CONV_BENCH(FIR_CONV, 36, 10);
  // CONV_BENCH(FIR_CONV, 37, 10);
  // CONV_BENCH(FIR_CONV, 38, 10);
  // CONV_BENCH(FIR_CONV, 39, 10);
  // CONV_BENCH(FIR_CONV, 40, 10);
  // CONV_BENCH(FIR_CONV, 41, 10);
  // CONV_BENCH(FIR_CONV, 42, 10);
  // CONV_BENCH(FIR_CONV, 43, 10);
  // CONV_BENCH(FIR_CONV, 44, 10);
  // CONV_BENCH(FIR_CONV, 45, 10);
  // CONV_BENCH(FIR_CONV, 46, 10);
  // CONV_BENCH(FIR_CONV, 47, 10);
  // CONV_BENCH(FIR_CONV, 48, 10);
  // CONV_BENCH(FIR_CONV, 49, 10);
  // CONV_BENCH(FIR_CONV, 50, 10);
  // CONV_BENCH(FIR_CONV, 51, 10);
  // CONV_BENCH(FIR_CONV, 52, 10);
  // CONV_BENCH(FIR_CONV, 53, 10);
  // CONV_BENCH(FIR_CONV, 54, 10);
  // CONV_BENCH(FIR_CONV, 55, 10);
  // CONV_BENCH(FIR_CONV, 56, 10);
  // CONV_BENCH(FIR_CONV, 57, 10);
  // CONV_BENCH(FIR_CONV, 58, 10);
  // CONV_BENCH(FIR_CONV, 59, 10);
  // CONV_BENCH(FIR_CONV, 60, 10);
  // CONV_BENCH(FIR_CONV, 61, 10);
  // CONV_BENCH(FIR_CONV, 62, 10);
  // CONV_BENCH(FIR_CONV, 63, 10);
  // CONV_BENCH(FIR_CONV, 64, 10);
  // CONV_BENCH(FIR_CONV, 65, 10);
  // CONV_BENCH(FIR_CONV, 66, 10);
  // CONV_BENCH(FIR_CONV, 67, 10);
  // CONV_BENCH(FIR_CONV, 68, 10);
  // CONV_BENCH(FIR_CONV, 69, 10);
  // CONV_BENCH(FIR_CONV, 70, 10);
  // CONV_BENCH(FIR_CONV, 71, 10);
  // CONV_BENCH(FIR_CONV, 72, 10);
  // CONV_BENCH(FIR_CONV, 73, 10);
  // CONV_BENCH(FIR_CONV, 74, 10);
  // CONV_BENCH(FIR_CONV, 75, 10);
  // CONV_BENCH(FIR_CONV, 76, 10);

  msp_send_printf("============================ benchmark mac ============================");

  CONV_BENCH(MAC_CONV, 0, 10);
  CONV_BENCH(MAC_CONV, 1, 10);
  CONV_BENCH(MAC_CONV, 2, 10);
  CONV_BENCH(MAC_CONV, 3, 10);
  CONV_BENCH(MAC_CONV, 4, 10);
  CONV_BENCH(MAC_CONV, 5, 10);
  CONV_BENCH(MAC_CONV, 6, 10);
  CONV_BENCH(MAC_CONV, 7, 10);
  CONV_BENCH(MAC_CONV, 8, 10);
  CONV_BENCH(MAC_CONV, 9, 10);
  CONV_BENCH(MAC_CONV, 10, 10);
  // CONV_BENCH(MAC_CONV, 11, 10);
  // CONV_BENCH(MAC_CONV, 12, 10);
  // CONV_BENCH(MAC_CONV, 13, 10);
  // CONV_BENCH(MAC_CONV, 14, 10);
  // CONV_BENCH(MAC_CONV, 15, 10);
  // CONV_BENCH(MAC_CONV, 16, 10);
  // CONV_BENCH(MAC_CONV, 17, 10);
  // CONV_BENCH(MAC_CONV, 18, 10);
  // CONV_BENCH(MAC_CONV, 19, 10);
  // CONV_BENCH(MAC_CONV, 20, 10);
  // CONV_BENCH(MAC_CONV, 21, 10);
  // CONV_BENCH(MAC_CONV, 22, 10);
  // CONV_BENCH(MAC_CONV, 23, 10);
  // CONV_BENCH(MAC_CONV, 24, 10);
  // CONV_BENCH(MAC_CONV, 25, 10);
  // CONV_BENCH(MAC_CONV, 26, 10);
  // CONV_BENCH(MAC_CONV, 27, 10);
  // CONV_BENCH(MAC_CONV, 28, 10);
  // CONV_BENCH(MAC_CONV, 29, 10);
  // CONV_BENCH(MAC_CONV, 30, 10);
  // CONV_BENCH(MAC_CONV, 31, 10);
  // CONV_BENCH(MAC_CONV, 32, 10);
  // CONV_BENCH(MAC_CONV, 33, 10);
  // CONV_BENCH(MAC_CONV, 34, 10);
  // CONV_BENCH(MAC_CONV, 35, 10);
  // CONV_BENCH(MAC_CONV, 36, 10);
  // CONV_BENCH(MAC_CONV, 37, 10);
  // CONV_BENCH(MAC_CONV, 38, 10);
  // CONV_BENCH(MAC_CONV, 39, 10);
  // CONV_BENCH(MAC_CONV, 40, 10);
  // CONV_BENCH(MAC_CONV, 41, 10);
  // CONV_BENCH(MAC_CONV, 42, 10);
  // CONV_BENCH(MAC_CONV, 43, 10);
  // CONV_BENCH(MAC_CONV, 44, 10);
  // CONV_BENCH(MAC_CONV, 45, 10);
  // CONV_BENCH(MAC_CONV, 46, 10);
  // CONV_BENCH(MAC_CONV, 47, 10);
  // CONV_BENCH(MAC_CONV, 48, 10);
  // CONV_BENCH(MAC_CONV, 49, 10);
  // CONV_BENCH(MAC_CONV, 50, 10);
  // CONV_BENCH(MAC_CONV, 51, 10);
  // CONV_BENCH(MAC_CONV, 52, 10);
  // CONV_BENCH(MAC_CONV, 53, 10);
  // CONV_BENCH(MAC_CONV, 54, 10);
  // CONV_BENCH(MAC_CONV, 55, 10);
  // CONV_BENCH(MAC_CONV, 56, 10);
  // CONV_BENCH(MAC_CONV, 57, 10);
  // CONV_BENCH(MAC_CONV, 58, 10);
  // CONV_BENCH(MAC_CONV, 59, 10);
  // CONV_BENCH(MAC_CONV, 60, 10);
  // CONV_BENCH(MAC_CONV, 61, 10);
  // CONV_BENCH(MAC_CONV, 62, 10);
  // CONV_BENCH(MAC_CONV, 63, 10);
  // CONV_BENCH(MAC_CONV, 64, 10);
  // CONV_BENCH(MAC_CONV, 65, 10);
  // CONV_BENCH(MAC_CONV, 66, 10);
  // CONV_BENCH(MAC_CONV, 67, 10);
  // CONV_BENCH(MAC_CONV, 68, 10);
  // CONV_BENCH(MAC_CONV, 69, 10);
  // CONV_BENCH(MAC_CONV, 70, 10);
  // CONV_BENCH(MAC_CONV, 71, 10);
  // CONV_BENCH(MAC_CONV, 72, 10);
  // CONV_BENCH(MAC_CONV, 73, 10);
  // CONV_BENCH(MAC_CONV, 74, 10);
  // CONV_BENCH(MAC_CONV, 75, 10);
  // CONV_BENCH(MAC_CONV, 76, 10);

  exit();
}
