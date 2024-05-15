#ifndef CONV2_ARRAY_H
#define CONV2_ARRAY_H

#include <libfixedAbstract/fixed.h>
#include <libmatAbstract/mat.h>
#include <libmsp/nv.h>
#include <libdsp/DSPLib.h>

extern __ro_hinv _q15 conv2_weight[];
extern __ro_hinv mat_t conv2_weight_meta;

extern __ro_hinv _q15 conv2_bias[];
extern __ro_hinv mat_t conv2_bias_meta;

#endif
