#ifndef INCLUDE_MSPBASE_TIMERS_H
#define INCLUDE_MSPBASE_TIMERS_H

#include <libmsp/macro_basics.h>
#include <msp430.h>

/* This header helps the user setup timers A0-4 with their TAxCCR0 register.
 * For more control, look at pages 658...
 *
 * The setup is as follows,
 *    t: selects the timer, A0-4
 *    clock: selects the clock driving the timer, ACLK or SMCLK
 *    div, divexp: clocks have two dividers; div does 1,2, and divexp
 *       does 1-8
 *    count: how many ticks the timer should count before firing an interrupt
 *       for TAtCCTL0
 *
 *    Compare mode so it causes an interrupt. look into timer.c for interrupt *
 * vector examples
 *
 * setup will be in up mode once start_timer is called
 *
 */

#define TIMERDIV_1 ID_0
#define TIMERDIV_2 ID_1
#define TIMERDIV_4 ID_2
#define TIMERDIV_8 ID_3

#define timer_setup_up(t, clock, div, divexp, count)                           \
  STIC3(TA, t, CTL) = STICH(TASSEL__, clock) | STICH(TIMERDIV_, div);          \
  STIC3(TA, t, EX0) = STICH(TAIDEX__, divexp);                                 \
  STIC3(TA, t, CCTL0) = CCIE;                                                  \
  STIC3(TA, t, CCR0) = count

#define timer_setup_cont(t, clock, div, divexp)                                \
  STIC3(TA, t, CTL) = STICH(TASSEL__, clock) | STICH(TIMERDIV_, div) | TAIE;   \
  STIC3(TA, t, EX0) = STICH(TAIDEX__, divexp)

#define timer_start_up(t) STIC3(TA, t, CTL) |= MC__UP | TACLR

#define timer_start_cont(t) STIC3(TA, t, CTL) |= MC__CONTINOUS | TACLR

#define timer_halt(t) STIC3(TA, t, CTL) &= ~(MC)

#define timer_resume_up(t) STIC3(TA, t, CTL) |= MC__UP

#define timer_resume_cont(t) STIC3(TA, t, CTL) |= MC__CONTINOUS

#define timer_reset(t) STIC3(TA, t, CTL) |= TACLR

#define timer_IFG_enable(t) STIC3(TA, t, CTL) |= TAIE

#define timer_IFG_disable(t) STIC3(TA, t, CTL) &= ~TAIE

#define timer_CCIE_disable(t) STIC3(TA, t, CCTL0) &= ~CCIE

#define timer_CCIE_enable(t) STIC3(TA, t, CCTL0) |= CCIE

#endif /* INCLUDE_MSPBASE_TIMERS_H */