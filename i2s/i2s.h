/*
 * i2s.c - i2s driver for ch32v203. NOTE - only available on C8 / R8 packages!
 * 09-20-23 E. Brombaugh
 */
#ifndef __i2s__
#define __i2s__

#include "ch32v20x.h"

extern uint32_t osc_phs[2], osc_frq[2];

void i2s_init(void);

#endif
