#include <libmsptimer/time_clock.h>
#include <libmsp/mspbase.h>

typedef struct {
  time_t deadline;
  bool waiting;
} deadline_t;

// These globals are non volatile because only atomic functions can use it.
// TODO: support non-atomic functions waiting
time_t current_time = {
    .seconds = 0,
    .ticks = 0,
};

static deadline_t app_deadline = {
    {.seconds = 0, .ticks = 0},
    .waiting = false,
};

static deadline_t kernel_deadline = {
    {.seconds = 0, .ticks = 0},
    .waiting = false,
};

/* ---- Time Struct Functions --- */

#define sanitize_time_t(a)                                                     \
  {                                                                            \
    a.seconds += a.ticks >> TICKS_SEC_SHIFT;                                   \
    a.ticks &= (TICKS_PER_SEC - 1);                                            \
  }

bool happens_after_time(time_t a, time_t b) {
  sanitize_time_t(a);
  sanitize_time_t(b);
  if (a.seconds == b.seconds) {
    return a.ticks > b.ticks;
  } else {
    return a.seconds > b.seconds;
  }
}

time_t add_times(time_t a, time_t b) {
  sanitize_time_t(a);
  sanitize_time_t(b);
  time_t ret = {0, 0};
  ret.ticks = a.ticks + b.ticks;
  ret.seconds = a.seconds + b.seconds;
  sanitize_time_t(ret);
  return ret;
}

time_t subtract_times(time_t second, time_t first) {
  if (happens_after_time(first, second)) {
    time_t ret = {0, 0};
    return ret;
  } else {
    sanitize_time_t(second);
    sanitize_time_t(first);

    if (second.ticks < first.ticks) {
      // safe to do since we know a is bigger than b
      second.ticks += TICKS_PER_SEC;
      second.seconds -= 1;
    }
    time_t ret = {0, 0};
    ret.seconds = second.seconds - first.seconds;
    ret.ticks = second.ticks - first.ticks;
    return ret;
  }
}

/* ---- Timer Updates --- */

#define will_occur_before_next_increment(d)                                    \
  (d.waiting && d.deadline.seconds < (current_time.seconds + SECOND_INCREMENT))

#define ticks_till_deadline(d)                                                 \
  ((d.deadline.seconds - current_time.seconds) * TICKS_PER_SEC +               \
   d.deadline.ticks)

static inline __attribute__((always_inline)) void timer_update_deadlines() {
  if (will_occur_before_next_increment(app_deadline)) {
    uint16_t ticks_to_wait = ticks_till_deadline(app_deadline);
    if (ticks_to_wait == 0) {
      // In case the deadline has zero leftover ticks
      STIC3(TA, CLOCK_TICK_TIMER, CCTL1) = CCIE | CCIFG;
    } else {
      STIC3(TA, CLOCK_TICK_TIMER, CCR1) = ticks_to_wait;
      STIC3(TA, CLOCK_TICK_TIMER, CCTL1) = CCIE;
    }
  }

  if (will_occur_before_next_increment(kernel_deadline)) {
    uint16_t ticks_to_wait = ticks_till_deadline(kernel_deadline);
    if (ticks_to_wait == 0) {
      // In case the deadline has zero leftover ticks
      STIC3(TA, CLOCK_TICK_TIMER, CCTL2) = CCIE | CCIFG;
    } else {
      STIC3(TA, CLOCK_TICK_TIMER, CCR2) = ticks_to_wait;
      STIC3(TA, CLOCK_TICK_TIMER, CCTL2) = CCIE;
    }
  }
}

static inline __attribute__((always_inline)) void timer_update_seconds() {
  current_time.seconds += SECOND_INCREMENT;
}

/* --- Time Management --- */

time_t get_current_time() {
  // NOTE: There is a data race that can occur, where by a time in the past or
  // future is returned.
  //       1. The TAxR has rolled over, but the interrupt has not been issued
  //       yet.
  //          1.S In this case, we wrap the operation in interrupt pervention,
  //              check if the roll over has happend without interrupt occurance
  //              and do the interrupt operation manually.
  //       2. The TAxR rolls over exactly when we check for 1.S and the 1.S
  //       condition returns true. If the conditions returns false but the TAxR
  //       rolls over before the function ends, we are still fine.
  //          2.S Since the difference between the 1.S check and the TAxR read
  //          is only a few instructions, we can assume that at most one roll
  //          over has happend.
  //          In the case of a roll over, if we check the TAxR inside the 1.S
  //          loop. We keep whichever value that is lower.
  uint16_t interrupt_state = __get_interrupt_state();
  __disable_interrupt();

  current_time.ticks = STIC3(TA, CLOCK_TICK_TIMER, R);
  // Check for 1.S
  if (STIC3(TA, CLOCK_TICK_TIMER, CTL) & TAIFG) {
    timer_update_seconds();
    STIC3(TA, CLOCK_TICK_TIMER, CTL) |= ~TAIFG;

    // 2.S
    uint16_t new_ticks = STIC3(TA, CLOCK_TICK_TIMER, R);
    current_time.ticks =
        (new_ticks < current_time.ticks) ? new_ticks : current_time.ticks;
  }

  __set_interrupt_state(interrupt_state);
  return current_time;
}

