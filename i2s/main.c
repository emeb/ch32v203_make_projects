/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2021/06/06
 * Description        : Main program body.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/

/*
 *@Note
 USART Print debugging routine:
 USART1_Tx(PA9).
 This example demonstrates using USART1(PA9) as a print debug port output.

*/

#include "debug.h"
#include "i2s.h"

/* Global typedef */

/* Global define */

/* Global Variable */

/*********************************************************************
 * @fn      GPIO_Toggle_INIT
 *
 * @brief   Initializes GPIOA.0
 *
 * @return  none
 */
void GPIO_Toggle_INIT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
	int count = 0;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    Delay_Init();
    USART_Printf_Init(115200);
    printf("\r\nI2S Example\r\n");
    printf("SystemClk:%lud\r\n", SystemCoreClock);

    GPIO_Toggle_INIT();
    printf("LED GPIO initialized\r\n");
	
#if 1
#if 0
	/* test if SPI2 I2SCFGR works */
	printf("I2SCFGR = 0x%08X\n\r", SPI2->I2SCFGR);
	printf("Enable I2S\n\r");
	//I2S_Cmd(SPI2, ENABLE);
	SPI2->I2SCFGR = SPI_I2SCFGR_I2SMOD;
	printf("I2SCFGR = 0x%08X\n\r", SPI2->I2SCFGR);
#else
	/* test if writing SPI2 reg works */
#define SPI2REG I2SCFGR
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	printf("SPI2REG = 0x%08X\n\r", SPI2->SPI2REG);
	printf("Write SPI2REG\n\r");
	SPI2->SPI2REG = 0xDEADBEEF;
	printf("SPI2REG = 0x%08X\n\r", SPI2->SPI2REG);
#endif

	printf("Done\n\r");
	while(1);
#else
	i2s_init();
    printf("I2S initialized\r\n");
	
    while(1)
    {
    	Delay_Ms( 100 );
        GPIO_WriteBit(GPIOA, GPIO_Pin_0, (count & 1) ? Bit_SET : Bit_RESET);
    	printf("%6d : ", count);
    	//printf("0x%08X ", osc_phs[0]);
    	//printf("0x%08X ", osc_phs[1]);
    	printf("0x%08X ", SPI2->CTLR1);
    	printf("0x%08X ", SPI2->CTLR2);
    	printf("0x%08X ", SPI2->I2SCFGR);
    	printf("0x%08X ", SPI2->STATR);
    	//printf("0x%08X ", DMA1_Channel5->MADDR);
    	//printf("0x%08X ", DMA1_Channel5->CNTR);
    	printf("\n\r");
        count++;
    }
#endif
}
