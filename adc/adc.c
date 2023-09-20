/*
 * adc.c - ch32v203 ADC driver
 * E. Brombaugh 09-19-23
 */

#include "adc.h"
#include "debug.h"
#include <string.h>

uint16_t adc_rawchls[ADC_NUMCHLS];

/*
 * set up ADC
 */
void adc_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
	ADC_InitTypeDef ADC_InitStructure = {0};
	DMA_InitTypeDef	DMA_InitStructure = {0};
	
	/* init channel storage */
	memset(adc_rawchls, 0, ADC_NUMCHLS*sizeof(uint16_t));
	
	/* Configure GPIOs for analog input */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* set up operating mode */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	ADC_InitStructure =
	(ADC_InitTypeDef){
		.ADC_Mode = ADC_Mode_Independent,
		.ADC_ScanConvMode = ENABLE,
		.ADC_ContinuousConvMode = ENABLE,
		.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None,
		.ADC_DataAlign = ADC_DataAlign_Right,
		.ADC_NbrOfChannel = ADC_NUMCHLS,
		.ADC_OutputBuffer = ADC_OutputBuffer_Disable,
		.ADC_Pga = ADC_Pga_1
	};
	ADC_Init(ADC1, &ADC_InitStructure);
	
	/* set up channels */
	ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 1, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 2, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 3, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 4, ADC_SampleTime_239Cycles5);

	/* Turn on ADC and calibrate */
	ADC_Cmd(ADC1, ENABLE);
	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));
	
	/* set up DMA */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	DMA_InitStructure = 
	(DMA_InitTypeDef){
		.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->RDATAR,
		.DMA_MemoryBaseAddr = (uint32_t)adc_rawchls,
		.DMA_DIR = DMA_DIR_PeripheralSRC,
		.DMA_BufferSize = ADC_NUMCHLS,
		.DMA_PeripheralInc = DMA_PeripheralInc_Disable,
		.DMA_MemoryInc = DMA_MemoryInc_Enable,
		.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord,
		.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord,
		.DMA_Mode = DMA_Mode_Circular,
		.DMA_Priority = DMA_Priority_Medium,
		.DMA_M2M = DMA_M2M_Disable,
	};
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	DMA_Cmd(DMA1_Channel1, ENABLE);
	ADC_DMACmd(ADC1, ENABLE);
	
	/* Start ADC converting */
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

/*
 * get ADC value
 */
uint16_t adc_get_chl(int8_t chl)
{
	if(chl >= ADC_NUMCHLS)
		return 0;
	
	return adc_rawchls[chl];
}