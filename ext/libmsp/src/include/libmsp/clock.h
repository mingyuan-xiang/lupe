#ifndef INCLUDE_MSPBASE_CLOCK_H
#define INCLUDE_MSPBASE_CLOCK_H

#include <libmsp/macro_basics.h>
#include <msp430.h>
#include <stdint.h>

/* This header assumes that several macros are defined in advance, before its
 * inclusion. If such values are not defined, you may get errors. Full
 * description are in page 93 of the manual for MSPFR5994.
 *
 * The base clock sources are as follows,
 *    LFXTCLK: 32768 Hz, crystal takes a bit to settle (>800 ms), so cannot use
 *    HFXTCLK: 4-24M Hz, crystal, energy hungry
 *    VLOCLK: typical 10k Hz, very low power but inaccurate, BAD (about 8.3kHZ?)
 *    DCOCLK: variable, controlled by digital oscilations, main clock
 *    MODCLK: typical 5M Hz, internal LP, OK power, can be inaccurate
 *    LFMODCLK: above divided by 128
 *
 * The clock signals available are,
 *    ACLK: Auxiliary clock
 *    MCLK: Master clock, used bu CPU and most of system
 *    SMCLK: Subsystem master clock. Mostly used by peripherals
 *    MODCLK: Module clock, may be used by peripherals, etc.
 *    VLOCLK: VLO clock, may be used by peripherals, etc.
 *
 * Note that the MODCLK and VLOCLK are enabled if you are using them.
 * To not turn them on, don't use them as clock sources. Save the energy.
 *
 * The values you SHOULD define are as follow,
 * DCOFREQ : DCO frequency. Valid values are (in 1 MHZ):
 *             1, 4, 8, 16
 *
 * ACLK_SRC: LFXTCLK, VLOCLK or LFMODCLK
 *
 * SMCLK_SRC: LFXTCLK, VLOCLK, LFMODCLK, DCOCLK, MODCLK, HFXTCLK
 *
 * MCLK_SRC: LFXTCLK, VLOCLK, LFMODCLK, DCOCLK, MODCLK, HFXTCLK
 *
 * ACLK_DIV, SMCLK_DIV, MCLK_DIV: 1,2,4,8,16,32
 */

#define DCOFREQ__1 DCOFSEL_0
#define DCOFREQ__4 DCOFSEL_3
#define DCOFREQ__8 DCOFSEL_6
#define DCOFREQ__16 (DCORSEL | DCOFSEL_4)

#define FRAMST__1 NWAITS_0
#define FRAMST__4 NWAITS_0
#define FRAMST__8 NWAITS_0
#define FRAMST__16 NWAITS_1

#ifndef DCOFREQ
#define DCOFREQ 1
#endif

#ifndef ACLK_SRC
#define ACLK_SRC VLOCLK
#endif

#ifndef MCLK_SRC
#define MCLK_SRC DCOCLK
#endif

#ifndef SMCLK_SRC
#define SMCLK_SRC DCOCLK
#endif

#ifndef ACLK_DIV
#define ACLK_DIV 1
#endif

#ifndef MCLK_DIV
#define MCLK_DIV 1
#endif

#ifndef SMCLK_DIV
#define SMCLK_DIV 1
#endif
/* UART breaks if some waiting is not done after changing DCOFREQ. The wait
 * times are informed by the errata + some experiments with different speeds.
 */
#if DCOFREQ == 1
#define ERRATA_WAIT 60
#elif DCOFREQ == 4
#define ERRATA_WAIT 60
#elif DCOFREQ == 8
#define ERRATA_WAIT 60
#elif DCOFREQ == 16
#define ERRATA_WAIT 60
#endif

/* updates the DCO clock frequency per errata */
#define dco_update(freq)                                                       \
  {                                                                            \
    FRCTL0 = FRCTLPW | STICH(FRAMST__, freq);                                  \
    uint16_t __temp_clock_div = CSCTL3;                                        \
    CSCTL0_H = CSKEY_H;                                                        \
    CSCTL1 = DCOFSEL_0;                                                        \
    /* Fix for CS12 per errata */                                              \
    CSCTL3 = DIVA__4 | DIVS__4 | DIVM__4;                                      \
    CSCTL1 = STICH(DCOFREQ__, freq);                                           \
    /* Per errata have to wait 10us, max speed of 16MHz requires 60 cycles*/   \
    __delay_cycles(ERRATA_WAIT);                                               \
    CSCTL3 = __temp_clock_div;                                                 \
    CSCTL0_H = 0;                                                              \
  }

