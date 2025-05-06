#ifndef INCLUDE_MSPBASE_H
#define INCLUDE_MSPBASE_H

#include <libmsp/adc.h>
#include <libmsp/clock.h>
#include <libmsp/comp.h>
#include <libmsp/dma.h>
#include <libmsp/gpio.h>
#include <libmsp/i2c.h>
#include <libmsp/nv.h>
#include <libmsp/ref.h>
#include <libmsp/timer.h>
#include <libmsp/uart.h>
#include <libmsp/watchdog.h>
#include <msp430.h>

#if defined(__clang__)
#warning Compiler may not support some intrinsics?
#define __delay_cycles(x)
#define __bic_SR_register_on_exit(x)
#define __bis_SR_register_on_exit(x)
#endif

#endif /* INCLUDE_MSPBASE_CLOCK_H */