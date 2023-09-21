/*
 * i2s.c - i2s driver for ch32v203. NOTE - only available on C8 / R8 packages!
 * 09-20-23 E. Brombaugh
 */
#include "i2s.h"

// this table contains a 512-sample 16-bit sine waveform
#include "Sine16bit.h"

// uncomment this to enable GPIO diag
#define I2S_DIAG

// Length of the DAC DMA buffer
#define I2SBUFSZ 32

/* audio output buffer */
uint16_t i2s_buffer[I2SBUFSZ];
uint32_t osc_phs[2], osc_frq[2];

/*
 * initialize I2S
 */
void i2s_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
	I2S_InitTypeDef I2S_InitStructure = {0};
	DMA_InitTypeDef	DMA_InitStructure = {0};
	
	/* init two oscillators */
	osc_phs[0] = 0;
	osc_phs[1] = 0;
	osc_frq[0] = 0x01000000;
	osc_frq[1] = 0x00400000;
	
	/* Set up I2S output pins */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
	
#ifdef I2S_DIAG
	// GPIO B12 50MHz Push-Pull for diags
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
#endif
	
	/* set up I2S */
	I2S_InitStructure =
	(I2S_InitTypeDef){
		.I2S_Mode = I2S_Mode_MasterTx,
		.I2S_Standard = I2S_Standard_MSB,
		.I2S_DataFormat = I2S_DataFormat_16b,
		.I2S_MCLKOutput = I2S_MCLKOutput_Disable,
		.I2S_AudioFreq = I2S_AudioFreq_48k,
		.I2S_CPOL = I2S_CPOL_High,
	};
	I2S_Init(SPI2, &I2S_InitStructure);
	
	/* set up DMA */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	DMA_InitStructure = 
	(DMA_InitTypeDef){
		.DMA_PeripheralBaseAddr = (uint32_t)&SPI2->DATAR,
		.DMA_MemoryBaseAddr = (uint32_t)i2s_buffer,
		.DMA_DIR = DMA_DIR_PeripheralDST,
		.DMA_BufferSize = I2SBUFSZ,
		.DMA_PeripheralInc = DMA_PeripheralInc_Disable,
		.DMA_MemoryInc = DMA_MemoryInc_Enable,
		.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord,
		.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord,
		.DMA_Mode = DMA_Mode_Circular,
		.DMA_Priority = DMA_Priority_Medium,
		.DMA_M2M = DMA_M2M_Disable,
	};
	DMA_Init(DMA1_Channel5, &DMA_InitStructure);
	
	/* set up DMA IRQ */
	DMA_ITConfig(DMA1_Channel5, DMA_IT_TC | DMA_IT_HT, ENABLE);
	NVIC_EnableIRQ( DMA1_Channel5_IRQn );
	
	/* Start it all up */
	DMA_Cmd(DMA1_Channel5, ENABLE);
	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);
	I2S_Cmd(SPI2, ENABLE);
}

/*
 * I2S buffer update - audio processing goes here
 */
void i2s_update(uint16_t *buffer)
{
	int i;
	
	/* fill the buffer with stereo data */
	for(i=0;i<I2SBUFSZ/2;i+=2)
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
 * I2S DMA IRQ handler
 */
void DMA1_Channel5_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void DMA1_Channel5_IRQHandler(void)
{
#ifdef I2S_DIAG
	GPIOB->BSHR = 1<<12;
#endif
	
	/* why is this needed? Can't just direct compare the reg in tests below */
	volatile uint16_t intfr = DMA1->INTFR;

	if( intfr & DMA1_IT_TC5 )
	{
		/* Transfer complete - update 2nd half */
		i2s_update(i2s_buffer+I2SBUFSZ/2);
		
		/* clear TC IRQ */
		DMA1->INTFCR = DMA1_IT_TC5;
	}
	
	if( intfr & DMA1_IT_HT5 )
	{
		/* Half transfer - update first half */
		i2s_update(i2s_buffer);
		
		/* clear HT IRQ */
		DMA1->INTFCR = DMA1_IT_HT5;
	}

	/* clear the Global IRQ */
	DMA1->INTFCR = DMA1_IT_GL5;
	
#ifdef I2S_DIAG
	GPIOB->BSHR = 1<<(16+12);
#endif
}
