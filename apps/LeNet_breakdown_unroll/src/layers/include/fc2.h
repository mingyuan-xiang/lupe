#ifndef FC2_LAYER_H
#define FC2_LAYER_H

#include <libfixedAbstract/fixed.h>
#include <libmspdriver/driverlib.h>
#include <libmatAbstract/mat.h>
#include <stdint.h>
#include <libdsp/DSPLib.h>

void fc2(mat_t* input, mat_t* output, mat_t* weight, mat_t* bias);

#endif