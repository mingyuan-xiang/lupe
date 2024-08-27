#ifndef MAC_CONV_TIME_LAYER_H
#define MAC_CONV_TIME_LAYER_H

#include <libfixedAbstract/fixed.h>
#include <libmspdriver/driverlib.h>
#include <libmatAbstract/mat.h>
#include <stdint.h>
#include <libdsp/DSPLib.h>

void mac_conv_time(mat_t* input, mat_t* output, mat_t* weight, mat_t* bias, uint16_t repeat, uint32_t* t);

#endif