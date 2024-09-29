#include <include/bias.h>

__ro_hinv mat_t bias_meta = {
	.dims = {1, 16},
	.len_dims = 2,
	.strides = {16, 1},
	.data = (fixed *)bias
};

__ro_hinv _q15 bias[] = {
	395,	70,	-18,	211,	250,	340,	-163,	1000,	-345,
	-806,	-16,	14,	-113,	-993,	-733,	-992
};

