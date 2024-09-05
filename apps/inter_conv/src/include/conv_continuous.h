#ifndef CONV_CONTINUOUS_LAYER_H
#define CONV_CONTINUOUS_LAYER_H

#include <libfixedAbstract/fixed.h>
#include <libmspdriver/driverlib.h>
#include <libmatAbstract/mat.h>
#include <stdint.h>
#include <libdsp/DSPLib.h>

void conv_continuous(mat_t* input, mat_t* output, mat_t* weight);

#endif