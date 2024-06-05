#include <layers/include/utils.h>
#include <layers/include/fc2.h>
#include <buffer/include/buffer.h>


static inline void new_mac_q15(const _q15 *srcA, const _q15 *srcB) {
  lea_mac_params->input2 = MSP_LEA_CONVERT_ADDRESS(srcB);

  /* Load source arguments to LEA. */
  LEAPMS0 = MSP_LEA_CONVERT_ADDRESS(srcA);
  LEAPMS1 = MSP_LEA_CONVERT_ADDRESS(lea_mac_params);

  /* Invoke the LEACMD__MAC command. */
  msp_lea_invokeCommand(LEACMD__MAC);
}

static void do_mac(mat_t* weight, mat_t* input, uint32_t input_fram_addr, mat_t* output, uint16_t line_size) {
  uint32_t weight_fram_addr = (uint32_t)weight->data + (input_fram_addr - ((uint32_t)input->data));
  uint32_t weight_line_offset = (weight->dims[1]) * sizeof(int16_t);
  uint16_t output_size = output->dims[1];
  int16_t res;

  /* send input neurons */
  DMA_makeTransfer(input_fram_addr, (uint32_t)lea_src, line_size);

  for (uint16_t i = 0; i < output_size; ++i) {
    /* send weight row */
    DMA_makeTransfer(weight_fram_addr, (uint32_t)lea_tmp, line_size);
    new_mac_q15(lea_src, lea_tmp);

    res = (int16_t)(lea_res[0] >> 13);
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

  msp_fill_q15_params fill_params = { .length = LEA_SRC_SIZE, .value = 0 };
  mac_init(MAKE_ALIGN_2(lea_input_line_remain_size));

  /* reset LEA's RAM */
  msp_fill_q15(&fill_params, lea_src);

  fill_params.length = LEA_DST_SIZE;
  msp_fill_q15(&fill_params, lea_tmp);

  /* add bias first so that we don't need to set the input to 0 */
  for (uint16_t i = 0; i < output_size; ++i) {
    DMA_makeTransfer((uint32_t)(bias->data), (uint32_t)(output->data), output_size);
  }

  /*
  * Do the remaining size MAC first. Otherwise, we have to use an additional branching
  * when (`lea_line_size > input_size`), which need call `do_mac()` with input_size instead.
  */
  if (lea_input_line_remain_size) {
    uint32_t remaining_fram_addr = (uint32_t)input->data + (input_size - lea_input_line_remain_size) * sizeof(int16_t);
    do_mac(weight, input, remaining_fram_addr, output, lea_input_line_remain_size);
  }
  lea_mac_params->vectorSize = MAKE_ALIGN_2(lea_line_size);
  for (uint16_t l = lea_input_line_remain_size; l < input_size; l += lea_line_size) {
    do_mac(weight, input, input_fram_addr, output, lea_line_size);

    input_fram_addr += lea_line_offset;
  }
  
  mac_clear();
}