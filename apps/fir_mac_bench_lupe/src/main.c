#include <libmsp/mspbase.h>
#include <libmspdriver/driverlib.h>
#include <libmsptimer/timekeeper.h>
#include <libmspsyncioutils/mspsyncioutils.h>
#include <msp430.h>

#include <include/utils.h>
#include <include/data.h>
#include <include/fir_conv.h>
#include <include/mac_conv.h>
#include <include/fir_conv_time.h>
#include <include/mac_conv_time.h>

#define FIR_CONV(N) fir_conv(&input_##N##_meta, &output_##N##_meta, &weight_##N##_meta, &bias_##N##_meta);
#define MAC_CONV(N) mac_conv(&input_##N##_meta, &output_##N##_meta, &weight_##N##_meta, &bias_##N##_meta);
#define CONV_BENCH(FUNC, N, FREQ) { \
  total_time = 0; \
  for (uint16_t i = 0; i < FREQ; ++i) { \
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

#define FIR_CONV_TIME(N, REPEAT) fir_conv_time(&input_##N##_meta, &output_##N##_meta, &weight_##N##_meta, &bias_##N##_meta, REPEAT, &t);
#define MAC_CONV_TIME(N, REPEAT) mac_conv_time(&input_##N##_meta, &output_##N##_meta, &weight_##N##_meta, &bias_##N##_meta, REPEAT, &t);
#define CONV_BENCH_TIME(FUNC, N, FREQ) { \
  total_time = 0; \
  uint32_t t = 0; \
  FUNC(N, FREQ) \
  total_time = t; \
  uint16_t flt_size = weight_##N##_meta.dims[2]; \
  uint16_t in_ch = input_##N##_meta.dims[1]; \
  uint16_t out_ch = output_##N##_meta.dims[1]; \
  uint16_t in_size = input_##N##_meta.dims[2]; \
  msp_send_printf("total cycles for compute (32767 Hz), repeat for %u times: %n (kernel size: %u, in_channels: %u, out_channels: %u, input size: %u)", FREQ, total_time, flt_size, in_ch, out_ch, in_size); \
}

#define REP 100

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
  
  msp_send_printf("=========================== benchmark fir ============================");

  CONV_BENCH(FIR_CONV, 0, REP);
  CONV_BENCH(FIR_CONV, 1, REP);

  msp_send_printf("=========================== benchmark fir (compute) ============================");

  CONV_BENCH_TIME(FIR_CONV_TIME, 0, REP);
  CONV_BENCH_TIME(FIR_CONV_TIME, 1, REP);

  msp_send_printf("=========================== benchmark mac ============================");

  CONV_BENCH(MAC_CONV, 0, REP);
  CONV_BENCH(MAC_CONV, 1, REP);

  msp_send_printf("=========================== benchmark mac (compute) ============================");

  CONV_BENCH_TIME(MAC_CONV_TIME, 0, REP);
  CONV_BENCH_TIME(MAC_CONV_TIME, 1, REP);

  exit();
}
