#include <libmsp/mspbase.h>
#include <libmspdriver/driverlib.h>
#include <libmspsyncioutils/mspsyncioutils.h>
#include <msp430.h>

#include <include/input.h>
#include <include/output.h>
#include <include/output_exp.h>
#include <include/weight.h>
#include <include/intermittent.h>
#include <include/utils.h>
#include <include/conv.h>
#include <libmsppoweroff/poweroff.h>
#include <librng/rng.h>

/* ACLK cycles (32768 Hz) */
#define DELAY 100

#define REPEAT 1 

void init() {
  watchdog_disable();
  gpio_init_all_ports();
  gpio_unlock();
  gpio_clear_interrupts();
  clock_init();
  uartio_open(0);
  init_lupe();
}

void exit() {
  msp_end_printing();
}

uint16_t verify() {
  uint16_t flag = 0;
  for (uint16_t i = 0; i < output_meta.dims[1]; ++i) {
    if (output_meta.data[i] != output_exp_meta.data[i]) {
      flag = 1;
      break;
    }
  }
  return flag;
}

int main() {
  init();

  uint16_t c = intermittent_status[COUNTER] + 1;
  VOLATILE_WRITE(c, COUNTER);

  for (uint16_t i = intermittent_status[MAIN_LOOP]; i < REPEAT; ++i) {
    start_intermittent_tests(0, DELAY);
    conv(&input_meta, &output_meta, &weight_meta);
    stop_intermittent_tests();

    conv(&input_meta, &output_exp_meta, &weight_meta);
    
    if (verify() != 0) {
      break;
    }

    uint16_t next_i = i + 1;
    VOLATILE_WRITE(next_i, MAIN_LOOP);
  }

  msp_send_printf("Restart times: %u (repeat: %u)", intermittent_status[COUNTER], REPEAT);
  
  msp_send_printf("Got activations for the last layer:");
  msp_send_mat(&output_meta);
  msp_send_printf("Expected:");
  msp_send_mat(&output_exp_meta);

  exit();
}