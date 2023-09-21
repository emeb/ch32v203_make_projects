/*
 * spi_dac.c - spi_dac driver for ch32v203.
 * 09-21-23 E. Brombaugh
 */
#ifndef __spi_dac__
#define __spi_dac__

#include "ch32v20x.h"

extern uint32_t osc_phs[2], osc_frq[2];

void spi_dac_init(void);

#endif
