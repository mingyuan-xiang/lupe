#include <layers/include/utils.h>
#include <layers/include/Flatten.h>
#include <buffer/include/buffer.h>
#include <libmspsyncioutils/mspsyncioutils.h>
#include <libmsptimer/timekeeper.h>

void Flatten(mat_t* input, mat_t* output) {
/* The data is stored in 1D array. So, we just need to feed the input to the output*/
  memcpy(output->data, input->data, MAT_GET_SIZE(input)*sizeof(uint16_t));
}