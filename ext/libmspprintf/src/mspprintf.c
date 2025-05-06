#include <libmsp/uart.h>
#include <libmspio/uartio.h>
#include <libmspprintf/mspprintf.h>
#include <msp430.h>

#include <stdarg.h>
#include <stdint.h>

// Tiny printf implementation by oPossum from here:
// http://forum.43oh.com/topic/1289-tiny-printf-c-version/#entry10652
//
// NOTE: We are not using the libc printf because it's huge:
// https://e2e.ti.com/support/development_tools/compiler/f/343/t/442632
static const unsigned long dv[] = {
    //  4294967296      // 32 bit unsigned max
    1000000000, // +0
    100000000,  // +1
    10000000,   // +2
    1000000,    // +3
    100000,     // +4
                //       65535      // 16 bit unsigned max
    10000,      // +5
    1000,       // +6
    100,        // +7
    10,         // +8
    1,          // +9
};

static const char hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
                             '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

static size_t xtoa(uint8_t port, unsigned long x, const unsigned long *dp) {
  size_t count = 0;
  char c;
  unsigned long d;
  if (x) {
    while (x < *dp)
      ++dp;
    do {
      d = *dp++;
      c = '0';
      while (x >= d)
        ++c, x -= d;
      count += PUTC(port, c);
    } while (!(d & 1));
  } else {
    count += PUTC(port, '0');
  }
  return count;
}

static inline size_t puth(uint8_t port, unsigned n) {
  return PUTC(port, hex[n & 15]);
}

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
int _msp_port_vprintf(uint8_t port, const char *format, va_list a) {
  char c;
  int i;
  long n;
  int fill_zeros;
  unsigned d;
  size_t count = 0;

  while ((c = *format++)) {
    if (c == '%') {
      fill_zeros = 0;
    parse_fmt_char:
      switch (c = *format++) {
      case 's': // String
        count += uartio_puts_no_newline(port, va_arg(a, const uint8_t *));
        break;
      case 'c':                              // Char
        count += PUTC(port, va_arg(a, int)); // TODO: 'char' generated a warning
        break;
      case 'i': // 16 bit Integer
      case 'u': // 16 bit Unsigned
        i = va_arg(a, int);
        if (c == 'i' && i < 0) {
          i = -i;
          count += PUTC(port, '-');
        }
        count += xtoa(port, (unsigned)i, dv + 5);
        break;
      case 'l': // 32 bit Long
      case 'n': // 32 bit uNsigned loNg
        n = va_arg(a, long);
        if (c == 'l' && n < 0) {
          n = -n;
          count += PUTC(port, '-');
        }
        count += xtoa(port, (unsigned long)n, dv);
        break;
      case 'x': // 16 bit heXadecimal
        i = va_arg(a, int);
        d = i >> 12;
        if (d > 0 || fill_zeros >= 4)
          count += puth(port, d);
        d = i >> 8;
        if (d > 0 || fill_zeros >= 3)
          count += puth(port, d);
        d = i >> 4;
        if (d > 0 || fill_zeros >= 2)
          count += puth(port, d);
        count += puth(port, i);
        break;
      case '0':
        c = *format++;
        fill_zeros = c - '0';
        goto parse_fmt_char;
      case 0:
        return 0;
      default:
        goto bad_fmt;
      }
    } else {
    bad_fmt:
      count += PUTC(port, c);
    }
  }

  return count;
}