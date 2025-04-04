#include <layers/include/utils.h>
#include <layers/include/fc2.h>
#include <buffer/include/buffer.h>
#include <libmspsyncioutils/mspsyncioutils.h>
#include <libmsptimer/timekeeper.h>

static void do_mac(mat_t* weight, mat_t* input, uint32_t input_fram_addr, mat_t* output, uint16_t line_size, msp_mac_q15_params* mac_params) {
  uint32_t weight_fram_addr = (uint32_t)weight->data + (input_fram_addr - ((uint32_t)input->data));
  uint32_t weight_line_offset = (weight->dims[1]) * sizeof(int16_t);

  mac_params->length = MAKE_ALIGN_2(line_size);
  uint16_t output_size = output->dims[1];
  int16_t res;

  /* send input neurons */
  DMA_makeTransfer(input_fram_addr, (uint32_t)lea_src, line_size);

  for (uint16_t i = 0; i < output_size; ++i) {
    /* send weight row */
    DMA_makeTransfer(weight_fram_addr, (uint32_t)lea_tmp, line_size);

    msp_mac_q15(mac_params, lea_src, lea_tmp, lea_res);

    res = (int16_t)(lea_res[0] >> 14);
    output->data[i] = __saturated_add_q15(output->data[i], res);

    weight_fram_addr += weight_line_offset;
  }
}

void fc2(mat_t* input, mat_t* output, mat_t* weight, mat_t* bias) {
  uint16_t input_size = input->dims[1];
  uint16_t output_size = output->dims[1];
  uint16_t lea_line_size = LEA_SRC_SIZE;
  uint16_t lea_input_line_remain_size = input_size % lea_line_size;

  uint32_t input_fram_addr = (uint32_t)input->data;
  uint32_t lea_line_offset = lea_line_size * sizeof(int16_t);

  msp_mac_q15_params mac_params = { .length = MAKE_ALIGN_2(lea_line_size) };
  msp_fill_q15_params fill_params = { .length = LEA_SRC_SIZE, .value = 0 };

  /* reset LEA's RAM */
  msp_fill_q15(&fill_params, lea_src);

  fill_params.length = LEA_DST_SIZE;
  msp_fill_q15(&fill_params, lea_tmp);

  /*
  * Do the remaining size MAC first. Otherwise, we have to use an additional branching
  * when (`lea_line_size > input_size`), which need call `do_mac()` with input_size instead.
  */
  if (lea_input_line_remain_size) {
    uint32_t remaining_fram_addr = (uint32_t)input->data + (input_size - lea_input_line_remain_size) * sizeof(int16_t);
    do_mac(weight, input, remaining_fram_addr, output, lea_input_line_remain_size, &mac_params);
  }

  for (uint16_t l = lea_input_line_remain_size; l < input_size; l += lea_line_size) {
    do_mac(weight, input, input_fram_addr, output, lea_line_size, &mac_params);

    input_fram_addr += lea_line_offset;
  }

  /* update bias */
  for (uint16_t i = 0; i < output_size; ++i) {
    output->data[i] = __saturated_add_q15(output->data[i], bias->data[i]);
  }
}