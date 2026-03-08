/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "stdarg.h"
#include "string.h"

#define U1_TXBUFF_SIZE     1024
#define U2_TXBUFF_SIZE     2048
#define U2_RXBUFF_SIZE     2048
#define U2_RXMAX_SIZE      256

#define TX_NUM  10
#define RX_NUM  10

typedef struct{        
    uint8_t *StartPtr;
    uint8_t *EndPtr;
}USART_LocationPtrCB;

typedef struct{          
    uint32_t Usart_RxCounter;
    uint32_t Usart_TxCounter;
    uint32_t Usart_TxTimer;
    uint8_t  Usart_TxCpltflag;
    USART_LocationPtrCB  Usart_RxLocation[RX_NUM];
    USART_LocationPtrCB *UsartRxDataInPtr;
    USART_LocationPtrCB *UsartRxDataOutPtr;
    USART_LocationPtrCB *UsartRxDataEndPtr;
    USART_LocationPtrCB  Usart_TxLocation[TX_NUM];
    USART_LocationPtrCB *UsartTxDataInPtr;
    USART_LocationPtrCB *UsartTxDataOutPtr;
    USART_LocationPtrCB *UsartTxDataEndPtr;
    UART_HandleTypeDef *huart;
    DMA_HandleTypeDef  *hdmarx;
    DMA_HandleTypeDef  *hdmatx;
}USART_ControlCB;      

extern USART_ControlCB UART2_ControlCB;
extern uint8_t  U1_TxBuff[U1_TXBUFF_SIZE];
extern uint8_t  U2_TxBuff[U2_TXBUFF_SIZE];
extern uint8_t  U2_RxBuff[U2_RXBUFF_SIZE];

#define U4_RXMAX_SIZE      1024
#define U4_GPIO_USE_PC1011 1

extern uint8_t  U4_RxBuf[U4_RXMAX_SIZE];
extern volatile uint16_t U4_RxLen;
extern volatile uint8_t  U4_RxFlag;
/* USER CODE END Includes */

extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart6;

extern UART_HandleTypeDef huart1;

extern UART_HandleTypeDef huart2;

extern UART_HandleTypeDef huart3;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_UART5_Init(void);
void MX_USART6_UART_Init(void);
void MX_USART1_UART_Init(void);
void MX_USART2_UART_Init(void);
void MX_USART3_UART_Init(void);

/* USER CODE BEGIN Prototypes */
void u1_printf(char* fmt,...);
void u2_TxData(uint8_t *, uint16_t);
void u2_BuffInit(void);
void u2_TxDataBuf(uint8_t *, uint32_t);
extern uint8_t u3_rx_byte;
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

