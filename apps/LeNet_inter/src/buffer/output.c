#include <buffer/include/output.h>

__ro_hinv mat_t output_meta = {
	.dims = {1, 10},
	.len_dims = 2,
	.strides = {10, 10, 1},
	.data = (fixed *)output
};

__ro_hinv _q15 output[] = {
	-5575, -2035, -366, 239, -2741, -4119, -14998, 12310, -3503, -14
};