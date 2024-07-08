#include <libmsp/mspbase.h>
#include <libmspdriver/driverlib.h>
#include <libmsptimer/timekeeper.h>
#include <libmspsyncioutils/mspsyncioutils.h>
#include <msp430.h>

#include <include/utils.h>
#include <include/data.h>
#include <include/fir_conv.h>

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

  uint16_t l;
  msp_send_printf("Test ResNet3\n");

  uint32_t total_time = 0;

  for (uint16_t i = 0; i < 10; ++i) {
    start_timer();
    fir_conv(&input_meta, &output_meta, &weight_meta, &bias_meta);
    uint32_t t = stop_timer();
    total_time += t;
  }

  msp_send_printf("total cycles (32767 Hz): %n", total_time);

  exit();
}
