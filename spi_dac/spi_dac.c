/*
 * spi_dac.c - spi_dac driver for ch32v203
 * 09-20-23 E. Brombaugh
 */
#include "spi_dac.h"

// this table contains a 512-sample 16-bit sine waveform
#include "Sine16bit.h"

// uncomment this to enable GPIO diag
#define SPI_DAC_DIAG

// Length of the DAC DMA buffer
#define SPIDACBUFSZ 32

/* audio output buffer */
uint16_t spi_dac_buffer[SPIDACBUFSZ];
uint32_t osc_phs[2], osc_frq[2];

/*
 * initialize spi_dac
 */
void spi_dac_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
	SPI_InitTypeDef SPI_InitStructure = {0};
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};
	TIM_OCInitTypeDef TIM_OCInitStructure = {0};
	DMA_InitTypeDef	DMA_InitStructure = {0};
	
	/* init two oscillators */
	osc_phs[0] = 0;
	osc_phs[1] = 0;
	osc_frq[0] = 0x01000000;
	osc_frq[1] = 0x00400000;
	
	/* Set up spi output pins SCK and MOSI */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
#ifdef SPI_DAC_DIAG
	// GPIO A4 50MHz Push-Pull for diags
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
#endif
	
	/* set up SPI port */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	SPI_InitStructure =
	(SPI_InitTypeDef){
		.SPI_Direction = SPI_Direction_1Line_Tx,
		.SPI_Mode = SPI_Mode_Master,
		.SPI_DataSize = SPI_DataSize_16b,
		.SPI_CPOL = SPI_CPOL_Low,
		.SPI_CPHA = SPI_CPHA_1Edge,
		.SPI_NSS = SPI_NSS_Soft,
		.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16,
		.SPI_FirstBit = SPI_FirstBit_MSB,
	};
	SPI_Init(SPI1, &SPI_InitStructure);
	SPI_Cmd(SPI1, ENABLE);
	
	/* GPIO A11 50MHz Push-Pull for WS from TIM1 CH4 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* set up TIM1 as timebase for WS and SPI TX */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	TIM_TimeBaseInitStructure =
	(TIM_TimeBaseInitTypeDef){
		.TIM_Prescaler = 3,		// for 144MHz -> 48MHz
		.TIM_CounterMode = TIM_CounterMode_Up,
		.TIM_Period = 499,
		.TIM_ClockDivision = TIM_CKD_DIV1,
		.TIM_RepetitionCounter = 0,
	};
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);
	//TIM_GenerateEvent(TIM1, TIM_EventSource_Update); // redundant
	TIM_CounterModeConfig(TIM1, TIM_CounterMode_CenterAligned3);
	
	/* Chl 4 is WS output 50% PWM */
	TIM_OCInitStructure =
	(TIM_OCInitTypeDef){
		.TIM_OCMode = TIM_OCMode_PWM1,
		.TIM_OutputState = TIM_OutputState_Enable,
		.TIM_OutputNState = TIM_OutputNState_Disable,
		.TIM_Pulse = 249,	// 50% 
		.TIM_OCPolarity = TIM_OCPolarity_High,
		.TIM_OCNPolarity = TIM_OCNPolarity_High,
		.TIM_OCIdleState = TIM_OCIdleState_Reset,
		.TIM_OCNIdleState = TIM_OCNIdleState_Reset,
	};
	TIM_OC4Init(TIM1, &TIM_OCInitStructure);
	TIM_CCxCmd(TIM1, TIM_Channel_4, TIM_CCx_Enable);
	TIM_CtrlPWMOutputs(TIM1, ENABLE);
	TIM_DMACmd(TIM1, TIM_DMA_CC4, ENABLE);
	TIM_Cmd(TIM1, ENABLE);

	/* set up DMA */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	DMA_InitStructure = 
	(DMA_InitTypeDef){
		.DMA_PeripheralBaseAddr = (uint32_t)&SPI1->DATAR,
		.DMA_MemoryBaseAddr = (uint32_t)spi_dac_buffer,
		.DMA_DIR = DMA_DIR_PeripheralDST,
		.DMA_BufferSize = SPIDACBUFSZ,
		.DMA_PeripheralInc = DMA_PeripheralInc_Disable,
		.DMA_MemoryInc = DMA_MemoryInc_Enable,
		.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord,
		.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord,
		.DMA_Mode = DMA_Mode_Circular,
		.DMA_Priority = DMA_Priority_Medium,
		.DMA_M2M = DMA_M2M_Disable,
	};
	DMA_Init(DMA1_Channel4, &DMA_InitStructure);
	
	/* set up DMA IRQ */
	DMA_ITConfig(DMA1_Channel4, DMA_IT_TC | DMA_IT_HT, ENABLE);
	NVIC_EnableIRQ(DMA1_Channel4_IRQn);
	
	/* Start it all up */
	DMA_Cmd(DMA1_Channel4, ENABLE);
}

/*
 * spi_dac buffer update - audio processing goes here
 */
void spi_dac_update(uint16_t *buffer)
{
	int i;
	
	/* fill the buffer with stereo data */
	for(i=0;i<SPIDACBUFSZ/2;i+=2)
	{
		/* right chl */
		*buffer++ = Sine16bit[osc_phs[0]>>24];
		osc_phs[0] += osc_frq[0];
		
		/* left chl */
		//*buffer++ = Sine16bit[osc_phs[1]>>24];
		*buffer++ = osc_phs[1]>>16;
		osc_phs[1] += osc_frq[1];
	}
}

/*
 * DMA IRQ handler
 */
void DMA1_Channel4_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void DMA1_Channel4_IRQHandler(void)
{
#ifdef SPI_DAC_DIAG
	GPIOA->BSHR = 1<<4;
#endif
	
	/* why is this needed? Can't just direct compare the reg in tests below */
	volatile uint16_t intfr = DMA1->INTFR;

	if( intfr & DMA1_IT_TC4 )
	{
		/* Transfer complete - update 2nd half */
		spi_dac_update(spi_dac_buffer+SPIDACBUFSZ/2);
		
		/* clear TC IRQ */
		DMA1->INTFCR = DMA1_IT_TC4;
	}
	
	if( intfr & DMA1_IT_HT4 )
	{
		/* Half transfer - update first half */
		spi_dac_update(spi_dac_buffer);
		
		/* clear HT IRQ */
		DMA1->INTFCR = DMA1_IT_HT4;
	}

	/* clear the Global IRQ */
	DMA1->INTFCR = DMA1_IT_GL4;
	
#ifdef SPI_DAC_DIAG
	GPIOA->BSHR = 1<<(16+4);
#endif
}
