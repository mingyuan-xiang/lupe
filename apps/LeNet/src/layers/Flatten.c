#include <layers/include/utils.h>
#include <layers/include/Flatten.h>
#include <buffer/include/buffer.h>

void Flatten(mat_t* input, mat_t* output) {
/* The data is stored in 1D array. So, we just need to feed the input to the output*/
  memcpy(output->data, input->data, MAT_GET_SIZE(input)*sizeof(uint16_t));
}