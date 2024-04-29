#ifndef CONV1_ARRAY_H
#define CONV1_ARRAY_H

#include <libfixedAbstract/fixed.h>
#include <libmatAbstract/mat.h>
#include <libmsp/nv.h>
#include <libdsp/DSPLib.h>

extern __ro_hinv _q15 conv1_weight[];
extern __ro_hinv mat_t conv1_weight_meta;

extern __ro_hinv _q15 conv1_bias[];
extern __ro_hinv mat_t conv1_bias_meta;

#endif
