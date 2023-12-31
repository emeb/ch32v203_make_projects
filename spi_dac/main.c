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

#include "uart_printf.h"
#include "systick.h"
#include "spi_dac.h"

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
    systick_init(1000);
    USART_Printf_Init(115200);
    printf("\r\nSPI DAC example\r\n");
    printf("SystemClk:%lud\r\n", SystemCoreClock);

    GPIO_Toggle_INIT();
    printf("LED GPIO initialized\r\n");

    spi_dac_init();
    printf("SPI DAC initialized\r\n");

    while(1)
    {
    	Delay_Ms( 100 );
        GPIO_WriteBit(GPIOA, GPIO_Pin_0, (count & 1) ? Bit_SET : Bit_RESET);
    	printf("%6d : ", count);
    	printf("0x%08lX ", osc_phs[0]);
    	printf("0x%08lX ", osc_phs[1]);
    	printf("0x%08lX ", systick_cnt);
    	//printf("0x%08lX ", SPI1->CTLR1);
    	//printf("0x%08lX ", SPI1->CTLR2);
    	//printf("0x%08lX ", SPI1->STATR);
    	//printf("0x%08lX ", DMA1_Channel4->MADDR);
    	//printf("0x%08lX ", DMA1_Channel4->CNTR);
    	printf("\n\r");
        count++;
    }
}
