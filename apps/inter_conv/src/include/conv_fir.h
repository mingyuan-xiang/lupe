#ifndef CONV_FIR_LAYER_H
#define CONV_FIR_LAYER_H

#include <libfixedAbstract/fixed.h>
#include <libmspdriver/driverlib.h>
#include <libmatAbstract/mat.h>
#include <stdint.h>
#include <libdsp/DSPLib.h>

void conv_fir(mat_t* input, mat_t* output, mat_t* weight);

#endif