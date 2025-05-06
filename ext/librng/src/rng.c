#include <libmsp/clock.h>
#include <libmsp/timer.h>
#include <librng/rng.h>

__nv uint16_t pcg_state = BASIC_SEED;

/* Not intermittent safe odered array shuffle */
void shuffle_array(uint16_t *ptr, size_t length) {
  for (size_t i = length - 1; i != 0; i--) {
    size_t s = prand_range(length);
    uint16_t temp = ptr[s];
    ptr[s] = ptr[i];
    ptr[i] = temp;
  }
}
/*
 * Random number generator based on clocks; run this to seed your function,
 * persumable at function program start before any interrupts or clocks have
 * been set.
 *
 * Adopted by Algorithm from TI SLAA338:
 * https://www.ti.com/lit/an/slaa338a/slaa338a.pdf
 *
 * Another source (though not for msp430):
 * https://github.com/0/msp430-rng
 */
uint16_t clock_rand() {
  uint16_t new, old;
  uint16_t result = 0;

  // setup the clock
  clock_src_update(DCOCLK, DCOCLK, VLOCLK, old);

  // save old settings
  uint16_t oldcctl2 = TA0CCTL2;
  uint16_t oldclt = TA0CTL;
  unsigned int int_state = __get_interrupt_state();
  __disable_interrupt();

  // TA0 is connected to ACLK on CCI2B
  // Setup: Caputre Mode on rising edge
  //			| CCI2B
  //			| Sync Capture
  //			| Enable Capture
  //			| Enable Intterupt
  TA0CCTL2 = CM_1 | CCIS_1 | SCS | CAP | CCIE;

  // Use Small clock as source | Start timer in continous mode
  TA0CTL = TASSEL__SMCLK | MC__CONTINOUS;

  for (uint16_t i = 0; i < 16; i++) {
    uint16_t ones = 0;

    // Take a majority vote for whether new bit should be 0 or 1
    for (uint16_t j = 0; j < 5; j++) {
      while (!(CCIFG & TA0CCTL2))
        ; // Wait for interrupt

      TA0CCTL2 &= ~CCIFG; // Clear interrupt

      if (1 & TA0CCR2) // If LSb set, count it
        ones++;
    }

    result >>= 1; // Save previous bits

    if (ones >= 3) // Best out of 5
      result |= 0x8000;
  }

  TA0CCTL2 = oldcctl2;
  TA0CTL = oldclt;
  __set_interrupt_state(int_state);
  clock_src_update_all(old, new);
  return result;
}