/* Initilizes all the clocks */
#define clock_init()                                                           \
  {                                                                            \
    FRCTL0 = FRCTLPW | STICH(FRAMST__, DCOFREQ);                               \
    CSCTL0_H = CSKEY_H;                                                        \
    CSCTL1 = DCOFSEL_0;                                                        \
    CSCTL2 = STICH(SELA__, ACLK_SRC) | STICH(SELM__, MCLK_SRC) |               \
             STICH(SELS__, SMCLK_SRC);                                         \
    /* Fix for CS12 per errata */                                              \
    CSCTL3 = DIVA__4 | DIVS__4 | DIVM__4;                                      \
    CSCTL1 = STICH(DCOFREQ__, DCOFREQ);                                        \
    /* Per errata have to wait 10us, max speed of 16MHz requires 60 cycles*/   \
    __delay_cycles(ERRATA_WAIT);                                               \
    CSCTL3 = STICH(DIVA__, ACLK_DIV) | STICH(DIVM__, MCLK_DIV) |               \
             STICH(DIVS__, SMCLK_DIV);                                         \
    CSCTL4 = HFXTOFF | LFXTOFF;                                                \
    CSCTL0_H = 0;                                                              \
  }

/* updates the clock divisors */
#define clock_div_update(master, small, aux, old)                              \
  CSCTL0_H = CSKEY_H;                                                          \
  old = CSCTL3;                                                                \
  CSCTL3 = STICH(DIVA__, aux) | STICH(DIVM__, master) | STICH(DIVS__, small);  \
  CSCTL0_H = 0

/* updates the clock sources */
#define clock_src_update(master, small, aux, old)                              \
  CSCTL0_H = CSKEY_H;                                                          \
  old = CSCTL2;                                                                \
  CSCTL2 = STICH(SELA__, aux) | STICH(SELM__, master) | STICH(SELS__, small);  \
  CSCTL0_H = 0

/* updates all clock sources */
#define clock_src_update_all(new, old)                                         \
  CSCTL0_H = CSKEY_H;                                                          \
  old = CSCTL2;                                                                \
  CSCTL2 = new;                                                                \
  CSCTL0_H = 0;

/*
 * sets up the lfxt as ACLK with pre-existing divs and ensures it is up
 * and running. Keep in mind that this ueses PJ.5-6
 *
 */
#define lfxt_start(tmp)                                                        \
  PJSEL0 |= BIT4 | BIT5;                                                       \
  CSCTL0_H = CSKEY_H;                                                          \
  tmp = CSCTL2;                                                                \
  CSCTL2 &= ~SELA;                                                             \
  CSCTL2 |= SELA__LFXTCLK;                                                     \
  CSCTL4 &= ~LFXTOFF;                                                          \
  do {                                                                         \
    CSCTL5 &= ~LFXTOFFG;                                                       \
    SFRIFG1 &= ~OFIFG;                                                         \
  } while (SFRIFG1 & OFIFG);                                                   \
  CSCTL0_H = 0

#define lfxt_stop(tmp)                                                         \
  CSCTL0_H = CSKEY_H;                                                          \
  CSCTL2 = tmp;                                                                \
  CSCTL4 |= LFXTOFF;                                                           \
  CSCTL0_H = 0

/* determines the clock speed */
// TODO: complete this
uint32_t mclock_freq();
uint32_t sclock_freq();
uint32_t aclock_freq();

/* Helps determine the i2c communication clock frequency.
 * Takes SMCLK and figures out what div would make it 100khz.
 * A 0 means the clock is not good for i2c (e.g. frequency too small, etc)
 */
uint16_t i2c_clock_div();
#endif /* INCLUDE_MSPBASE_CLOCK_H */