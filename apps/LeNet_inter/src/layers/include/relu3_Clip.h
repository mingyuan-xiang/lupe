#ifndef RELU3_CLIP_LAYER_H
#define RELU3_CLIP_LAYER_H

#include <libfixedAbstract/fixed.h>
#include <libmspdriver/driverlib.h>
#include <libmatAbstract/mat.h>
#include <stdint.h>
#include <libdsp/DSPLib.h>

void relu3_Clip(mat_t* input, mat_t* output);

#endif