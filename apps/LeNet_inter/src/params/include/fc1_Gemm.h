#ifndef FC1_GEMM_ARRAY_H
#define FC1_GEMM_ARRAY_H

#include <libfixedAbstract/fixed.h>
#include <libmatAbstract/mat.h>
#include <libmsp/nv.h>
#include <libdsp/DSPLib.h>

extern __ro_hinv _q15 fc1_weight[];
extern __ro_hinv mat_t fc1_weight_meta;

extern __ro_hinv _q15 fc1_bias[];
extern __ro_hinv mat_t fc1_bias_meta;

#endif
