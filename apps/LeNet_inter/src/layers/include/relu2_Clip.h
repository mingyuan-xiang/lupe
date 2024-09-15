#ifndef RELU2_CLIP_LAYER_H
#define RELU2_CLIP_LAYER_H

#include <libfixedAbstract/fixed.h>
#include <libmspdriver/driverlib.h>
#include <libmatAbstract/mat.h>
#include <stdint.h>
#include <libdsp/DSPLib.h>

void relu2_Clip(mat_t* input, mat_t* output);

#endif