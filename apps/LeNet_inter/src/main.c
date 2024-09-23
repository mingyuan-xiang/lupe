#include <libmsp/mspbase.h>
#include <libmspdriver/driverlib.h>
#include <libmsptimer/timekeeper.h>
#include <libmspsyncioutils/mspsyncioutils.h>
#include <msp430.h>

#include <include/LeNet_inter.h>
#include <buffer/include/input.h>
#include <buffer/include/input1.h>
#include <buffer/include/output.h>
#include <buffer/include/output1.h>
#include <buffer/include/buffer.h>
#include <layers/include/utils.h>
#include <layers/include/intermittent.h>
#include <libmsppoweroff/poweroff.h>
#include <librng/rng.h>

#include <params/include/conv1_Conv.h>
#include <params/include/conv2_Conv.h>
#include <params/include/conv3_Conv.h>
#include <params/include/fc1_Gemm.h>
#include <params/include/fc2_Gemm.h>

#include <layers/include/conv2_Conv.h>

/* ACLK cycles (32768 Hz) */
#define LOWER_BOUND 1
#define UPPER_BOUND 10

#define REPEAT 100

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
    if (output_meta.data[i] != output1_meta.data[i]) {
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

    start_intermittent_tests(0, delay);
    conv2_Conv(&input_meta, &input1_meta, &output1_meta);
    stop_intermittent_tests();

    intermittent_status[COMPUTE_CK] = INTERMITTENT_LeNet_START;

    intermittent_status[COUNTER] += verify();
    if (intermittent_status[COUNTER] != 0) {
      break;
    }

    intermittent_status[MAIN_LOOP]++;
  }

  msp_send_printf("Incorrect results: %u (repeat: %u)", intermittent_status[COUNTER], REPEAT);

  if (intermittent_status[COUNTER] != 0) {
    // msp_send_mat(&fc2_Gemm_out_meta);
    msp_send_mat(&output1_meta);
    msp_send_mat(&output_meta);
    msp_send_printf("MAIN_LOOP: %u", intermittent_status[MAIN_LOOP]);
  }

  exit();
}