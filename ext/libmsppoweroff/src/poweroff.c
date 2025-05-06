#include <msp430.h>
#include <libmsp/clock.h>
#include <libmsp/gpio.h>
#include <libmsp/timer.h>
#include <libmsp/mspbase.h>
#include <libmspio/uartio.h>
#include <libmspprintf/mspprintf.h>
#include <libmsppoweroff/poweroff.h>

static uint16_t old_csctl2 = 0;

void intermittent_init() {
  lfxt_start(old_csctl2);
}
void intermittent_stop() { lfxt_stop(old_csctl2); }

static uint16_t poweroff_timer_cnt;

static unsigned int intermittent_int_state;
static uint32_t intermittent_tick;

#define CNT_MAX 65536
#define US_PER_CYCLE 15 // (1000000 / CNT_MAX)

static void set_timer_threshold(uint16_t cycles) {
  poweroff_timer_cnt = cycles;
}

void start_intermittent_tests(uint16_t cycles) {
  intermittent_tick = 0;
  set_timer_threshold(cycles);

  // setup the timer on up mode & run
  timer_setup_up(CONFIG_INTERMITTENT_TIMER, SMCLK, 8, 4, cycles);

  intermittent_int_state = __get_interrupt_state();
  __enable_interrupt();

  // start the timer on up mode & run
  timer_start_up(CONFIG_INTERMITTENT_TIMER);
}

void stop_intermittent_tests() {
  // disable the timer
  timer_halt(CONFIG_INTERMITTENT_TIMER);
  __set_interrupt_state(intermittent_int_state);

  timer_halt(CONFIG_INTERMITTENT_TIMER);
  timer_reset(CONFIG_INTERMITTENT_TIMER);
  timer_IFG_disable(CONFIG_INTERMITTENT_TIMER);
}

void __attribute__((interrupt(STIC3(TIMER, CONFIG_INTERMITTENT_TIMER, _A0_VECTOR))))
  STIC3(TIMER, CONFIG_INTERMITTENT_TIMER, _A0_ISR)(void) {
  PMMCTL0 = PMMPW | PMMSWPOR;
}