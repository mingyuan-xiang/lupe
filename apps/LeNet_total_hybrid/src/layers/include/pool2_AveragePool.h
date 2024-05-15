#ifndef POOL2_AVERAGEPOOL_LAYER_H
#define POOL2_AVERAGEPOOL_LAYER_H

#include <libfixedAbstract/fixed.h>
#include <libmspdriver/driverlib.h>
#include <libmatAbstract/mat.h>
#include <stdint.h>
#include <libdsp/DSPLib.h>

void pool2_AveragePool(mat_t* input, mat_t* output);

#endif