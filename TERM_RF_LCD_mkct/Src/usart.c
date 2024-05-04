/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */
#include "stdio.h"
#include "stdint.h"
#include "setup.h"
#include "system.h"
#include "control.h"
#include "mk_conf_tree.h"

USART_TX_BUFFER USART1Tx;
USART_RX_BUFFER USART1Rx;
//USART_TX_BUFFER USART2Tx;
USART_RX_BUFFER USART2Rx;

u16 flag_usb_pkt = 0;
u16 flag_sbus_pkt = 0;

/* USER CODE END 0 */

/* USART2 init function */

void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  LL_USART_InitTypeDef USART_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);

  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
  /**USART2 GPIO Configuration
  PA2   ------> USART2_TX
  PA3   ------> USART2_RX
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_2;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_3;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_FLOATING;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USART2 interrupt Init */
  NVIC_SetPriority(USART2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(USART2_IRQn);

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  USART_InitStruct.BaudRate = 19200;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART2, &USART_InitStruct);
  LL_USART_ConfigAsyncMode(USART2);
  LL_USART_Enable(USART2);
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/* USER CODE BEGIN 1 */
//=========================================================================
 void type_byte(uint8_t data)
 {
    while (!LL_USART_IsActiveFlag_TXE(USART1));
    LL_USART_TransmitData8(USART1, data);
 }
//=========================================================================
void USART_Write(USART_TypeDef *USARTx, uint8_t data)
 {
    while (!LL_USART_IsActiveFlag_TXE(USARTx));
    LL_USART_TransmitData8(USARTx, data);
 }

//=========================================================================
void USART_SetBaudRate(USART_TypeDef *USARTx, uint32_t baud)
{
    uint32_t PeriphClk;
    LL_RCC_ClocksTypeDef rcc_clocks;
    
    LL_RCC_GetSystemClocksFreq(&rcc_clocks);

    LL_USART_Disable(USARTx);
    
    if(USARTx == USART1) PeriphClk = rcc_clocks.PCLK2_Frequency;
    else if(USARTx == USART2) PeriphClk = rcc_clocks.PCLK1_Frequency;
    
    LL_USART_SetBaudRate(USARTx, PeriphClk, baud);
    LL_USART_Enable(USARTx);
}

//=========================================================================
void  USART1_RX_Callback(void)
{
 	u8 Data;
	volatile USART_RX_BUFFER *USARTRx;

    USARTRx = &USART1Rx;
 	Data = LL_USART_ReceiveData8(USART1);
 	if(USART_RxCount((*USARTRx)) < (USART_RX_BUFFER_SIZE - 1))
 	{
 		USARTRx->Buffer[USARTRx->WrPos++] = Data;
 		USARTRx->WrPos %= USART_RX_BUFFER_SIZE;
  	}
}
 
//=========================================================================
void  USART2_RX_Callback(void)
{
 	u8 Data;
	volatile USART_RX_BUFFER *USARTRx;

//    if(dev_var.dir) return;
    
    USARTRx = &USART2Rx;
 	Data = LL_USART_ReceiveData8(USART2);
 	if(USART_RxCount((*USARTRx)) < (USART_RX_BUFFER_SIZE - 1))
 	{
 		USARTRx->Buffer[USARTRx->WrPos++] = Data;
 		USARTRx->WrPos %= USART_RX_BUFFER_SIZE;
  	}
}
 
//=========================================================================
void USART2_Proc(void)
{
    if(USART2Rx.WrPos != USART2Rx.RdPos)
    {
        dev_var.count_pkt_in++;
    	MKBUS_rx(&mkChan1, USART2Rx.Buffer[USART2Rx.RdPos] );
        USART2Rx.RdPos = (USART2Rx.RdPos + 1) & USART_RX_MASK;
    }
//--------------------------------------------------------------------    
	if( (mkChan1.time+10 < HAL_GetTick()) && (mkChan1.txLen))
    {
        MKBUS_send(mkChan1.pkt, mkChan1.txLen);
		mkChan1.txLen = 0;
        
	}
}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
