#include <libmsp/mspbase.h>
#include <libmspdriver/driverlib.h>
#include <libmspsyncioutils/mspsyncioutils.h>
#include <msp430.h>

#include <include/input.h>
#include <include/image.h>
#include <include/output.h>
#include <include/output_exp.h>
#include <include/weight.h>
#include <include/bias.h>
#include <include/intermittent.h>
#include <include/utils.h>
#include <include/conv.h>
#include <include/conv_exp.h>
#include <libmsppoweroff/poweroff.h>
#include <librng/rng.h>

/* ACLK cycles (32768 Hz) */
#define DELAY 50

#define REPEAT 10

void init() {
  watchdog_disable();
  gpio_init_all_ports();
  gpio_unlock();
  gpio_clear_interrupts();
  clock_init();
  uartio_open(0);
  init_lupe();
}

void exit() {
  msp_end_printing();
}

uint16_t verify() {
  return memcmp(output_meta.data, output_exp_meta.data, output_meta.strides[0]*sizeof(int16_t));
}

uint16_t rand(uint16_t low, uint16_t high) {
  return prand_range(high - low) + low;
}

int main() {
  init();

  uint16_t c = intermittent_status[COUNTER] + 1;
  VOLATILE_WRITE(c, COUNTER);

  for (uint16_t i = intermittent_status[MAIN_LOOP]; i < REPEAT; ++i) {
    if (intermittent_status[COMPUTE_CK] == INTERMITTENT_MobileNetV2_inter_START) {
      DMA_makeTransfer((uintptr_t)(image_meta.data), (uintptr_t)(input_meta.data), image_meta.strides[0]);
      intermittent_status[COMPUTE_CK] = INTERMITTENT_features_features_1_pw_conv_linear_pw_conv_linear_0_Conv_PREPARE;
    }

    start_intermittent_tests(0, DELAY);
    conv(&input_meta, &output_meta, &weight_meta, &bias_meta);
    stop_intermittent_tests();

    DMA_makeTransfer((uintptr_t)(image_meta.data), (uintptr_t)(input_meta.data), image_meta.strides[0]);
    conv_exp(&input_meta, &output_exp_meta, &weight_meta, &bias_meta);

    intermittent_status[COMPUTE_CK] = INTERMITTENT_MobileNetV2_inter_START;

    if (verify() != 0) {
      break;
    } else {
      memset(log, 0, sizeof(int16_t)*LOG_SIZE);
    }

    uint16_t next_i = i + 1;
    VOLATILE_WRITE(next_i, MAIN_LOOP);
  }

  msp_send_printf("Restart times: %u (repeat: %u)", intermittent_status[COUNTER], REPEAT);
  msp_send_printf("output_meta.strides[0]: %u", output_meta.strides[0]);

  if (verify() != 0) {
    uint16_t cnt = 0;
    for (uint16_t i = 0; i < output_meta.dims[1]; ++i) {
      for (uint16_t j = 0; j < output_meta.dims[2]; ++j) {
        for (uint16_t k = 0; k < output_meta.dims[3]; ++k) {
          if (output_meta.data[cnt] != output_exp_meta.data[cnt]) {
            msp_send_printf(
              "(1, %u, %u, %u): got: %i, expected: %i", i, j, k,
              output_meta.data[cnt], output_exp_meta.data[cnt]
            );
          }
          cnt++;
        }
      }
    }

    for (int16_t i = 1; i < log[0]; i += 3) {
      msp_send_printf(
        "COMPUTE_IO_COL: %i COMPUTE_IO_ROW: %i, COMPUTE_IN_CH: %i",
        log[i], log[i+1], log[i+2]
      );
    }

    // msp_send_printf("Got activations for the last layer:");
    // msp_send_mat(&output_meta);
    // msp_send_printf("Expected:");
    // msp_send_mat(&output_exp_meta);
  }

  exit();
}