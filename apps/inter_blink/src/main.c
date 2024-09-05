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

#define FREQ 10000
#define LEN 10
#define LIMIT (FREQ * (LEN - 1))

__ro_hinv uint32_t times[LEN];
__ro_hinv uint32_t cnts[LEN];

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

    if (cnt % FREQ == 0) {
      uint16_t i = cnt / FREQ;
      times[i] = get_saved_time();
      cnts[i] = cnt;
    }

    if (cnt == LIMIT) {
      break;
    }

    cnt++;
  }
  stop_intermittent_tests();

  intermittent_stop();

  for (uint16_t i = 0 ; i < LEN; ++i) {
    msp_send_printf("cnt = %n; time = %n", cnts[i], times[i]);
  }
}