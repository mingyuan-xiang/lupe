#include <libmsp/mspbase.h>
#include <libmspdriver/driverlib.h>
#include <libmsptimer/timekeeper.h>
#include <libmspsyncioutils/mspsyncioutils.h>
#include <msp430.h>

#include <include/LeNet_inter.h>
#include <buffer/include/input.h>
#include <buffer/include/output.h>
#include <buffer/include/buffer.h>
#include <layers/include/utils.h>
#include <layers/include/intermittent.h>
#include <libmsppoweroff/poweroff.h>
#include <librng/rng.h>

#define LOWER_BOUND 100
#define UPPER_BOUND 5000

#define REPEAT 1

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

uint16_t verify() {
  uint16_t flag = 0;
  for (uint16_t i = 0; i < output_meta.dims[1]; ++i) {
    if (output_meta.data[i] != fc2_Gemm_out_meta.data[i]) {
      flag = 1;
    }
  }
  return flag;
}

uint16_t rand(uint16_t low, uint16_t high) {
  return prand_range(high - low) + low;
}

int main() {
  init();

  for (uint16_t i = intermittent_status[MAIN_LOOP]; i < REPEAT; ++i) {
    uint16_t delay = rand(LOWER_BOUND, UPPER_BOUND);

    start_intermittent_tests_us(delay);
    LeNet_inter(&input_meta);
    stop_intermittent_tests();

    // intermittent_status[COMPUTE_CK] = INTERMITTENT_LeNet_START;

    intermittent_status[COUNTER] += verify();
    if (intermittent_status[COUNTER] != 0) {
      break;
    }

    intermittent_status[MAIN_LOOP]++;
  }

  msp_send_printf("Incorrect results: %u (repeat: %u)", intermittent_status[COUNTER], REPEAT);

  if (intermittent_status[COUNTER] != 0) {
    // msp_send_mat(&input_meta);
    // msp_send_mat(&conv1_Conv_in_meta);
    // msp_send_mat(&conv1_Conv_out_meta);
    // msp_send_mat(&fc2_Gemm_out_meta);
    msp_send_printf("%u", intermittent_status[COMPUTE_CK]);
    msp_send_printf("%u", intermittent_status[COMPUTE_IO_ROW]);
    msp_send_printf("%u", intermittent_status[COMPUTE_IN_CH]);
    msp_send_printf("%u", intermittent_status[COMPUTE_OUT_CH]);
  }

  exit();
}