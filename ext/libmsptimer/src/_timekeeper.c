#include <libmsp/clock.h>
#include <libmsp/gpio.h>
#include <libmsp/timer.h>
#include <libmspprintf/mspprintf.h>
#include <msp430.h>
#include <string.h>

#include <stdbool.h>
#include <stdint.h>

#include <libmsptimer/_timekeeper.h>

static uint32_t stop_watch_tick = 0;
static uint32_t sleep_tick = 0;
static uint16_t sleep_last = 0;

static uint16_t old_csctl2 = 0;
void timers_init() { lfxt_start(old_csctl2); }
void timers_stop() { lfxt_stop(old_csctl2); }

// Measures how long a function takes.
static inline uint32_t _stop_watch_(stop_watch_fn fn, void *arg, bool aclk) {
  stop_watch_tick = 0;

  if (aclk) {
    // Timer sourced by AUX and interrupt setup
    timer_setup_cont(CONFIG_STOPWATCH_TIMER, ACLK, 1, 1);
  } else {
    timer_setup_cont(CONFIG_STOPWATCH_TIMER, SMCLK, 1, 1);
  }
  unsigned int int_state = __get_interrupt_state();
  __enable_interrupt();

  // start the timer on continuous mode & run
  timer_start_cont(CONFIG_STOPWATCH_TIMER);

  fn(arg);

  // disable the timer
  timer_halt(CONFIG_STOPWATCH_TIMER);
  __set_interrupt_state(int_state);

  // read the last count
  // this value is in lfxt clock cycles
  uint32_t time;

  // TODO: this is a bug?
  time = (stop_watch_tick << 16) | TA1R;

  timer_halt(CONFIG_STOPWATCH_TIMER);
  timer_reset(CONFIG_STOPWATCH_TIMER);
  timer_IFG_disable(CONFIG_STOPWATCH_TIMER);

  return time;
}

#ifdef CONFIG_TIMER_ALPACA
#include <libalpaca/alpaca.h>
#include <libalpaca/utility.h>
TASK(CONFIG_TIMER_ALPACA, stop_watch_task);
__nv task_t *stop_watch_task_todo;
__nv uint32_t stop_watch_task_count;
__nv unsigned int _int_state;
void stop_watch_task() {
  uint16_t state = CUR_SCRATCH[0];
  if (state == 0) {
    stop_watch_tick = 0;
    timer_setup_cont(CONFIG_STOPWATCH_TIMER, ACLK, 1, 1);
    _int_state = __get_interrupt_state();
    stop_watch_task_todo->info.return_task = CUR_TASK;
    scratch_bak[0] = 1;
    write_to_gbuf((uint8_t *)scratch_bak, (uint8_t *)CUR_SCRATCH,
                  sizeof(uint16_t));
    __enable_interrupt();
    timer_start_cont(CONFIG_STOPWATCH_TIMER);
    transition_to(stop_watch_task_todo);
  } else {
    // disable the timer
    timer_halt(CONFIG_STOPWATCH_TIMER);
    __set_interrupt_state(_int_state);

    // read the last count
    // this value is in lfxt clock cycles
    stop_watch_task_count = ((uint32_t)stop_watch_tick << 16) | TA1R;

    timer_halt(CONFIG_STOPWATCH_TIMER);
    timer_reset(CONFIG_STOPWATCH_TIMER);
    timer_IFG_disable(CONFIG_STOPWATCH_TIMER);
    setup_cleanup(CUR_TASK);
    TRANSITION_TO(task_cleanup);
  }
}
#endif

uint32_t stop_watch_cycle(stop_watch_fn fn, void *arg, bool aclk) {
  return _stop_watch_(fn, arg, aclk);
}

uint32_t stop_watch_ms(stop_watch_fn fn, void *arg) {
  uint32_t t = _stop_watch_(fn, arg, true);

  // turn into ms
  t *= 1000;
  t = t >> 15;
  return t;
}

static inline void _sleep_timer_(bool aclk) {
  if (sleep_last == 0 && sleep_tick == 0)
    return;

  // Timer sourced by ACLK and interrupt setup
  unsigned int int_state = __get_interrupt_state();
  __enable_interrupt();

  if (sleep_tick == 0) {
    timer_setup_up(CONFIG_SLEEP_TIMER, ACLK, 1, 1, sleep_last);
    timer_start_up(CONFIG_SLEEP_TIMER);
  } else {
    timer_setup_cont(CONFIG_SLEEP_TIMER, ACLK, 1, 1);
    timer_start_cont(CONFIG_SLEEP_TIMER);
  }

  CONFIG_TIMEKEEPER_SLEEP;

  _set_interrupt_state(int_state);
  timer_halt(CONFIG_SLEEP_TIMER);
  timer_reset(CONFIG_SLEEP_TIMER);
  timer_CCIE_disable(CONFIG_SLEEP_TIMER);

  sleep_last = 0;
  sleep_tick = 0;
}

