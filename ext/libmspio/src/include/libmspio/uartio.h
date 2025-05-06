#ifndef INCLUDE_MSPIO_UARTIO_H
#define INCLUDE_MSPIO_UARTIO_H

#include <stddef.h>
#include <stdint.h>

#define BAUD_SUCCESS 1
#define BAUD_FAIL 0

/* This library provides a general view of using the uart to send and recieve
 *     information. It is not extremely efficient since it's general.
 *
 *    To enable LPM when waiting, define UARTSLEEP
 *
 *    Each module has a different interrupt handler. To enable one, define
 *       UARTx_INT_ENABLE, where x is 0-3.
 *
 *    NOTE: ENABLE AT BUILD TIME.
 *
 */

#define PUTC(p, c) uartio_putchar(p, c)

/* Tries to set the baud rate for a port.
 * Tries because it only can determine baudrate for a limited set of frequencies
 *    and only for baudrate 19200 at the moment.
 *    Acceptable SMCLK freq:
 *       8  MHz -> ovs: 1 brw: 26 brf: 0x0  brs: 0xB6
 *       4  MHz -> ovs: 1 brw: 13 brf: 0x0  brs: 0x84
 *       2  MHz -> ovs: 1 brw:  6 brf: 0x8  brs: 0x20
 *       1  MHz -> ovs: 1 brw:  3 brf: 0x4  brs: 0x02
 *       .5 MHz -> ovs: 1 brw:  1 brf: 0xA  brs: 0x00
 *       .25MHz -> ovs: 0 brw: 13 brf: 0x0  brs: 0x00
 *
 * Note: UART setup with this function uses SMCLK, but does not change it.
 *       If successful, it returns BAUD_SUCCESS, else BAUD_FAIL
 */
uint8_t uartio_baud_set(uint8_t port, uint16_t baud);

/* opens and closes UART channells.
 * open uses uartio_baud_set, so there is a chance it may fail atm
 */
uint8_t uartio_open(uint8_t port);
void uartio_close(uint8_t port);

// sends a UART payload with length len on port
size_t uartio_send_sync(uint8_t port, uint8_t *payload, size_t len);

/* receives a UART payload */
size_t uartio_recv_sync(uint8_t port, uint8_t *payload, size_t len);

/* sends one character via uart */
size_t uartio_putchar(uint8_t port, uint8_t c);

/* receive a char via uart */
uint8_t uartio_getchar(uint8_t port);

/* sends a string via uart w/ no new line*/
size_t uartio_puts_no_newline(uint8_t port, const uint8_t *ptr);

/* sends a string via uart w/ new line */
size_t uartio_puts(uint8_t port, const uint8_t *ptr);

#endif /* INCLUDE_MSPIO_UARTIO_H */