bool set_kernel_wakeup(time_t deadline) {
  time_t current = get_current_time();
  if (happens_after_time(current, deadline)) {
    // skip if deadline is already past
    return false;
  }

  kernel_deadline.deadline = deadline;
  kernel_deadline.waiting = true;
  timer_update_deadlines();
  return true;
}

void sleep_till(time_t deadline) {
  time_t current = get_current_time();
  if (happens_after_time(current, deadline)) {
    // skip if deadline is already past
    return;
  }

  __disable_interrupt();
  app_deadline.deadline = deadline;
  app_deadline.waiting = true;
  timer_update_deadlines();
  __bis_SR_register(LPM3_bits + GIE);
}

/* --- Clock & Timer Initilizations --- */

static inline __attribute__((always_inline)) void timers_start() {
  // stop timer (in case it was running)
  timer_halt(CLOCK_TICK_TIMER);
  timer_IFG_disable(CLOCK_TICK_TIMER);

  current_time.seconds = 0;
  current_time.ticks = 0;
  timer_reset(CLOCK_TICK_TIMER);

  STIC3(TA, CLOCK_TICK_TIMER, EX0) = TAIDEX__1;
  // ccr0 w/ period TICKS_PER_SEC
  STIC3(TA, CLOCK_TICK_TIMER, CCR0) = TICKS_PER_SEC - 1;
  // Use Aux clock, div 4, interrupts enabled in up mode and reset the counter
  // as well
  STIC3(TA, CLOCK_TICK_TIMER, CTL) =
      TASSEL__ACLK | TIMERDIV_4 | TAIE | MC__UP | TACLR;
}

void time_clock_init(bool wait_for_lfxt) {
  // Setup FRAM & LFXTCLK pins
  __disable_interrupt();
  FRCTL0 = FRCTLPW | STICH(FRAMST__, DCOFREQ);
  PJSEL0 |= BIT4 | BIT5;
  CSCTL0_H = CSKEY_H;
  // First turn on the LFXTCLK
  CSCTL4 = HFXTOFF;
  CSCTL1 = DCOFSEL_0;
  CSCTL3 = DIVA__4 | DIVS__4 | DIVM__32;
  CSCTL2 = SELA__LFXTCLK | STICH(SELM__, MCLK_SRC) | STICH(SELS__, SMCLK_SRC);
  do {
    CSCTL5 &= ~LFXTOFFG;
    SFRIFG1 &= ~OFIFG;
  } while (wait_for_lfxt && (SFRIFG1 & OFIFG));
  /* Fix for CS12 per errata */
  CSCTL3 = DIVA__4 | DIVS__4 | DIVM__4;
  CSCTL1 = STICH(DCOFREQ__, DCOFREQ); /* Per errata have to wait 10us, max speed
                                         of 16MHz requires 60 cycles*/
  __delay_cycles(ERRATA_WAIT);
  CSCTL3 = STICH(DIVA__, ACLK_DIV) | STICH(DIVM__, MCLK_DIV) |
           STICH(DIVS__, SMCLK_DIV);
  CSCTL0_H = 0;
  timers_start();
}

/* --- Interrupts --- */

__attribute__((interrupt(STIC3(TIMER, CLOCK_TICK_TIMER, _A1_VECTOR)))) void
STIC3(TIMER, CLOCK_TICK_TIMER, _A1_ISR)(void) {
  switch (__even_in_range(STIC3(TA, CLOCK_TICK_TIMER, IV), TAIV__TAIFG)) {
    {
    case TAIV__NONE:
      break; // No interrupt
    case TAIV__TACCR1:
      app_deadline.waiting = false;
      STIC3(TA, CLOCK_TICK_TIMER, CCR1) = 0;
      STIC3(TA, CLOCK_TICK_TIMER, CCTL1) = ~CCIE;
      LPM3_EXIT; // done waiting, wake up
      break;
    case TAIV__TACCR2:
      kernel_deadline.waiting = false;
      STIC3(TA, CLOCK_TICK_TIMER, CCR2) = 0;
      STIC3(TA, CLOCK_TICK_TIMER, CCTL2) = ~CCIE;
      // TODO: What to do if kernel is supposed to wake up?
      break;
    case TAIV__TACCR3:
      break; // reserve d
    case TAIV__TACCR4:
      break; // reserved
    case TAIV__TACCR5:
      break; // reserved
    case TAIV__TACCR6:
      break;          // reserved
    case TAIV__TAIFG: // overflow
      timer_update_seconds();
      timer_update_deadlines();
      break;
    default:
      break;
    }
  }
}