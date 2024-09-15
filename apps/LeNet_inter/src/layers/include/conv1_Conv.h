#ifndef CONV1_CONV_LAYER_H
#define CONV1_CONV_LAYER_H

#include <libfixedAbstract/fixed.h>
#include <libmspdriver/driverlib.h>
#include <libmatAbstract/mat.h>
#include <stdint.h>
#include <libdsp/DSPLib.h>

void conv1_Conv(mat_t* input, mat_t* output, mat_t* weight, mat_t* bias);

#endif