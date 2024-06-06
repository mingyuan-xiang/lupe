#include <layers/include/utils.h>
#include <layers/include/fc1.h>
#include <buffer/include/buffer.h>



void fc1(mat_t* input, mat_t* output, mat_t* weight, mat_t* bias) {
  uint16_t input_size = input->dims[1];
  uint16_t output_size = output->dims[1];
  uint16_t lea_line_size = LEA_SRC_SIZE;
  uint16_t lea_input_line_remain_size = input_size % lea_line_size;;
  uint32_t weight_line_offset = (weight->dims[1]) * sizeof(int16_t);

  uint32_t input_fram_addr = (uint32_t)input->data;
  uint32_t weight_fram_addr = (uint32_t)weight->data;
  uint32_t lea_line_offset = lea_line_size * sizeof(int16_t);

  msp_fill_q15_params fill_params = { .length = LEA_SRC_SIZE, .value = 0 };
  msp_mac_q15_params mac_params = { .length = MAKE_ALIGN_2(lea_line_size) };
  int16_t res;

  /* reset LEA's RAM */
  msp_fill_q15(&fill_params, lea_src);

  fill_params.length = LEA_DST_SIZE;
  msp_fill_q15(&fill_params, lea_tmp);

  /* add bias first so that we don't need to set the input to 0 */
  DMA_makeTransfer((uint32_t)(bias->data), (uint32_t)(output->data), output_size);

  /*
  * Do the remaining size MAC when the input size is not a multiple
  * of the lea buffer size
  */
  mac_params.length = MAKE_ALIGN_2(lea_input_line_remain_size);

  /* send input neurons */
  DMA_makeTransfer(input_fram_addr, (uint32_t)lea_src, lea_input_line_remain_size);

  for (uint16_t i = 0; i < output_size; ++i) {
    /* send weight row */
    DMA_makeTransfer(weight_fram_addr, (uint32_t)lea_tmp, lea_input_line_remain_size);
    msp_mac_q15(&mac_params, lea_src, lea_tmp, lea_res);

    res = (int16_t)(lea_res[0] >> 13);
    output->data[i] = __saturated_add_q15(output->data[i], res);

    weight_fram_addr += weight_line_offset;
  }
  input_fram_addr += lea_input_line_remain_size * sizeof(int16_t);

  /* Compute the rest MAC */
  for (uint16_t l = lea_input_line_remain_size; l < input_size; l += lea_line_size) {
    weight_fram_addr = (uint32_t)weight->data + (input_fram_addr - ((uint32_t)input->data));
    mac_params.length = MAKE_ALIGN_2(lea_line_size);

    /* send input neurons */
    DMA_makeTransfer(input_fram_addr, (uint32_t)lea_src, lea_line_size);

    for (uint16_t i = 0; i < output_size; ++i) {
      /* send weight row */
      DMA_makeTransfer(weight_fram_addr, (uint32_t)lea_tmp, lea_line_size);
      msp_mac_q15(&mac_params, lea_src, lea_tmp, lea_res);

      res = (int16_t)(lea_res[0] >> 13);
      output->data[i] = __saturated_add_q15(output->data[i], res);

      weight_fram_addr += weight_line_offset;
    }
    input_fram_addr += lea_line_offset;
  }
  
}