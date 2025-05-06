#ifndef INCLUDE_MSPBASE_UART_H
#define INCLUDE_MSPBASE_UART_H

#include <libmsp/gpio.h>
#include <libmsp/macro_basics.h>
#include <msp430.h>
#include <stdbool.h>
#include <stdint.h>
#define UART_MODULE_COUNT 4
/* With the help of this header, you can setup a uart communication.
 *
 * The basic operations follow a few main components, for more info see page 767
 *
 * UART pins: these pins are usually on the same port and are the TX/RX pins
 *    UART needs to communicate. These pins need to be setup properly and
 *    there is no clear rule for it. Use the device datasheet to figure it out.
 *
 * UART x: the UART module has multiple uart capable modules (?), each is
 *    denominated with an x
 *
 */

#define uart0_pins()                                                           \
  P2SEL1 |= BIT0 | BIT1;                                                       \
  P2SEL0 &= ~(BIT0 | BIT1)

#define uart1_pins()                                                           \
  P2SEL1 |= BIT5 | BIT6;                                                       \
  P2SEL0 &= ~(BIT5 | BIT6)

#define uart2_pins()                                                           \
  P5SEL0 |= BIT4 | BIT5;                                                       \
  P5SEL1 &= ~(BIT4 | BIT5)

#define uart3_pins()                                                           \
  P6SEL0 |= BIT0 | BIT1;                                                       \
  P6SEL1 &= ~(BIT0 | BIT1)

/* Setups UART as follows,
 *    backed by small clock
 *    set to baud rate determined by the user
 *
 * To set the baud rate, a few pieces of information is required,
 *    BRCLK (i.e. clock frequency)
 *    Baud rate
 *
 * Once you have the above information, look at page 782 of the manual,
 *    (or do the math described there).It should give you the following
 *    information,
 *       UCOS16: 0 or 1
 *       UCBRx : some number
 *       UCBRFx: some number
 *       UCBRSx: some number in hex
 *
 * Feed the above to the program in the following order:
 *    ovs: over sample mode, give it UCOS16 (only 1 or 0)
 *    brw: the UCBRx
 *    brf: first modulation stage select, UCBRFx
 *    brs: second modulation stage select, UCBRSx
 */
#define uart_open(x, ovs, brw, brf, brs)                                       \
  STIC3(UCA, x, CTLW0) |= UCSWRST;                                             \
  STIC3(UCA, x, CTLW0) |= UCSSEL__SMCLK;                                       \
  STIC3(UCA, x, BRW) = brw;                                                    \
  STIC3(UCA, x, MCTLW) = ovs | (brf << 4) | (brs << 8);                        \
  STIC3(UCA, x, CTLW0) &= ~UCSWRST

#define uart_close(x) STIC3(UCA, x, CTLW0) |= UCSWRST

#define uart_busy(x)                                                           \
  while (STIC3(UCA, x, STATW) & UCBUSY)                                        \
    ;

/* The pins determine which interrupt should be enabled.
 * Use UCTXIE for TX an UCRXIE for RX
 */
#define uart_intr_en(x, pins) STIC3(UCA, x, IE) |= pins

#define uart_intr_dis(x, pins) STIC3(UCA, x, IE) |= pins

#endif /* INCLUDE_MSPBASE_UART_H */