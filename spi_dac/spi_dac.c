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

/* uncomment this to use WCH Peripheral API instead of direct write */
#define WCH_PERIPH_API

/* audio output buffer */
uint16_t spi_dac_buffer[SPIDACBUFSZ];
uint32_t osc_phs[2], osc_frq[2];

/*
 * initialize spi_dac
 */
void spi_dac_init(void)
{
	
	/* init two oscillators */
	osc_phs[0] = 0;
	osc_phs[1] = 0;
	osc_frq[0] = 0x01000000;
	osc_frq[1] = 0x00400000;
	
	/* Set up spi output pins SCK and MOSI */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure =
	{
		.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7,
		.GPIO_Speed = GPIO_Speed_50MHz,
		.GPIO_Mode = GPIO_Mode_AF_PP,
	};
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* GPIO A11 50MHz Push-Pull for WS from TIM1 CH4 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
#ifdef SPI_DAC_DIAG
	/* GPIO A4 50MHz Push-Pull for diags */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
#endif
	
	/* set up SPI port */
#ifdef WCH_PERIPH_API
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	SPI_InitTypeDef SPI_InitStructure =
	(SPI_InitTypeDef){
		.SPI_Direction = SPI_Direction_1Line_Tx,
		.SPI_Mode = SPI_Mode_Master,
		.SPI_DataSize = SPI_DataSize_16b,
		.SPI_CPOL = SPI_CPOL_Low,
		.SPI_CPHA = SPI_CPHA_1Edge,
		.SPI_NSS = SPI_NSS_Soft,
		.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64,
		.SPI_FirstBit = SPI_FirstBit_MSB,
	};
	SPI_Init(SPI1, &SPI_InitStructure);
	SPI_Cmd(SPI1, ENABLE);
#else
	RCC->APB2PCENR |= RCC_APB2Periph_SPI1;
	SPI1->CTLR1 = 
		SPI_NSS_Soft | SPI_CPHA_1Edge | SPI_CPOL_Low | SPI_DataSize_16b |
		SPI_Mode_Master | SPI_Direction_1Line_Tx |
		SPI_BaudRatePrescaler_64;

	// enable SPI port
	SPI1->CTLR1 |= SPI_CTLR1_SPE;
#endif

	/* set up TIM1 as timebase for WS and SPI TX */
#ifdef WCH_PERIPH_API
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure =
	(TIM_TimeBaseInitTypeDef){
		.TIM_Prescaler = 2,		// for 144MHz -> 48MHz
		.TIM_CounterMode = TIM_CounterMode_Up,
		.TIM_Period = 499,
		.TIM_ClockDivision = TIM_CKD_DIV1,
		.TIM_RepetitionCounter = 0,
	};
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);
	TIM_CounterModeConfig(TIM1, TIM_CounterMode_CenterAligned3);
	
	/* Chl 4 is WS output 50% PWM */
	TIM_OCInitTypeDef TIM_OCInitStructure = {0};
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
#else
	RCC->APB2PCENR |= RCC_APB2Periph_TIM1;
	
	// Reset TIM1 to init all regs
	RCC->APB2PRSTR |= RCC_APB2Periph_TIM1;
	RCC->APB2PRSTR &= ~RCC_APB2Periph_TIM1;
	
	// CTLR1: default is up, events generated, edge align
	// CTLR1: up/down, events on both edges
	TIM1->CTLR1 = TIM_CMS;
	// SMCFGR: default clk input is CK_INT
	
	// Prescaler 
	TIM1->PSC = 0x0002;
	
	// Auto Reload - sets period to ~47kHz
	TIM1->ATRLR = 499;
	
	// Reload immediately
	TIM1->SWEVGR |= TIM_UG;
	
	// Enable CH4 output, positive pol
	TIM1->CCER |= TIM_CC4E;// | TIM_CC4P;
	
	// CH2 Mode is output, PWM1 (CC1S = 00, OC1M = 110)
	TIM1->CHCTLR2 |= TIM_OC4M_2 | TIM_OC4M_1;
	
	// Set the Capture Compare Register value to 50% initially
	TIM1->CH4CVR = 256;
	
	// Enable TIM1 outputs
	TIM1->BDTR |= TIM_MOE;
	
	// Enable CH4 DMA Req
	TIM1->DMAINTENR |= TIM_CC4DE;
	
	// Enable TIM1
	TIM1->CTLR1 |= TIM_CEN;
#endif
	
	/* set up DMA */
#ifdef WCH_PERIPH_API
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	DMA_InitTypeDef DMA_InitStructure = 
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
#else
	RCC->AHBPCENR |= RCC_AHBPeriph_DMA1;
	DMA1_Channel4->PADDR = (uint32_t)&SPI1->DATAR;
	DMA1_Channel4->MADDR = (uint32_t)spi_dac_buffer;
	DMA1_Channel4->CNTR  = SPIDACBUFSZ;
	DMA1_Channel4->CFGR  =
		DMA_M2M_Disable |		 
		DMA_Priority_VeryHigh |
		DMA_MemoryDataSize_HalfWord |
		DMA_PeripheralDataSize_HalfWord |
		DMA_MemoryInc_Enable |
		DMA_Mode_Circular |
		DMA_DIR_PeripheralDST |
		DMA_IT_TC | DMA_IT_HT;

	NVIC_EnableIRQ( DMA1_Channel4_IRQn );
	DMA1_Channel4->CFGR |= DMA_CFGR1_EN;
#endif
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
