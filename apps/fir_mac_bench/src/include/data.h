#ifndef DATA_ARRAY_H
#define DATA_ARRAY_H

#include <libfixedAbstract/fixed.h>
#include <libmatAbstract/mat.h>
#include <libmsp/nv.h>
#include <libdsp/DSPLib.h>

extern __ro_hinv mat_t weight_0_meta;
extern __ro_hinv mat_t bias_0_meta;
extern __ro_hinv mat_t input_0_meta;
extern __ro_hinv mat_t output_0_meta;
extern __ro_hinv mat_t weight_1_meta;
extern __ro_hinv mat_t bias_1_meta;
extern __ro_hinv mat_t input_1_meta;
extern __ro_hinv mat_t output_1_meta;
extern __ro_hinv _q15 data[];
extern __ro_hinv mat_t data_meta;
#endif
