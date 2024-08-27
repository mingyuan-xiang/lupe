#ifndef CPU_CONV_LAYER_H
#define CPU_CONV_LAYER_H

#include <libfixedAbstract/fixed.h>
#include <libmspdriver/driverlib.h>
#include <libmatAbstract/mat.h>
#include <stdint.h>
#include <libdsp/DSPLib.h>

void cpu_conv(mat_t* input, mat_t* output, mat_t* weight, mat_t* bias);

#endif