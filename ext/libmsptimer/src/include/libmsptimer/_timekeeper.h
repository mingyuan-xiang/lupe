#ifndef INCLUDE_TIMEKEEPER_H
#define INCLUDE_TIMEKEEPER_H
#include <libmsp/macro_basics.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/* This library provides timekeeper functionlity as follows,
 *    stop_watch : measures how long a function takes using lfxt
 *                 accurate, but cannot be run intermittently
 *    sleep_timer: sleeps for a certain duration
 */
#ifndef CONFIG_SLEEP_TIMER
#define CONFIG_SLEEP_TIMER 2
#endif

#ifndef CONFIG_STOPWATCH_TIMER
#define CONFIG_STOPWATCH_TIMER 1
#endif

#ifndef CONFIG_TIMEKEEPER_SLEEP
#define CONFIG_TIMEKEEPER_SLEEP LPM3
#define CONFIG_TIMEKEEPER_SLEEP_EXIT LPM3_EXIT
#endif

/* Function type for the stopwatch function. */
typedef void (*stop_watch_fn)(void *);

/* If using the timers in this library, initilize them as follows
 * Similarly, you can stop the initilized processes.
 *
 * Takes circa 35 ms experiments, i.e. don't use this library in
 * production.
 */
void timers_init();
void timers_stop();

/* Measures how long a function takes with LFXTCLK for 1 operation.
 * fn should not use ACLK. ACLK & PJ.5-6 settings are altered
 * in this function.
 *
 * The return value is in milli-seconds or cycles
 * lfxt runs the timer in with lfxtclk; if false, SMCLK is used
 */
uint32_t stop_watch_cycle(stop_watch_fn fn, void *arg, bool aclk);
uint32_t stop_watch_ms(stop_watch_fn fn, void *arg);

/* Uses the SMCLK or ACLK (LFXTCLK ideally) to sleep for a certain duration in
 * ms Once the sleep is over, the function will return. ACLK & PJ.5-6 settings
 * are altered in this function.
 *
 * end is either in milli-seconds or cycles.
 * Note: if using SMCLK, don't use sleep_timer_ms. It DOES NOT work due to math.
 */
void sleep_cycle(uint32_t end, bool aclk);
void sleep_ms(uint32_t end);

/* Measures how many cycles a clock approximately takes in 1 second.
 * To do so, it replaces the SMCLK with selected clk source and runs
 * lfxt for 32768 cycles.
 * In essence, this function runs a sleep timer with 32768 on the LFXT
 * and uses a stop watch with the smlk. So in reality, the value returned
 * is how long it took for the 32768 LFXT cycles to run + stopwatch DCO
 * overhead. With DCO set to 1 MHz, this takes 3 LFXT cycles, approximately 0.1
 * ms or 90-100 cycles.
 * values for c go as follows,
 *    1: DCO
 *    2: MODCLK
 *    3: LFMODCLK
 *    4: VLOCLK
 *    5: LFXTCLK (calibrated based on this)
 *    6: HFXTCLK
 *
 * Assumes lfxtclk is setup by timers_init
 */
uint32_t measure_freq(uint8_t c);

/* Measures how many DCO cycles a function takes.
 * Order of operation is as follows:
 *	1. Measures cycle counts per 1 second.
 *	2. Attempts function for 2 * count times.
 *	3. Attempts function twice for count times.
 *	4. Calculates how many cycles the function takes.
 *
 * Note 1: Ensure that the called function's side effects do not inhibit
 * continued operation.
 *
 * Note 2: The returned count includes the cost of calling and returning
 * from the function is approx 25 cycles.
 */
uint32_t stop_watch_repeated_cycles(stop_watch_fn fn, void *arg, size_t count);

#endif /* INCLUDE_TIMEKEEPER_H */