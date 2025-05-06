#ifndef INCLUDE_MSPBASE_WD_H
#define INCLUDE_MSPBASE_WD_H
#include <msp430.h>

#define watchdog_disable() WDTCTL = WDTPW | WDTHOLD

#endif /* INCLUDE_MSPBASE_WD_H */