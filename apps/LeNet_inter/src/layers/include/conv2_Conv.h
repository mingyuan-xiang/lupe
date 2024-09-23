#ifndef CONV2_CONV_LAYER_H
#define CONV2_CONV_LAYER_H

#include <libfixedAbstract/fixed.h>
#include <libmspdriver/driverlib.h>
#include <libmatAbstract/mat.h>
#include <stdint.h>
#include <libdsp/DSPLib.h>

void conv2_Conv(mat_t* input, mat_t* input1, mat_t* output);

#endif