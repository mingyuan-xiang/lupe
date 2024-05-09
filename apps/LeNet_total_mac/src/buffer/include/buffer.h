#ifndef BUFFER_ARRAY_H
#define BUFFER_ARRAY_H

#include <libfixedAbstract/fixed.h>
#include <libmatAbstract/mat.h>
#include <libmsp/nv.h>
#include <libdsp/DSPLib.h>

extern __ro_hinv _q15 conv1_in[];
extern __ro_hinv mat_t conv1_in_meta;
extern __ro_hinv _q15 conv1_out[];
extern __ro_hinv mat_t conv1_out_meta;

extern __ro_hinv _q15 Relu_in[];
extern __ro_hinv mat_t Relu_in_meta;
extern __ro_hinv _q15 Relu_out[];
extern __ro_hinv mat_t Relu_out_meta;

extern __ro_hinv _q15 pool1_AveragePool_in[];
extern __ro_hinv mat_t pool1_AveragePool_in_meta;
extern __ro_hinv _q15 pool1_AveragePool_out[];
extern __ro_hinv mat_t pool1_AveragePool_out_meta;

extern __ro_hinv _q15 conv2_in[];
extern __ro_hinv mat_t conv2_in_meta;
extern __ro_hinv _q15 conv2_out[];
extern __ro_hinv mat_t conv2_out_meta;

extern __ro_hinv _q15 Relu_1_in[];
extern __ro_hinv mat_t Relu_1_in_meta;
extern __ro_hinv _q15 Relu_1_out[];
extern __ro_hinv mat_t Relu_1_out_meta;

extern __ro_hinv _q15 pool2_AveragePool_in[];
extern __ro_hinv mat_t pool2_AveragePool_in_meta;
extern __ro_hinv _q15 pool2_AveragePool_out[];
extern __ro_hinv mat_t pool2_AveragePool_out_meta;

extern __ro_hinv _q15 conv3_in[];
extern __ro_hinv mat_t conv3_in_meta;
extern __ro_hinv _q15 conv3_out[];
extern __ro_hinv mat_t conv3_out_meta;

extern __ro_hinv _q15 Relu_2_in[];
extern __ro_hinv mat_t Relu_2_in_meta;
extern __ro_hinv _q15 Relu_2_out[];
extern __ro_hinv mat_t Relu_2_out_meta;

extern __ro_hinv _q15 Flatten_in[];
extern __ro_hinv mat_t Flatten_in_meta;
extern __ro_hinv _q15 Flatten_out[];
extern __ro_hinv mat_t Flatten_out_meta;

extern __ro_hinv _q15 fc1_in[];
extern __ro_hinv mat_t fc1_in_meta;
extern __ro_hinv _q15 fc1_out[];
extern __ro_hinv mat_t fc1_out_meta;

extern __ro_hinv _q15 Relu_3_in[];
extern __ro_hinv mat_t Relu_3_in_meta;
extern __ro_hinv _q15 Relu_3_out[];
extern __ro_hinv mat_t Relu_3_out_meta;

extern __ro_hinv _q15 fc2_in[];
extern __ro_hinv mat_t fc2_in_meta;
extern __ro_hinv _q15 fc2_out[];
extern __ro_hinv mat_t fc2_out_meta;

extern __ro_hinv _q15 in_buffer[];
extern __ro_hinv mat_t in_buffer_meta;

extern __ro_hinv _q15 out_buffer[];
extern __ro_hinv mat_t out_buffer_meta;

#endif
