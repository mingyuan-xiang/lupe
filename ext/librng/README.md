# librng

Intermittent safe rng for msp430, inspired by [msp430-rng](https://github.com/0/msp430-rng/tree/master), [SLAA338](http://www.ti.com/sc/docs/psheets/abstract/apps/slaa338.htm), and [PCG example](https://www.pcg-random.org/).
Provides a real random (based on msp430 clocks) and a pseudorandom number generator (based on PCG).

### Random
Truly random number generation. Self contained but messes around with the clocks to get things working. Intended to be used in program startup to create a seed for the PRNG.

### Pseudorandom
A very simple and fast PRNG implemented by a PCG algorithm. The PCG code is from the [PCG example](https://www.pcg-random.org/download.html#c-implementation).


