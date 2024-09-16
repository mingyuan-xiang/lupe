#ifndef INTERMITTENT_H
#define INTERMITTENT_H

#define COMPUTE_CK 0
#define CONV_PAD 1
#define CONV_IN_CH 2
#define CONV_OUT_CN 3
#define CONV_IO_ROW 4
#define CONV_K_ROW 5
#define BUFFER_COMMIT 6

#define DOUBLE_BUFFER_COMPLETE 0 // Complete double buffering. No need for recovery
#define DOUBLE_BUFFER_TMP 1 // Start transferring to tmp buffer
#define DOUBLE_BUFFER_FINAL 2 // Start transferring to output buffer

#define COMPUTE_EXIT 0
#define COMPUTE_MAIN 1
#define COMPUTE_BIAS 2

#endif