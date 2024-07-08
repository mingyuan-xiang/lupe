#ifndef INPUT_ARRAY_H
#define INPUT_ARRAY_H

#include <libfixedAbstract/fixed.h>
#include <libmatAbstract/mat.h>
#include <libmsp/nv.h>
#include <libdsp/DSPLib.h>

extern __ro_hinv _q15 data[];

extern __ro_hinv mat_t input_meta;
extern __ro_hinv mat_t output_meta;
extern __ro_hinv mat_t weight_meta;
extern __ro_hinv mat_t bias_meta;

#endif