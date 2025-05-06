#include <libmsp/timer.h>
#include <msp430.h>

// NOTE: check the datasheet to see which CCRn are avilable for your timer.

#define X 0
void __attribute__((interrupt(STIC3(TIMER, X, _A1_VECTOR))))
STIC3(TIMER, X, _A1_ISR)(void) {
  switch (__even_in_range(STIC3(TA, X, IV), TAIV__TAIFG)) {
  case TAIV__NONE:
    break; // No interrupt
  case TAIV__TACCR1:
    break; // CCR1 not used
  case TAIV__TACCR2:
    break; // CCR2 not used
  case TAIV__TACCR3:
    break; // reserved
  case TAIV__TACCR4:
    break; // reserved
  case TAIV__TACCR5:
    break; // reserved
  case TAIV__TACCR6:
    break;          // reserved
  case TAIV__TAIFG: // overflow
    P1OUT ^= BIT0;
    break;
  default:
    break;
  }
}

// FOR CCR0

void __attribute__((interrupt(TIMER0_A0_VECTOR))) Timer0_A0_ISR(void) {
  // Do something
}