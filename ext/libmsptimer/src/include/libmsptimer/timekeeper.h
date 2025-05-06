#ifndef INCLUDE_TIMEKEEPER_H
#define INCLUDE_TIMEKEEPER_H
#include <libmsp/macro_basics.h>
#include <stdint.h>
#include <string.h>

#ifndef CONFIG_STOPWATCH_TIMER
#define CONFIG_STOPWATCH_TIMER 1
#endif

/* If using the timers in this library, initilize them as follows
 * Similarly, you can stop the initilized processes.
 *
 * Takes circa 35 ms experiments, i.e. don't use this library in
 * production.
 */
void timers_init();
void timers_stop();

/*
* This function starts the timer.
*/
void start_timer();

/*
* This function stops the timer and returns the number of clock cycles
* @return the number of clock cycles
*/
uint32_t stop_timer();

uint32_t get_time();

#endif /* INCLUDE_TIMEKEEPER_H */