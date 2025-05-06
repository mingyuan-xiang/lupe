#ifndef INCLUDE_MSPRNG_H
#define INCLUDE_MSPRNG_H

#include <libmsp/nv.h>
#include <msp430.h>
#include <stdint.h>
#include <string.h>

#define BASIC_SEED 0x20dfU

extern uint16_t pcg_state;
uint16_t clock_rand();

/*
 * minimal PCG16 code / (c) 2014 M.E. O'Neill / pcg-random.org
 *  Licensed under Apache License 2.0 (NO WARRANTY, etc. see website)
 *
 *  NOTE: the MSP430 only has instructions for single left or right shifts.
 *  Hence, shifting a value right by 13 bits takes 13 instructions. This
 *  makes the PCG performance terrible compared to a simple LCG. Use
 *  the PCG if performance isn't the biggest issue
 *  Approx 17 cycles for LCG version
 */
static inline uint16_t prand() {
  // MPY32
  MPY32CTL0 = MPYDLYWRTEN | MPYDLY32_0;
  MPY = CONFIG_PCG_MULT;

#ifdef CONFIG_RNG_PCG
  // store old state
  uint16_t oldstate = pcg_state;

  // Advance internal state
  OP2 = pcg_state;
  pcg_state = RESLO + CONFIG_PCG_INCR;

  // Calculate output function (XSH RR), uses old state for max ILP
  uint16_t word = ((oldstate >> ((oldstate >> 13u) + 3u)) ^ oldstate) * 62169u;
  return (word >> 11u) ^ word;
#else
  OP2 = pcg_state;
  pcg_state = RESLO + CONFIG_PCG_INCR;

  return pcg_state;
#endif
}

/* Produces a random number in the given range
 * Approx adds 10 cycles to how many long prand takes
 */
static inline uint16_t prand_range(uint16_t range) {
  uint16_t x = prand();
  MPY = range;
  OP2 = x;
  return RESHI;
}

/* Randomly shuffles an array */
void shuffle_array(uint16_t *ptr, size_t length);

/* Seeds the psuedo rng based on clock randomness */
static inline void pseed() { pcg_state = clock_rand(); }

#endif /* INCLUDE_MSPRNG_H */