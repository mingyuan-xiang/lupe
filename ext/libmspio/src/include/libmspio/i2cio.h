#ifndef INCLUDE_MSPIO_I2CIO_H
#define INCLUDE_MSPIO_I2CIO_H

#include <libmsp/clock.h>
#include <libmsp/i2c.h>
#include <stddef.h>

#define i2cio_read_single(x, reg, var)                                         \
  i2c_int_clear(x);                                                            \
  i2c_tx_start(x);                                                             \
  i2c_wait_txbusy(x);                                                          \
  i2c_txbuf(x, reg);                                                           \
  i2c_wait_txbusy(x);                                                          \
  i2c_rx(x);                                                                   \
  i2c_repeated(x);                                                             \
  i2c_wait_sttbusy(x);                                                         \
  i2c_stop(x);                                                                 \
  i2c_wait_rxbusy(x);                                                          \
  i2c_rxbuf(x, var);                                                           \
  i2c_wait_busy(x)

#define i2cio_write_single(x, reg, var)                                        \
  i2c_int_clear(x);                                                            \
  i2c_tx_start(x);                                                             \
  i2c_wait_txbusy(x);                                                          \
  i2c_txbuf(x, reg);                                                           \
  i2c_wait_txbusy(x);                                                          \
  i2c_txbuf(x, var);                                                           \
  i2c_wait_txbusy(x);                                                          \
  i2c_stop(x);                                                                 \
  i2c_wait_busy(x)

#define i2cio_read_multi(x, reg, arr, count)                                   \
  i2c_int_clear(x);                                                            \
  i2c_tx_start(x);                                                             \
  i2c_wait_txbusy(x);                                                          \
  i2c_txbuf(x, reg);                                                           \
  i2c_wait_txbusy(x);                                                          \
  i2c_rx(x);                                                                   \
  i2c_repeated(x);                                                             \
  if (count == 1) {                                                            \
    i2c_wait_sttbusy(x);                                                       \
    i2c_stop(x);                                                               \
  }                                                                            \
  {                                                                            \
    for (size_t i = 0; i < count; i++) {                                       \
      i2c_wait_rxbusy(x);                                                      \
      i2c_rxbuf(x, arr[i]);                                                    \
                                                                               \
      if (i == (count - 2))                                                    \
        i2c_stop(x);                                                           \
    }                                                                          \
  }                                                                            \
  i2c_wait_busy(x)

#define i2cio_write_multi(x, reg, arr, count)                                  \
  i2c_int_clear(x);                                                            \
  i2c_tx_start(x);                                                             \
  i2c_wait_txbusy(x);                                                          \
  i2c_txbuf(x, reg);                                                           \
  {                                                                            \
    for (size_t i = 0; i < count; i++) {                                       \
      i2c_wait_txbusy(x);                                                      \
      i2c_txbuf(x, arr[i]);                                                    \
    }                                                                          \
  }                                                                            \
  i2c_stop(x);                                                                 \
  i2c_wait_busy(x)

#define i2cio_init_maker(name)                                                 \
  void name(uint8_t port, uint8_t addr) {                                      \
    uint16_t div = i2c_clock_div();                                            \
    switch (port) {                                                            \
    case 0:                                                                    \
      i2c0_pins();                                                             \
      i2c_init(0, addr, div);                                                  \
      break;                                                                   \
    case 1:                                                                    \
      i2c1_pins();                                                             \
      i2c_init(1, addr, div);                                                  \
      break;                                                                   \
    case 2:                                                                    \
      i2c2_pins();                                                             \
      i2c_init(2, addr, div);                                                  \
      break;                                                                   \
    case 3:                                                                    \
      i2c3_pins();                                                             \
      i2c_init(3, addr, div);                                                  \
      break;                                                                   \
    }                                                                          \
  }

#endif /* INCLUDE_MSPIO_I2CIO_H */