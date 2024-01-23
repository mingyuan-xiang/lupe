#include <msp430.h>
#include <libmsp/mspbase.h>
#include <libmsppoweroff/poweroff.h>
#include <libmspsyncioutils/mspsyncioutils.h>
#include <librng/rng.h>

#define LOWER_BOUND 10
#define UPPER_BOUND 20

uint16_t rand(uint16_t low, uint16_t high) {
  return prand_range(high - low) + low;
}

int main() {
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
  intermittent_init();

  uint16_t delay = rand(LOWER_BOUND, UPPER_BOUND);

  start_intermittent_tests_ms(delay);
  while (1) {
    gpio_toggle(1, 0);
    gpio_toggle(1, 1);
  }
  stop_intermittent_tests();

  intermittent_stop();
}