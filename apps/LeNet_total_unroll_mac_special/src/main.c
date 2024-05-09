#include <libmsp/mspbase.h>
#include <libmspdriver/driverlib.h>
#include <libmsptimer/timekeeper.h>
#include <libmspsyncioutils/mspsyncioutils.h>
#include <msp430.h>

#include <include/LeNet.h>
#include <buffer/include/input.h>
#include <layers/include/utils.h>

#define TIME_SHIFTER 4

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
  msp_send_printf("Test LeNet\n");
  l = LeNet(&input_meta);
  msp_send_printf("Got %u, Expecting: 7", l);

  exit();
}