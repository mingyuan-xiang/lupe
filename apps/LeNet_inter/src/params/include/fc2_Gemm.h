#ifndef FC2_GEMM_ARRAY_H
#define FC2_GEMM_ARRAY_H

#include <libfixedAbstract/fixed.h>
#include <libmatAbstract/mat.h>
#include <libmsp/nv.h>
#include <libdsp/DSPLib.h>

extern __ro_hinv _q15 fc2_weight[];
extern __ro_hinv mat_t fc2_weight_meta;

extern __ro_hinv _q15 fc2_bias[];
extern __ro_hinv mat_t fc2_bias_meta;

#endif
