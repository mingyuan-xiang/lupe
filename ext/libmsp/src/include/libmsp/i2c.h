#ifndef INCLUDE_MSPBASE_I2C_H
#define INCLUDE_MSPBASE_I2C_H

#include <msp430.h>

#define i2c0_pins()                                                            \
  P1SEL1 |= BIT6 | BIT7;                                                       \
  P1SEL0 &= ~(BIT6 | BIT7)

#define i2c0_clear() P1SEL1 &= ~(BIT6 | BIT7)

#define i2c1_pins()                                                            \
  P5SEL0 |= BIT0 | BIT1;                                                       \
  P5SEL1 &= ~(BIT0 | BIT1)

#define i2c1_clear() P5SEL0 &= ~(BIT0 | BIT1)

#define i2c2_pins()                                                            \
  P7SEL0 |= BIT0 | BIT1;                                                       \
  P7SEL1 &= ~(BIT0 | BIT1)

#define i2c2_clear() P7SEL0 &= ~(BIT0 | BIT1)

#define i2c3_pins()                                                            \
  P6SEL0 |= BIT5 | BIT4;                                                       \
  P6SEL1 &= ~(BIT5 | BIT4)

#define i2c3_clear() P6SEL0 &= ~(BIT5 | BIT4)

// Turns out I2CSA is defined as a macro, so normal substitution wouldn't work
#define __i2csa(x) __STIC3(UCB, x, I2CSA)

#define i2c_init(x, peripheral_addr, div)                                      \
  STIC3(UCB, x, CTLW0) |= UCSWRST;                                             \
  __i2csa(x) = peripheral_addr;                                                \
  STIC3(UCB, x, CTLW0) |= UCMODE_3 | UCMST | UCSYNC | UCSSEL__SMCLK;           \
  STIC3(UCB, x, BRW) = div;                                                    \
  STIC3(UCB, x, CTLW0) &= ~UCSWRST

#define i2c_m(x, peripheral)                                                   \
  STIC3(UCB, x, CTLW0) |= UCSWRST;                                             \
  STIC3(UCB, x, I2CSA) = peripheral_addr;                                      \
  STIC3(UCB, x, CTLW0) &= ~UCSWRST

#define i2c_clk(x, div)                                                        \
  STIC3(UCB, x, CTLW0) |= UCSWRST;                                             \
  STIC3(UCB, x, BRW) = div;                                                    \
  STIC3(UCB, x, CTLW0) &= ~UCSWRST

#define i2c_tx_start(x) STIC3(UCB, x, CTLW0) |= UCTR | UCTXSTT

#define i2c_repeated(x) STIC3(UCB, x, CTLW0) |= UCTXSTT

#define i2c_rx(x) STIC3(UCB, x, CTLW0) &= ~UCTR

#define i2c_stop(x) STIC3(UCB, x, CTLW0) |= UCTXSTP

#define i2c_rxbuf(x, var) var = STIC3(UCB, x, RXBUF)

#define i2c_txbuf(x, var) STIC3(UCB, x, TXBUF) = var

#define i2c_int_clear(x) STIC3(UCB, x, IFG) &= ~(UCTXIFG | UCRXIFG)

#define i2c_sttbusy(x) STIC3(UCB, x, CTLW0) & UCTXSTT

#define i2c_wait_sttbusy(x) while (i2c_sttbusy(x))

#define i2c_busy(x) STIC3(UCB, x, STATW) & UCBBUSY

#define i2c_wait_busy(x) while (i2c_busy(x))

#define i2c_rxbusy(x) STIC3(UCB, x, IFG) & UCRXIFG

#define i2c_wait_rxbusy(x) while (!(i2c_rxbusy(x)))

#define i2c_txbusy(x) STIC3(UCB, x, IFG) & UCTXIFG

#define i2c_wait_txbusy(x) while (!(i2c_txbusy(x)))

#endif /* INCLUDE_MSPBASE_I2C_H */