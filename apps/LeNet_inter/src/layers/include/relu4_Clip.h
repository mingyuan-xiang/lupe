#ifndef RELU4_CLIP_LAYER_H
#define RELU4_CLIP_LAYER_H

#include <libfixedAbstract/fixed.h>
#include <libmspdriver/driverlib.h>
#include <libmatAbstract/mat.h>
#include <stdint.h>
#include <libdsp/DSPLib.h>

void relu4_Clip(mat_t* input, mat_t* output);

#endif