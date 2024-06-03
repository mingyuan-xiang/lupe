#ifndef CLIP_6_LAYER_H
#define CLIP_6_LAYER_H

#include <libfixedAbstract/fixed.h>
#include <libmspdriver/driverlib.h>
#include <libmatAbstract/mat.h>
#include <stdint.h>
#include <libdsp/DSPLib.h>

void Clip_6(mat_t* input, mat_t* output);

#endif