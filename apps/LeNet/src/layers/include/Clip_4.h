#ifndef CLIP_4_LAYER_H
#define CLIP_4_LAYER_H

#include <libfixedAbstract/fixed.h>
#include <libmspdriver/driverlib.h>
#include <libmatAbstract/mat.h>
#include <stdint.h>
#include <libdsp/DSPLib.h>

void Clip_4(mat_t* input, mat_t* output);

#endif