void sleep_cycle(uint32_t end, bool aclk) {
  sleep_tick = end >> 16;
  sleep_last = end & 0xFFFF;
  _sleep_timer_(aclk);
}

void sleep_ms(uint32_t end) {
  // Turn end from ms into a lfxt clock cycles

  // each 2000 ms, the clock incurr one tick, soo..
  sleep_tick = end / 2000;

  // // the rest should get translated into clock cycles
  end %= 2000;
  end = end << 15;
  end /= 1000;

  sleep_last = end;

  _sleep_timer_(true);
}

static void _smlk_cycles_kernel(void *x) {
  sleep_tick = 0;
  sleep_last = 32768;
  _sleep_timer_(true);
}

uint32_t measure_freq(uint8_t c) {
  // This function is for diagnositcs mostly, so it's ok to have this 1 extra
  // save function in there. It'll be fine.
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
  uint16_t new, old;
  uint32_t x = 0;
  switch (c) {
  case 1:
    clock_src_update(DCOCLK, DCOCLK, LFXTCLK, old);
    x = stop_watch_cycle(_smlk_cycles_kernel, 0, false);
    clock_src_update_all(old, new);
    break;
  case 2:
    clock_src_update(DCOCLK, MODCLK, LFXTCLK, old);
    x = stop_watch_cycle(_smlk_cycles_kernel, 0, false);
    clock_src_update_all(old, new);
    break;
  case 3:
    clock_src_update(DCOCLK, LFMODCLK, LFXTCLK, old);
    x = stop_watch_cycle(_smlk_cycles_kernel, 0, false);
    clock_src_update_all(old, new);
    break;
  case 4:
    clock_src_update(DCOCLK, VLOCLK, LFXTCLK, old);
    x = stop_watch_cycle(_smlk_cycles_kernel, 0, false);
    clock_src_update_all(old, new);
    break;
  case 5:
    x = stop_watch_cycle(_smlk_cycles_kernel, 0, true);
    break;
  default:
    break;
  }
  return x;
}

stop_watch_fn _stop_watch_repeated_fn;
size_t _stop_watch_repeated_count;
void _stop_watch_repeated_bench1(void *arg) {
  for (size_t i = _stop_watch_repeated_count; i != 0; i--) {
    _stop_watch_repeated_fn(arg);
  }
}
void _stop_watch_repeated_bench2(void *arg) {
  for (size_t i = _stop_watch_repeated_count; i != 0; i--) {
    _stop_watch_repeated_fn(arg);
    _stop_watch_repeated_fn(arg);
  }
}

uint32_t stop_watch_repeated_cycles(stop_watch_fn fn, void *arg, size_t count) {
  _stop_watch_repeated_fn = fn;
  uint32_t dco_freq = measure_freq(1) >> 15;
  _stop_watch_repeated_count = count << 1;
  uint32_t count1 = stop_watch_cycle(_stop_watch_repeated_bench1, arg, true);
  _stop_watch_repeated_count = count;
  uint32_t count2 = stop_watch_cycle(_stop_watch_repeated_bench2, arg, true);
  uint32_t single = ((2 * count2 - count1) * dco_freq);
  return single / (count << 1);
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
    stop_watch_tick++;
    break;
  default:
    break;
  }
}

void __attribute__((interrupt(STIC3(TIMER, CONFIG_SLEEP_TIMER, _A1_VECTOR))))
STIC3(TIMER, CONFIG_SLEEP_TIMER, _A1_ISR)(void) {
  switch (__even_in_range(STIC3(TA, CONFIG_SLEEP_TIMER, IV), TAIV__TAIFG)) {
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
    // check to see if we need to switch from cont mode to up mode
    if (--sleep_tick == 0) {
      // time's up
      if (sleep_last == 0) {
        CONFIG_TIMEKEEPER_SLEEP_EXIT;
      } else {
        timer_IFG_disable(CONFIG_SLEEP_TIMER);
        timer_setup_up(CONFIG_SLEEP_TIMER, ACLK, 1, 1, sleep_last);
        timer_start_up(CONFIG_SLEEP_TIMER);
      }
    }
    break;
  default:
    break;
  }
}

void __attribute__((interrupt(STIC3(TIMER, CONFIG_SLEEP_TIMER, _A0_VECTOR))))
STIC3(TIMER, CONFIG_SLEEP_TIMER, _A0_ISR)(void) {
  CONFIG_TIMEKEEPER_SLEEP_EXIT;
}