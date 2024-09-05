#include <libdsp/DSPLib.h>
#include <libmatAbstract/mat.h>
#include <libmsp/mspbase.h>
#include <libmspdriver/driverlib.h>
#include <libmsptimer/timekeeper.h>
#include <msp430.h>
#include <libmspsyncioutils/mspsyncioutils.h>
#include <libmsppoweroff/poweroff.h>
#include <librng/rng.h>

#include <include/data.h>

#define MAGIC_NUMBER 42
#define LOWER_BOUND 100
#define UPPER_BOUND 5000
#define NUM_OF_RUNS 10
#define PSEUDO_RAND_BOUND 1000

#define ABS(x) (x > 0 ? ((uint16_t)(x)) : ((uint16_t)(-x)))

__ro_hinv uint16_t cnt[NUM_OF_RUNS];
__ro_hinv uint32_t results[NUM_OF_RUNS];

void pseudo_rand_seed() {
  uint16_t sec = 10 * __TIME__[6] + __TIME__[7] + 1;
  uint16_t min = __TIME__[4] + 1;
  uint16_t pseudo_rand = min * 60 + sec;

  for (uint16_t i = 0; i < pseudo_rand; ++i) {
    prand_range(PSEUDO_RAND_BOUND);
  }
}

void convNN_continuous(mat_t* src, mat_t* flt, mat_t* dst){
  uint32_t flt_lea_addr = (uint32_t)lea_flt;
  uint32_t flt_fram_addr = (uint32_t)(flt->data);
  uint32_t input_lea_addr = (uint32_t)lea_src;
  uint32_t input_fram_addr = (uint32_t)(src->data);
  uint32_t input_channel_addr = input_fram_addr;
  uint32_t output_lea_addr = (uint32_t)lea_dst;
  uint32_t output_fram_addr = (uint32_t)(dst->data);
  int16_t* conv_flt = lea_flt;

  msp_fill_q15_params fill_params = {
    .length = LEA_SRC_SIZE,
    .value = 0
  };
  msp_fill_q15(&fill_params, lea_src);
  fill_params.length = LEA_FLT_SIZE;
  msp_fill_q15(&fill_params, lea_flt);
  fill_params.length = LEA_DST_SIZE;
  msp_fill_q15(&fill_params, lea_dst);
  fill_params.length = LEA_TMP_SIZE;
  msp_fill_q15(&fill_params, lea_tmp);

  set_add_params(10);
  set_fir_params(14, 6);

  for (uint16_t i = 0; i < 16; ++i) {
    input_fram_addr = (uint32_t)(src->data);
    for (uint16_t j = 0; j < 6; ++j) {
      uint32_t tmp_output_addr = output_fram_addr;
      input_channel_addr = input_fram_addr;

      flt_lea_addr = flt_lea_addr + 2;
      fill_params.length = LEA_FLT_SIZE;
      msp_fill_q15(&fill_params, lea_flt);
      for (uint16_t k = 0; k < 5; ++k) {
        DMA_makeTransfer(flt_fram_addr, flt_lea_addr, 5);
        flt_lea_addr = flt_lea_addr + 12;
        flt_fram_addr = flt_fram_addr + 10;
      }
      flt_lea_addr = (uint32_t)lea_flt;
      for (uint16_t l = 0; l < 10; ++l) {
        uint32_t tmp_input_addr = input_channel_addr;
        DMA_makeTransfer(tmp_output_addr, output_lea_addr, 10);
        conv_flt = lea_flt;
        for (uint16_t k = 0; k < 5; ++k) {
          DMA_makeTransfer(tmp_input_addr, input_lea_addr, 14);
          fir_q15(conv_flt, lea_src, lea_tmp);
          add_q15(lea_dst, lea_tmp, lea_dst);
          conv_flt = conv_flt + 6;
          tmp_input_addr = tmp_input_addr + 28;
        }

        DMA_makeTransfer(output_lea_addr, tmp_output_addr, 10);
        tmp_output_addr = tmp_output_addr + 20;
        input_channel_addr = input_channel_addr + 28;
      }
      input_fram_addr = input_fram_addr + 392;
    }
    output_fram_addr = output_fram_addr + 200;
  }
}

void reset_loop_var(uint16_t i) {
  loop_var[0] = 0;
  loop_var[1] = 0;
  loop_var[3] = i;
  loop_var[4] = 0;
}

void setup_loop_var() {
  if (loop_var[2] != MAGIC_NUMBER) {
    convNN_continuous(&in_meta, &conv_meta, &out_exp_meta);
    loop_var[2] = MAGIC_NUMBER;
    reset_loop_var(0);
  }
}

uint32_t conv_check(mat_t* res, mat_t* exp) {
  uint16_t size = res->dims[0] * res->strides[0];
  uint32_t sum = 0;
  for (uint16_t i = 0; i < size; ++i) {
    int16_t diff = res->data[i] - exp->data[i];
    sum += (uint32_t)(ABS(diff));
  }
  return sum;
}

void reset_mat(mat_t* mat) {
  uint16_t size = mat->dims[0] * mat->strides[0];
  for (uint16_t i = 0; i < size; ++i) {
    mat->data[i] = 0;
  }
}

int main() {
  init();

  setup_loop_var();

  for (uint16_t i = 0; i < 10; ++i) {
    rand(1, 50);
  }

  // repeat for 1000 times
  // the convNN takes about 13147 ACLK cycles (~0.4 s)
  uint16_t i = loop_var[3];
  while (i < 1) {
    loop_var[4]++;
    uint16_t delay = rand(LOWER_BOUND, UPPER_BOUND);

    start_intermittent_tests_us(delay);
    convNN(&in_meta, &conv_meta, &out_meta);
    stop_intermittent_tests();

    results[i] = conv_check(&out_meta, &out_exp_meta);
    cnt[i] = loop_var[4] - 1;
    ++i;
    reset_loop_var(i);
    reset_mat(&out_meta);
  }

  convNN_continuous(&in_meta, &conv_meta, &out_exp_meta);
  msp_send_printf("expected differences: 0");

  for (i = 0; i < 1; ++i) {
    if (results[i] != 0) {
      msp_send_printf("=====================================================================");
      msp_send_printf("Error: Results don't match!!!");
      msp_send_printf("iteration:%u, differences: %n, restart times: %u", i, results[i], cnt[i]);
      msp_send_printf("=====================================================================");
    } else {
      msp_send_printf("iteration:%u, differences: %n, restart times: %u", i, results[i], cnt[i]);
    }
  }

  exit();
}

/* Wake up CPU once DMA is complete */
void __attribute__((interrupt(DMA_VECTOR))) dma_isr_handler(void) {
  switch (__even_in_range(DMAIV, DMAIV_DMA2IFG)) {
  case DMAIV_DMA0IFG:
  case DMAIV_DMA1IFG:
  case DMAIV_DMA2IFG:
    break;
  default:
    break;
  }
  __bic_SR_register_on_exit(LPM0_bits);
}
