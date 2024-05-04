/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "system.h"

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
#define USART_TX_BUFFER_SIZE			512				//	Размер исходящего буфера USART
#define USART_RX_BUFFER_SIZE			512				//	Размер входящего буфера USART
#define USART_TX_MASK (USART_TX_BUFFER_SIZE-1)
#define USART_RX_MASK (USART_RX_BUFFER_SIZE-1)

#define USART_TxCount(USARTTX)			(USARTTX.WrPos - USARTTX.RdPos + ((USARTTX.WrPos < USARTTX.RdPos)?USART_TX_BUFFER_SIZE:0))		//	Кол-во пакетов на отправку в буфере
#define USART_RxCount(USARTRX)			(USARTRX.WrPos - USARTRX.RdPos + ((USARTRX.WrPos < USARTRX.RdPos)?USART_RX_BUFFER_SIZE:0))		//	Кол-во принятых пакетов в буфере

typedef struct
{
	s16 WrPos;
	s16 RdPos;
	uint8_t Buffer[USART_TX_BUFFER_SIZE];
} USART_TX_BUFFER;

typedef struct
{
	s16 WrPos;
	s16 RdPos;
	uint8_t Buffer[USART_RX_BUFFER_SIZE];
} USART_RX_BUFFER;

extern USART_TX_BUFFER USART1Tx;
extern USART_RX_BUFFER USART1Rx;
//extern USART_TX_BUFFER USART2Tx;
extern USART_RX_BUFFER USART2Rx;

extern s16 timeout_usb;

/* USER CODE END Private defines */

void MX_USART2_UART_Init(void);

/* USER CODE BEGIN Prototypes */

void USART_Write(USART_TypeDef *USARTx, uint8_t data);
void type_byte(uint8_t data);
void USART_SetBaudRate(USART_TypeDef *USARTx, uint32_t baud);
void  USART1_RX_Callback(void);
void  USART2_RX_Callback(void);
void USART1_Proc(void);
void USART2_Proc(void);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
