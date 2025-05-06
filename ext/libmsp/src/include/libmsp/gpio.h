#ifndef INCLUDE_MSPBASE_GPIO_H
#define INCLUDE_MSPBASE_GPIO_H

/**
 * Internal macros
 */
#define __gpio_dir_in(port, pin)                                               \
  P##port##DIR &= ~BIT##pin;                                                   \
  P##port##REN &= ~BIT##pin

#define __gpio_dir_out(port, pin) (P##port##DIR |= BIT##pin)

#define __gpio_sel_0(port, pin) (P##port##SEL0 |= BIT##pin)
#define __gpio_sel_1(port, pin) (P##port##SEL1 |= BIT##pin)

#define __gpio_unsel_0(port, pin) (P##port##SEL0 &= (~BIT##pin))
#define __gpio_unsel_1(port, pin) (P##port##SEL1 &= (~BIT##pin))

#define __gpio_set(port, pin) (P##port##OUT |= BIT##pin)
#define __gpio_clear(port, pin) (P##port##OUT &= ~BIT##pin)
#define __gpio_toggle(port, pin) (P##port##OUT ^= BIT##pin)

#define __gpio_read(port, pin) (P##port##IN & BIT##pin)

#define __gpio_pulldown(port, pin)                                             \
  (P##port##REN |= BIT##pin);                                                  \
  (P##port##OUT &= ~BIT##pin)

#define __gpio_pullup(port, pin)                                               \
  (P##port##REN |= BIT##pin);                                                  \
  (P##port##OUT |= BIT##pin)

#define __gpio_nopull(port, pin) (P##port##REN &= ~BIT##pin)

#define __gpio_int_rise(port, pin) (P##port##IES &= ~BIT##pin)
#define __gpio_int_fall(port, pin) (P##port##IES |= BIT##pin)
#define __gpio_dis_int(port, pin) (P##port##IE &= ~BIT##pin)
#define __gpio_en_int(port, pin) (P##port##IE |= BIT##pin)

/**
 * Disable the GPIO power-on default high-impedance mode to activate
 * previously configured port settings.
 */
#define gpio_unlock() PM5CTL0 &= ~LOCKLPM5

/**
 * Initialize all GPIO ports for optimized power consumption.
 * Everything is selected as low output.
 *    PXOUT = 0; PXDIR = 0xFF
 */
#define gpio_init_all_ports()                                                  \
  P1OUT = 0;                                                                   \
  P1DIR = 0xFF;                                                                \
  P2OUT = 0;                                                                   \
  P2DIR = 0xFF;                                                                \
  P3OUT = 0;                                                                   \
  P3DIR = 0xFF;                                                                \
  P4OUT = 0;                                                                   \
  P4DIR = 0xFF;                                                                \
  P5OUT = 0;                                                                   \
  P5DIR = 0xFF;                                                                \
  P6OUT = 0;                                                                   \
  P6DIR = 0xFF;                                                                \
  P7OUT = 0;                                                                   \
  P7DIR = 0xFF;                                                                \
  P8OUT = 0;                                                                   \
  P8DIR = 0xFF;                                                                \
  PJOUT = 0;                                                                   \
  PJDIR = 0xFF

/**
 * Initialize all GPIO interrupts at startup.
 * Only do at wake up when not from LPMx.5
 */
#define gpio_clear_interrupts()                                                \
  P1IFG = 0;                                                                   \
  P2IFG = 0;                                                                   \
  P3IFG = 0;                                                                   \
  P4IFG = 0;                                                                   \
  P5IFG = 0;                                                                   \
  P6IFG = 0;                                                                   \
  P7IFG = 0;                                                                   \
  P8IFG = 0;

/**
 * Set GPIO pin as input.
 *
 * @param port port number
 * @param pin  pin number
 * @param both port, pin given as a single pair
 */
#define gpio_dir_in(port, pin) __gpio_dir_in(port, pin)
#define gpio_dir_in_(both) __gpio_dir_in(both)

/**
 * Set GPIO pin as output.
 *
 * @param port port number
 * @param pin  pin number
 */
#define gpio_dir_out(port, pin) __gpio_dir_out(port, pin)
#define gpio_dir_out_(both) __gpio_dir_out(both)

/**
 * Select GPIO pin's function.
 *
 * @param port port number
 * @param pin  pin number
 */

#define gpio_sel0(port, pin) __gpio_sel_0(port, pin)
#define gpio_sel1(port, pin) __gpio_sel_1(port, pin)
#define gpio_unsel0(port, pin) __gpio_unsel_0(port, pin)
#define gpio_unsel1(port, pin) __gpio_unsel_1(port, pin)

#define gpio_sel0_(both) __gpio_sel_0(both)
#define gpio_sel1_(both) __gpio_sel_1(both)
#define gpio_unsel0_(both) __gpio_unsel_0(both)
#define gpio_unsel1_(both) __gpio_unsel_1(both)

/**
 * Set GPIO pin's output state.
 *
 * @param port port number
 * @param pin  pin number
 */
#define gpio_set(port, pin) __gpio_set(port, pin)
#define gpio_set_(both) __gpio_set(both)
/**
 * Clear GPIO pin's output state.
 *
 * @param port port number
 * @param pin  pin number
 */
#define gpio_clear(port, pin) __gpio_clear(port, pin)
#define gpio_clear_(both) __gpio_clear(both)
/**
 * Toggle GPIO pin's output state.
 *
 * @param port port number
 * @param pin  pin number
 */
#define gpio_toggle(port, pin) __gpio_toggle(port, pin)
#define gpio_toggle_(both) __gpio_toggle(both)
/**
 * Produce a spike on a GPIO pin, by setting and then clearing
 * the pin's output state.
 *
 * @param port port number
 * @param pin  pin number
 */
#define gpio_spike(port, pin)                                                  \
  __gpio_set(port, pin);                                                       \
  __gpio_clear(port, pin)

#define gpio_spike_(both)                                                      \
  __gpio_set(both);                                                            \
  __gpio_clear(both)

/**
 * Read input GPIO pin's state.
 *
 * @param port port number
 * @param pin  pin number
 */
#define gpio_read(port, pin) __gpio_read(port, pin)
#define gpio_read_(both) __gpio_read(both)
/**
 * Set the resistor pulldown/up state.
 *
 * @param port port number
 * @param pin  pin number
 */
#define gpio_pullup(port, pin) __gpio_pullup(port, pin)
#define gpio_pulldown(port, pin) __gpio_pulldown(port, pin)

#define gpio_pullup_(both) __gpio_pullup(both)
#define gpio_pulldown_(both) __gpio_pulldown(both)

#define gpio_nopull(port, pin) __gpio_nopull(port, pin)
#define gpio_nopull_(both) __gpio_nopull(both)

/**
 * Maniuplate the interruptability of GPIO pin.
 *
 * @param port port number
 * @param pin  pin number
 */
#define gpio_intr_rising(port, pin) __gpio_int_rise(port, pin)
#define gpio_intr_falling(port, pin) __gpio_int_fall(port, pin)
#define gpio_intr_dis(port, pin) __gpio_dis_int(port, pin)
#define gpio_intr_en(port, pin) __gpio_en_int(port, pin)

#define gpio_intr_rising_(both) __gpio_int_rise(both)
#define gpio_intr_falling_(both) __gpio_int_fall(both)
#define gpio_intr_dis_(both) __gpio_dis_int(both)
#define gpio_intr_en_(both) __gpio_en_int(both)

#endif /* INCLUDE_MSPBASE_GPIO_H */