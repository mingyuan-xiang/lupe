#ifndef FC2_GEMM_LAYER_H
#define FC2_GEMM_LAYER_H

#include <libfixedAbstract/fixed.h>
#include <libmspdriver/driverlib.h>
#include <libmatAbstract/mat.h>
#include <stdint.h>
#include <libdsp/DSPLib.h>

void fc2_Gemm(mat_t* input, mat_t* output, mat_t* weight, mat_t* bias);

#endif