#ifndef CONV3_ARRAY_H
#define CONV3_ARRAY_H

#include <libfixedAbstract/fixed.h>
#include <libmatAbstract/mat.h>
#include <libmsp/nv.h>
#include <libdsp/DSPLib.h>

extern __ro_hinv _q15 conv3_weight[];
extern __ro_hinv mat_t conv3_weight_meta;

extern __ro_hinv _q15 conv3_bias[];
extern __ro_hinv mat_t conv3_bias_meta;

#endif
