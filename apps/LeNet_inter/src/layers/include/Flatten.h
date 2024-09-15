#ifndef FLATTEN_LAYER_H
#define FLATTEN_LAYER_H

#include <libfixedAbstract/fixed.h>
#include <libmspdriver/driverlib.h>
#include <libmatAbstract/mat.h>
#include <stdint.h>
#include <libdsp/DSPLib.h>

void Flatten(mat_t* input, mat_t* output);

#endif