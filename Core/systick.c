/*
 * systick.c - systick IRQ and delay driver
 * 09-23-23 E. Brombaugh
 */

#include "systick.h"

uint32_t  p_us = 0, p_ms = 0, systick_inc = 0, systick_cnt = 0;

/*
 * set up the delay constants, start counter and optionally start the IRQ
 */
void systick_init(uint32_t systick_hz)
{
	/* set up timing constants */
    p_us = SystemCoreClock / 8000000;
    p_ms = (uint16_t)p_us * 1000;
	
	/* start up the Systick counter */
	SysTick->CTLR = 1;
	
	/* configure systick IRQ if Hz rate defined */
	if(systick_hz)
	{
		systick_inc = SystemCoreClock / (8 * systick_hz);
		systick_cnt = 0;
		
		/* enable the SysTick IRQ */
		NVIC_EnableIRQ(SysTicK_IRQn);
		SysTick->CTLR |= (1<<1);
	}
	else
	{
		systick_inc = 0;
		
		/* disable the SysTick IRQ */
		NVIC_DisableIRQ(SysTicK_IRQn);
		SysTick->CTLR &= ~(1<<1);
	}
}

/*
 * common delay routine
 * kanged from ch32v003fun
 */
void DelaySysTick( uint32_t n )
{
	uint32_t targend = SysTick->CNT + n;
	while( ((int32_t)( SysTick->CNT - targend )) < 0 );
}

/*
 * SysTick ISR just counts ticks for now
 */
void SysTick_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void SysTick_Handler(void)
{
	// move the compare further ahead in time.
	// as a warning, if more than this length of time
	// passes before triggering, you may miss your
	// interrupt.
	SysTick->CMP += systick_inc;

	/* clear IRQ */
	SysTick->SR = 0;

	/* update counter */
	systick_cnt++;
}
