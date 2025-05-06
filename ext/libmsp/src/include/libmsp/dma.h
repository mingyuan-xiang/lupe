#ifndef INCLUDE_LIBMSP_DMA
#define INCLUDE_LIBMSP_DMA
#include <libmsp/macro_basics.h>
#include <msp430.h>

// The compiler turns the normal data16_write_addr into something ugly, so we
// fix it here.
#define _gdata16_write_addr(addr, src)                                         \
  ({                                                                           \
    uintptr_t __src = src;                                                     \
    __asm__ __volatile("movx.a %1, %0\n\t" : "=m"(addr) : "r"(__src));         \
  })

#define __gdata16_write_addr(addr, src) _gdata16_write_addr(addr, src)

// DMA Single Block transfer, word to word
// Block mode, both addresses incremented, word to word, enable interrupt
/* __data16_write_addr(&(uintptr_t)(STIC3(DMA, CHANNEL, SA)), SRCADDR);        \
__data16_write_addr(&(uintptr_t)(STIC3(DMA, CHANNEL, DA)), DSTADDR);        \ */
#define DMA_WW(CHANNEL, SRCADDR, DSTADDR, SIZE)                                \
  {                                                                            \
    STIC3(DMA, CHANNEL, CTL) = DMADT_1 | DMADSTINCR_3 | DMASRCINCR_3 |         \
                               DMADSTBYTE__WORD | DMASRCBYTE__WORD | DMAIE;    \
    __gdata16_write_addr((STIC3(DMA, CHANNEL, SA)), (uintptr_t)SRCADDR);       \
    __gdata16_write_addr((STIC3(DMA, CHANNEL, DA)), (uintptr_t)DSTADDR);       \
    STIC3(DMA, CHANNEL, SZ) = SIZE;                                            \
    STIC3(DMA, CHANNEL, CTL) |= DMAEN | DMAREQ;                                \
  }

#define DMA_SETW(CHANNEL, SRCADDR, DSTADDR, SIZE)                              \
  {                                                                            \
    STIC3(DMA, CHANNEL, CTL) = DMADT_1 | DMADSTINCR_3 | DMASRCINCR_0 |         \
                               DMADSTBYTE__WORD | DMASRCBYTE__WORD | DMAIE;    \
    __gdata16_write_addr((STIC3(DMA, CHANNEL, SA)), (uintptr_t)SRCADDR);       \
    __gdata16_write_addr((STIC3(DMA, CHANNEL, DA)), (uintptr_t)DSTADDR);       \
    STIC3(DMA, CHANNEL, SZ) = SIZE;                                            \
    STIC3(DMA, CHANNEL, CTL) |= DMAEN | DMAREQ;                                \
  }

// DMA transfer, word to byte
// DMA transfer, byte to word

#endif // INCLUDE_LIBMSP_DMA