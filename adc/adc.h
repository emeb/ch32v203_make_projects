/*
 * adc.h - ch32v203 ADC driver
 * E. Brombaugh 09-19-23
 */

#ifndef __adc__
#define __adc__

#include "ch32v20x.h"

#define ADC_NUMCHLS 4

void adc_init(void);
uint16_t adc_get_chl(int8_t chl);

#endif