#ifndef INCLUDE_LIBMSP_COMP
#define INCLUDE_LIBMSP_COMP
#include <libmsp/gpio.h>
#include <libmsp/macro_basics.h>
#include <msp430.h>

/* Sets the gpio pin to become analog input.
 * All Cx pins on MSP430fr5994 behave similarly, so this is safe.
 */
#define comp_gpio(port, pin)                                                   \
  __gpio_sel_0(port, pin);                                                     \
  __gpio_sel_1(port, pin)

#define comp_gpio_(both)                                                       \
  __gpio_sel_0(both);                                                          \
  __gpio_sel_1(both)

#define comp_on() CECTL1 |= CEON
#define comp_off() CECTL1 &= ~CEON

// After setup, cmp CERDYIFG is set once comp is ready
// until then, it will be zero, hence, spin until it is one.
// The bit has to be cleared by software, so clean up
#define comp_settle()                                                          \
  {                                                                            \
    while (!(CEINT & CERDYIFG))                                                \
      ;                                                                        \
    CEINT &= ~CERDYIFG;                                                        \
  }

// When turning on and settling, disable interrupts first
#define comp_on_settle(POWER)                                                  \
  {                                                                            \
    uint16_t old_ceint = CEINT;                                                \
    CEINT = 0;                                                                 \
    CECTL1 |= STICH(CEPWRMD_, POWER) | CEON;                                   \
    while (!(CEINT & CERDYIFG))                                                \
      ;                                                                        \
    CEINT &= ~CERDYIFG;                                                        \
    CEINT = old_ceint;                                                         \
  }

#define comp_int_enable() CEINT |= CEIE
#define comp_int_disable() CEINT &= ~CEIE

#endif // INCLUDE_LIBMSP_DMA