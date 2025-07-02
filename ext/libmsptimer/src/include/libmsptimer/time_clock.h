#ifndef __SOLEMN_TIME
#define __SOLEMN_TIME

#include <libmsptimer/peripheral_defs.h>
#include <libmsptimer/_timekeeper.h>
#include <libmsp/macro_basics.h>
#include <libmsp/mspbase.h>
#include <stdbool.h>
#include <stdint.h>

/*
 * Setups the clocks and manages the timers.
 *
 * As set out in the time module, each second must be 8192 ticks.
 * This is setup using LFXT clock, which operates at 32768 Hz, so
 * timers are setup to divide this value by 4.
 * Timers will be setup in up mode, w/ period 8192 so a ticks occurs at
 * each time point.
 *
 * We do not utilize a remanance timekeeper, so there is no point in having
 *   a non_volatile tick counter. Our goal is to NEVER die.
 *
 * Some notes:
 *   1. TODO: LFXT takes time to wake up and just spins when doing so. Make this
 *            more efficient.
 *
 *   2. Clock 0 is used to remain in group B. Use other group B peripherals when
 * possible. 2.1 TA0 & 3 2.2 Comp 2.3 ADC 2.4 REF
 *
 *   3. The module assumes that interrupts are enabled all the time.
 */

/* ---- Constants & Structs --- */

// Rshift counts to turn a tick into a second
#define TICKS_SEC_SHIFT 13
// Ticks per second
#define TICKS_PER_SEC (1 << TICKS_SEC_SHIFT)
// The seconds increment for each timer iteration
#define SECOND_INCREMENT 1

/*
 * A struct to keep track of time.
 *
 * Time is split between ticks and seconds, where a number of
 * ticks constite a second. The number of ticks per second is
 * closely tied to the clock operation, so ensure this modules matches
 * the clock module.
 *
 * Why split between the seconds and ticks? In case the ticks per seconds
 * changes, the representation itself does not change.
 */

/* ---- Constants & Structs --- */

// TODO: this should either also rely on a remanance keeper or have a global
// counter
typedef struct {
  uint32_t seconds;
  uint16_t ticks;
} time_t;

/* ---- Time Struct Functions --- */

/* Overhead on average 68*1953/1000  ~= 130 cycles */
bool happens_after_time(time_t a, time_t b);

/* Overhead on average: 73*1953/1000 ~= 150 cycles*/
time_t add_times(time_t a, time_t b);
time_t subtract_times(time_t second, time_t first);

/* ---- Time Management --- */

/* Overhead on average: 1953/30 ~= 60 cycles*/
time_t get_current_time();

/* Overhead on average: 100*1953 ~= 195 cycles*/
void sleep_till(time_t deadline);

static inline __attribute__((always_inline)) void sleep_for(time_t duration) {
  time_t current = get_current_time();
  current = add_times(current, duration);
  sleep_till(current);
}

/* ---- Time Management --- */
bool set_kernel_wakeup(time_t deadline);

/* --- Clock Management --- */

// TODO: this needs to be moved to the kernel side of things.
void time_clock_init(bool wait_for_lfxt);

#endif // __SOLEMN_TIME