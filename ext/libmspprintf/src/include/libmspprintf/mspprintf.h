#ifndef INCLUDE_MSPPRINTF_H
#define INCLUDE_MSPPRINTF_H

#include <stdarg.h>
#include <stdint.h>

/* This library uses one of the UART channels to setup a uart communication.*/

int _msp_port_vprintf(uint8_t port, const char *format, va_list a);

/* Prints out to the desired port.
 *
 * Formatting guidlines:
 * %c - Character
 * %s - String
 * %i - signed Integer(16 bit)
 * %u - Unsigned integer (16 bit)
 * %x - heXadecimal (16 bit)
 * %l - signed Long (32 bit)
 * %n - uNsigned loNg (32 bit)
 */
static inline int msp_port_printf(uint8_t port, const char *format, ...) {
  va_list a;
  va_start(a, format);
  int ret = _msp_port_vprintf(port, format, a);
  va_end(a);
  return ret;
}

#ifdef CONFIG_PRINT_PORT
/* Formatting guidlines:
 * %c - Character
 * %s - String
 * %i - signed Integer (16 bit)
 * %u - Unsigned integer (16 bit)
 * %x - heXadecimal (16 bit)
 * %l - signed Long (32 bit)
 * %n - uNsigned loNg (32 bit)
 */
static inline int msp_printf(const char *format, ...) {
  va_list a;
  va_start(a, format);
  int ret = _msp_port_vprintf(CONFIG_PRINT_PORT, format, a);
  va_end(a);
  return ret;
}
#endif /* CONFIG_PRINT_PORT */
#endif /* INCLUDE_MSPPRINTF_H */