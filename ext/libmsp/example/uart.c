#include <libmsp/macro_basics.h>
#include <libmsp/uart.h>
#include <msp430.h>

// Define which uart port you wish to use.
#define X 0
void __attribute__((interrupt(STIC3(EUSCI_A, X, _VECTOR))))
STIC3(USCI_A, X, _ISR)(void) {
  switch (__even_in_range(STIC3(UCA, X, IV), USCI_UART_UCTXCPTIFG)) {
  case USCI_NONE:
    break;
  case USCI_UART_UCRXIFG:
    break;
  case USCI_UART_UCTXIFG:
    break;
  case USCI_UART_UCSTTIFG:
    break;
  case USCI_UART_UCTXCPTIFG:
    break;
  default:
    break;
  }
}