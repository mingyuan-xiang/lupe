#ifndef INCLUDE_MSPBASE_ADC_H
#define INCLUDE_MSPBASE_ADC_H

#include <libmsp/gpio.h>
#include <libmsp/macro_basics.h>
#include <msp430.h>

/* Sets the gpio pin to become analog input.
 * All Ax pins on MSP430fr5994 behave similarly, so this is safe.
 */
#define adc_gpio(port, pin)                                                    \
  __gpio_sel_0(port, pin);                                                     \
  __gpio_sel_1(port, pin)

#define adc_gpio_(both)                                                        \
  __gpio_sel_0(both);                                                          \
  __gpio_sel_1(both)

/* The following sets up a single ADC conversion for a given pin with the
 * following config (page 868)
 *    64 ADCCLK sample and hold time
 *    sourced by SMCLK
 *    12-bit resolution
 *    VR+ = VREF, VR- = AVSS
 *    pin can be specified via analog_pin
 *    memory location can be specified using mem (values 0-7 only)
 *
 * To expand the functinoality, the user should write the interrupt handler.
 * See the example in adc.c
 */

#define adc_setup_single_conv(analog_pin, mem)                                 \
  ADC12CTL0 &= ~ADC12ENC;                                                      \
  ADC12CTL0 |= ADC12SHT0_4;                                                    \
  ADC12CTL1 = ADC12SHP | ADC12SSEL_3;                                          \
  ADC12CTL2 = ADC12RES__12BIT;                                                 \
  ADC12CTL3 |= STICH(ADC12CSTARTADD__ADC12MEM, mem);                           \
  STICH(ADC12MCTL, mem) = ADC12VRSEL_1 | STICH(ADC12INCH_, analog_pin);        \
  ADC12IER0 |= STICH(ADC12IE, mem)

#define adc_setup_temp(mem)                                                    \
  ADC12CTL0 &= ~ADC12ENC;                                                      \
  ADC12CTL0 |= ADC12SHT0_4;                                                    \
  ADC12CTL1 = ADC12SHP | ADC12SSEL_3;                                          \
  ADC12CTL2 = ADC12RES__12BIT;                                                 \
  ADC12CTL3 |= STICH(ADC12CSTARTADD__ADC12MEM, mem) | ADC12TCMAP;              \
  STICH(ADC12MCTL, mem) = ADC12VRSEL_1 | ADC12INCH_30;                         \
  ADC12IER0 |= STICH(ADC12IE, mem)

#define adc_clean_single_conv(analog_pin, mem)                                 \
  ADC12CTL0 &= ~(ADC12ENC | ADC12ON | ADC12SHT0_8);                            \
  ADC12IER0 &= ~STICH(ADC12IE, mem);                                           \
  ADC12CTL1 &= ~(ADC12SHP | ADC12SSEL_3);                                      \
  ADC12CTL2 &= ~ADC12RES__12BIT;                                               \
  ADC12CTL3 &= ~(STICH(ADC12CSTARTADD__ADC12MEM, mem) | ADC12TCMAP);           \
  STICH(ADC12MCTL, mem) &= ~STICH(ADC12INCH_, analog_pin)

#define adc_begin()                                                            \
  ADC12CTL0 |= ADC12ON;                                                        \
  ADC12CTL0 |= ADC12ENC | ADC12SC

#define adc_busy() while (ADC12CTL1 & ADC12BUSY)

#endif /* INCLUDE_MSPBASE_ADC_H */