#include <libmsp/clock.h>

#define F1M 1000000
#define F2_67M 2670000
#define F3_5M 3500000
#define F4M 4000000
#define F5_33M 5330000
#define F7M 7000000
#define F8M 8000000
#define F16M 16000000

#define FLFXT 32768
#define FVLO 10000
#define FMOD 4800000
#define FLFMOD (FMOD >> 7)
#define FHFXT 4000000

#define __apply_mdiv(x) (x >> (CSCTL3 & DIVM))
#define __apply_sdiv(x) (x >> ((CSCTL3 & DIVS) >> 4))

#define __mclock_freq(tmp)                                                     \
  switch (CSCTL1 & (DCOFSEL | DCORSEL)) {                                      \
  case DCOFSEL_0: /* 1 MHz */                                                  \
  case (DCOFSEL_0 | DCORSEL):                                                  \
    tmp = F1M;                                                                 \
    break;                                                                     \
  case DCOFSEL_1: /* 2.67 MHz */                                               \
    tmp = F2_67M;                                                              \
    break;                                                                     \
  case DCOFSEL_2: /* 3.5 MHz */                                                \
    tmp = F3_5M;                                                               \
    break;                                                                     \
  case DCOFSEL_3: /* 4 MHz */                                                  \
    tmp = F4M;                                                                 \
    break;                                                                     \
  case DCOFSEL_4: /* 5.33 MHz */                                               \
  case (DCOFSEL_1 | DCORSEL):                                                  \
    tmp = F5_33M;                                                              \
    break;                                                                     \
  case DCOFSEL_5: /* 7 MHz */                                                  \
  case (DCOFSEL_2 | DCORSEL):                                                  \
    tmp = F7M;                                                                 \
    break;                                                                     \
  case DCOFSEL_6: /* 8 MHz */                                                  \
  case (DCOFSEL_3 | DCORSEL):                                                  \
    tmp = F8M;                                                                 \
    break;                                                                     \
  case (DCOFSEL_4 | DCORSEL): /* 16 MHz */                                     \
    tmp = F16M;                                                                \
    break;                                                                     \
  default:                                                                     \
    tmp = 0;                                                                   \
    break;                                                                     \
  }

#define __mclock_i2c(tmp)                                                      \
  switch (CSCTL1 & (DCOFSEL | DCORSEL)) {                                      \
  case DCOFSEL_0: /* 1 MHz */                                                  \
  case (DCOFSEL_0 | DCORSEL):                                                  \
    tmp = 10;                                                                  \
    break;                                                                     \
  case DCOFSEL_1: /* 2.67 MHz */                                               \
    tmp = 27;                                                                  \
    break;                                                                     \
  case DCOFSEL_2: /* 3.5 MHz */                                                \
    tmp = 35;                                                                  \
    break;                                                                     \
  case DCOFSEL_3: /* 4 MHz */                                                  \
    tmp = 40;                                                                  \
    break;                                                                     \
  case DCOFSEL_4: /* 5.33 MHz */                                               \
  case (DCOFSEL_1 | DCORSEL):                                                  \
    tmp = 53;                                                                  \
    break;                                                                     \
  case DCOFSEL_5: /* 7 MHz */                                                  \
  case (DCOFSEL_2 | DCORSEL):                                                  \
    tmp = 7;                                                                   \
    break;                                                                     \
  case DCOFSEL_6: /* 8 MHz */                                                  \
  case (DCOFSEL_3 | DCORSEL):                                                  \
    tmp = 8;                                                                   \
    break;                                                                     \
  case (DCOFSEL_4 | DCORSEL): /* 16 MHz */                                     \
    tmp = 16;                                                                  \
    break;                                                                     \
  default:                                                                     \
    tmp = 0;                                                                   \
    break;                                                                     \
  }

uint32_t mclock_freq() {
  uint32_t f;
  __mclock_freq(f);
  return __apply_mdiv(f);
}

uint32_t sclock_freq() {
  uint32_t f = 0;
  switch (CSCTL2 & SELS) {
  case SELS__LFXTCLK:
    f = FLFXT;
    break;
  case SELS__VLOCLK:
    f = FVLO;
    break;
  case SELS__LFMODCLK:
    f = FLFMOD;
    break;
  case SELS__DCOCLK:
    __mclock_freq(f);
    break;
  case SELS__MODCLK:
    f = FMOD;
    break;
  case SELS__HFXTCLK:
    f = FHFXT;
    break;
  default:
    break;
  }
  return __apply_sdiv(f);
}

uint16_t i2c_clock_div() {
  uint16_t div = 0;

  switch (CSCTL2 & SELS) {
  case SELS__LFXTCLK:
  case SELS__VLOCLK:
  case SELS__LFMODCLK:
  case SELS__MODCLK:
  case SELS__HFXTCLK:
    return 0;
  case SELS__DCOCLK:
    __mclock_i2c(div);
    break;
  default:
    break;
  }
  return div;
}