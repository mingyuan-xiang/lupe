#include <stdint.h>
#include <libmsp/mspbase.h>
#include <libmspdriver/driverlib.h>
#include <libmsptimer/timekeeper.h>
#include <libmspsyncioutils/mspsyncioutils.h>

#include <libfixedAbstract/fixed.h>
#include <libmspdriver/driverlib.h>
#include <libmatAbstract/mat.h>
#include <stdint.h>
#include <libdsp/DSPLib.h>

#define TIME 328

void init() {
  watchdog_disable();
  gpio_init_all_ports();
  gpio_unlock();
  gpio_clear_interrupts();
  clock_init();
  uartio_open(0);
  timers_init();
}

void exit() {
  timers_stop();
}

int main() {
  init();

  start_timer();

  uint32_t cnt = 0;

  while(1) {
    uint32_t time = get_time();


  }

  exit();
}