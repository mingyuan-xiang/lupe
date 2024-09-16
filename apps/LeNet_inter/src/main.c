#include <libmsp/mspbase.h>
#include <libmspdriver/driverlib.h>
#include <libmsptimer/timekeeper.h>
#include <libmspsyncioutils/mspsyncioutils.h>
#include <msp430.h>

#include <include/LeNet_inter.h>
#include <buffer/include/input.h>
#include <layers/include/utils.h>
#include <layers/include/intermittent.h>

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

  msp_send_printf("Test LeNet_inter\n");

  uint32_t total_time = 0;
  uint32_t cnt = 0;

  for (uint16_t i = 0; i < 10000; ++i) {
    msp_recv_mat(&input_meta);

    intermittent_status[COMPUTE_CK] = INTERMITTENT_LeNet_START;

    start_timer();
    l = LeNet_inter(&input_meta);

    uint32_t t = stop_timer();
    total_time += t;

    msp_send_printf("%u", l);

    if (cnt == 9 || i == 9999) {
      msp_send_printf("Running LeNet_inter (no_opt) total cycles (32767 Hz) (CNT=%n): %n", cnt + 1, total_time);
      total_time = 0;
      cnt = 0;
    } else {
      ++cnt;
    }
  }
  exit();
}