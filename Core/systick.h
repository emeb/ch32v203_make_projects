/*
 * systick.h - systick IRQ and delay driver
 * 09-23-23 E. Brombaugh
 */

#ifndef __systick__
#define __systick__

#include "stdio.h"
#include "ch32v20x.h"

#define Delay_Us(n) DelaySysTick((n)*p_us)
#define Delay_Ms(n) DelaySysTick((n)*p_ms)

extern uint32_t  p_us, p_ms, systick_cnt;

void systick_init(uint32_t systick_hz);
void DelaySysTick(uint32_t n);

#endif
