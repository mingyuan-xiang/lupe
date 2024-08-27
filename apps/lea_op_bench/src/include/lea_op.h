#ifndef LEA_OP_TIME_LAYER_H
#define LEA_OP_TIME_LAYER_H

#include <libfixedAbstract/fixed.h>
#include <libmspdriver/driverlib.h>
#include <libmatAbstract/mat.h>
#include <libdsp/DSPLib.h>

#define LEA_SIZE 400

extern _q15 lea_flt[];
extern _q15 lea_src[];
extern _q15 lea_tmp[];
extern _q15 lea_dst[];
extern _iq31 lea_res[];

msp_status msp_add_q15_time(const msp_add_q15_params *params, const _q15 *srcA, const _q15 *srcB, _q15 *dst, uint16_t repeat);
msp_status msp_mac_q15_time(const msp_mac_q15_params *params, const _q15 *srcA, const _q15 *srcB, _iq31 *result, uint16_t repeat);
msp_status msp_offset_q15_time(const msp_offset_q15_params *params, const _q15 *src, _q15 *dst, uint16_t repeat);
msp_status msp_fir_q15_time(const msp_fir_q15_params *params, const _q15 *src, _q15 *dst, uint16_t repeat);
msp_status msp_fill_q15_time(const msp_fill_q15_params *params, _q15 *dst, uint16_t repeat);
msp_status msp_mpy_q15_time(const msp_mpy_q15_params *params, const _q15 *srcA, const _q15 *srcB, _q15 *dst, uint16_t repeat);
msp_status msp_deinterleave_q15_time(const msp_deinterleave_q15_params *params, const _q15 *src, _q15 *dst, uint16_t repeat);
msp_status msp_matrix_mpy_q15_time(const msp_matrix_mpy_q15_params *params, const _q15 *srcA, const _q15 *srcB, _q15 *dst, uint16_t repeat);

#endif