#include <include/bias.h>

__ro_hinv mat_t bias_meta = {
	.dims = {1, 8},
	.len_dims = 2,
	.strides = {8, 1},
	.data = (fixed *)bias
};

__ro_hinv _q15 bias[] = {
	645,	491,	342,	675,	568,	665,	99,	418
};

