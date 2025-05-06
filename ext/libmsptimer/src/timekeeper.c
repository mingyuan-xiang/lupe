#include <libmsp/clock.h>
#include <libmsp/gpio.h>
#include <libmsp/timer.h>
#include <libmspprintf/mspprintf.h>
#include <libmsptimer/timekeeper.h>
#include <msp430.h>
#include <string.h>

#include <stdbool.h>
#include <stdint.h>

static uint16_t old_csctl2 = 0;
void timers_init() { lfxt_start(old_csctl2); }
void timers_stop() { lfxt_stop(old_csctl2); }

static uint32_t msp430_timer_tick;
static unsigned int timer_int_state;

void start_timer() {
  msp430_timer_tick = 0;

  /* The original code can choose ACLK and SMCLK, but always using ACLK is
  * stable and reasonable if there exists a crystal.
  */
  timer_setup_cont(CONFIG_STOPWATCH_TIMER, ACLK, 1, 1);

  timer_int_state = __get_interrupt_state();
  __enable_interrupt();

  // start the timer on continuous mode & run
  timer_start_cont(CONFIG_STOPWATCH_TIMER);
}

uint32_t stop_timer() {
  // disable the timer
  timer_halt(CONFIG_STOPWATCH_TIMER);
  __set_interrupt_state(timer_int_state);

  // read the last count
  // this value is in lfxt clock cycles
  uint32_t time;

  time = (msp430_timer_tick << 16) | TA1R;

  timer_halt(CONFIG_STOPWATCH_TIMER);
  timer_reset(CONFIG_STOPWATCH_TIMER);
  timer_IFG_disable(CONFIG_STOPWATCH_TIMER);

  return time;
}

uint32_t get_time() {
  uint32_t time;

  time = (msp430_timer_tick << 16) | TA1R;

  return time;
}

void
    __attribute__((interrupt(STIC3(TIMER, CONFIG_STOPWATCH_TIMER, _A1_VECTOR))))
    STIC3(TIMER, CONFIG_STOPWATCH_TIMER, _A1_ISR)(void) {
  switch (__even_in_range(STIC3(TA, CONFIG_STOPWATCH_TIMER, IV), TAIV__TAIFG)) {
  case TAIV__NONE:
    break; // No interrupt
  case TAIV__TACCR1:
    break; // CCR1 not used
  case TAIV__TACCR2:
    break; // CCR2 not used
  case TAIV__TACCR3:
    break; // reserved
  case TAIV__TACCR4:
    break; // reserved
  case TAIV__TACCR5:
    break; // reserved
  case TAIV__TACCR6:
    break;          // reserved
  case TAIV__TAIFG: // overflow
    msp430_timer_tick++;
    break;
  default:
    break;
  }
}