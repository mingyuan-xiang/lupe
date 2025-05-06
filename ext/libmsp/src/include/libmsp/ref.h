#ifndef INCLUDE_MSPBASE_REF_H
#define INCLUDE_MSPBASE_REF_H

#include <libmsp/macro_basics.h>
#include <msp430.h>

/* This header assumes that several macros are defined in advance, before its
 * inclusion. If such values are not defined, you will may errors. Full
 * description are in page 862 of the manual for MSPFR5994.
 *
 * The valid references are as follows,
 *    2.5 V : requires VCC 2.7+ V
 *    2.0 V : requires VCC 2.2+ V
 *    1.2 V : requires VCC 1.8+ V
 */

#define REFV__12 REFVSEL_0
#define REFV__20 REFVSEL_1
#define REFV__25 REFVSEL_2

#define ref_ready() while (!(REFCTL0 & REFGENRDY))

#define ref_busy() while (REFCTL0 & REFGENBUSY)

/* given voltage should be 12, 20, or 25 */
#define ref_start(voltage) REFCTL0 |= STICH(REFV__, voltage) | REFTCOFF | REFON

#define ref_start_temp(voltage) REFCTL0 |= STICH(REFV__, voltage) | REFON

#define ref_stop() REFCTL0 &= ~(REFVSEL_3 | REFON)

#endif /* INCLUDE_MSPBASE_REF_H */