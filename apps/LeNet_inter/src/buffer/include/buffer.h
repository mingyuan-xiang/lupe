#ifndef BUFFER_ARRAY_H
#define BUFFER_ARRAY_H

#include <libfixedAbstract/fixed.h>
#include <libmatAbstract/mat.h>
#include <libmsp/nv.h>
#include <libdsp/DSPLib.h>

extern __ro_hinv mat_t conv1_Conv_in_meta;
extern __ro_hinv mat_t conv1_Conv_out_meta;

extern __ro_hinv mat_t relu1_Clip_in_meta;
extern __ro_hinv mat_t relu1_Clip_out_meta;

extern __ro_hinv mat_t pool1_AveragePool_in_meta;
extern __ro_hinv mat_t pool1_AveragePool_out_meta;

extern __ro_hinv mat_t conv2_Conv_in_meta;
extern __ro_hinv mat_t conv2_Conv_out_meta;

extern __ro_hinv mat_t relu2_Clip_in_meta;
extern __ro_hinv mat_t relu2_Clip_out_meta;

extern __ro_hinv mat_t pool2_AveragePool_in_meta;
extern __ro_hinv mat_t pool2_AveragePool_out_meta;

extern __ro_hinv mat_t conv3_Conv_in_meta;
extern __ro_hinv mat_t conv3_Conv_out_meta;

extern __ro_hinv mat_t relu3_Clip_in_meta;
extern __ro_hinv mat_t relu3_Clip_out_meta;

extern __ro_hinv mat_t Flatten_in_meta;
extern __ro_hinv mat_t Flatten_out_meta;

extern __ro_hinv mat_t fc1_Gemm_in_meta;
extern __ro_hinv mat_t fc1_Gemm_out_meta;

extern __ro_hinv mat_t relu4_Clip_in_meta;
extern __ro_hinv mat_t relu4_Clip_out_meta;

extern __ro_hinv mat_t fc2_Gemm_in_meta;
extern __ro_hinv mat_t fc2_Gemm_out_meta;

extern __ro_hinv _q15 in_buffer[];
extern __ro_hinv mat_t in_buffer_meta;

extern __ro_hinv _q15 out_buffer[];
extern __ro_hinv mat_t out_buffer_meta;

#endif
