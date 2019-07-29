/**
  ******************************************************************************
  * File Name          : USART.c
  * Description        : This file provides code for the configuration
  *                      of the USART instances.
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2019 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */
#define UART1_RXBUF_SIZE 64

extern osSemaphoreId UART1_DMA_TX_lock;
extern osSemaphoreId UART2_DMA_TX_lock;
extern osSemaphoreId UART6_TX_lock;
extern osSemaphoreId UART6_RX_lock;

volatile uint8_t dummy;
/* USER CODE END 0 */

/* USART1 init function */

void MX_USART1_UART_Init(void)
{
  LL_USART_InitTypeDef USART_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
  
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  /**USART1 GPIO Configuration  
  PA9   ------> USART1_TX
  PA10   ------> USART1_RX 
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_9|LL_GPIO_PIN_10;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USART1 DMA Init */
  
  /* USART1_TX Init */
  LL_DMA_SetChannelSelection(DMA2, LL_DMA_STREAM_7, LL_DMA_CHANNEL_4);

  LL_DMA_SetDataTransferDirection(DMA2, LL_DMA_STREAM_7, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);

  LL_DMA_SetStreamPriorityLevel(DMA2, LL_DMA_STREAM_7, LL_DMA_PRIORITY_LOW);

  LL_DMA_SetMode(DMA2, LL_DMA_STREAM_7, LL_DMA_MODE_NORMAL);

  LL_DMA_SetPeriphIncMode(DMA2, LL_DMA_STREAM_7, LL_DMA_PERIPH_NOINCREMENT);

  LL_DMA_SetMemoryIncMode(DMA2, LL_DMA_STREAM_7, LL_DMA_MEMORY_INCREMENT);

  LL_DMA_SetPeriphSize(DMA2, LL_DMA_STREAM_7, LL_DMA_PDATAALIGN_BYTE);

  LL_DMA_SetMemorySize(DMA2, LL_DMA_STREAM_7, LL_DMA_MDATAALIGN_BYTE);

  LL_DMA_DisableFifoMode(DMA2, LL_DMA_STREAM_7);

  USART_InitStruct.BaudRate = 9600;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART1, &USART_InitStruct);
  LL_USART_ConfigAsyncMode(USART1);
  LL_USART_Enable(USART1);

}
/* USART2 init function */

void MX_USART2_UART_Init(void)
{
  LL_USART_InitTypeDef USART_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
  
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  /**USART2 GPIO Configuration  
  PA2   ------> USART2_TX
  PA3   ------> USART2_RX 
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_2|LL_GPIO_PIN_3;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USART2 DMA Init */
  
  /* USART2_TX Init */
  LL_DMA_SetChannelSelection(DMA1, LL_DMA_STREAM_6, LL_DMA_CHANNEL_4);

  LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_STREAM_6, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);

  LL_DMA_SetStreamPriorityLevel(DMA1, LL_DMA_STREAM_6, LL_DMA_PRIORITY_LOW);

  LL_DMA_SetMode(DMA1, LL_DMA_STREAM_6, LL_DMA_MODE_NORMAL);

  LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_STREAM_6, LL_DMA_PERIPH_NOINCREMENT);

  LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_STREAM_6, LL_DMA_MEMORY_INCREMENT);

  LL_DMA_SetPeriphSize(DMA1, LL_DMA_STREAM_6, LL_DMA_PDATAALIGN_BYTE);

  LL_DMA_SetMemorySize(DMA1, LL_DMA_STREAM_6, LL_DMA_MDATAALIGN_BYTE);

  LL_DMA_DisableFifoMode(DMA1, LL_DMA_STREAM_6);

  USART_InitStruct.BaudRate = 115200;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART2, &USART_InitStruct);
  LL_USART_ConfigAsyncMode(USART2);
  LL_USART_Enable(USART2);

}
/* USART6 init function */

void MX_USART6_UART_Init(void)
{
  LL_USART_InitTypeDef USART_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART6);
  
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
  /**USART6 GPIO Configuration  
  PC6   ------> USART6_TX
  PC7   ------> USART6_RX 
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_6|LL_GPIO_PIN_7;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_8;
  LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* USART6 DMA Init */
  
  /* USART6_TX Init */
  LL_DMA_SetChannelSelection(DMA2, LL_DMA_STREAM_6, LL_DMA_CHANNEL_5);

  LL_DMA_SetDataTransferDirection(DMA2, LL_DMA_STREAM_6, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);

  LL_DMA_SetStreamPriorityLevel(DMA2, LL_DMA_STREAM_6, LL_DMA_PRIORITY_LOW);

  LL_DMA_SetMode(DMA2, LL_DMA_STREAM_6, LL_DMA_MODE_NORMAL);

  LL_DMA_SetPeriphIncMode(DMA2, LL_DMA_STREAM_6, LL_DMA_PERIPH_NOINCREMENT);

  LL_DMA_SetMemoryIncMode(DMA2, LL_DMA_STREAM_6, LL_DMA_MEMORY_INCREMENT);

  LL_DMA_SetPeriphSize(DMA2, LL_DMA_STREAM_6, LL_DMA_PDATAALIGN_BYTE);

  LL_DMA_SetMemorySize(DMA2, LL_DMA_STREAM_6, LL_DMA_MDATAALIGN_BYTE);

  LL_DMA_DisableFifoMode(DMA2, LL_DMA_STREAM_6);

  USART_InitStruct.BaudRate = 115200;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART6, &USART_InitStruct);
  LL_USART_ConfigAsyncMode(USART6);
  LL_USART_Enable(USART6);

}

/* USER CODE BEGIN 1 */

void USART2_IRQHandler(void)
{
	if (LL_USART_IsActiveFlag_TC(USART2)) {
		LL_USART_ClearFlag_TC(USART2);
		osSemaphoreRelease(UART2_DMA_TX_lock);
	}
}

void UART2_user_init(void)
{
	osSemaphoreWait(UART2_DMA_TX_lock, osWaitForever);
	LL_DMA_SetPeriphAddress(DMA1, LL_DMA_STREAM_6, (uint32_t)&USART2->DR);

	HAL_NVIC_SetPriority(USART2_IRQn, 6, 0);
	HAL_NVIC_EnableIRQ(USART2_IRQn);
}

int UART2_write(const char *data, unsigned int size)
{
	if (!size) return size;

	while (LL_DMA_IsEnabledStream(DMA1, LL_DMA_STREAM_6));

	LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_6, (uint32_t)data);
	LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_6, size);

	LL_DMA_ClearFlag_HT6(DMA1);
	LL_DMA_ClearFlag_TC6(DMA1);

	LL_USART_ClearFlag_TC(USART2);
	LL_USART_EnableIT_TC(USART2);
	LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_6);

	LL_USART_EnableDMAReq_TX(USART2);

	osSemaphoreWait(UART2_DMA_TX_lock, osWaitForever);
	return size;
}


static volatile char UART1_rxbuf[UART1_RXBUF_SIZE];
static volatile unsigned int UART1_rxbufp_isr = 0;
static volatile unsigned int UART1_rxbufp_thd = 0;
static volatile uint8_t UART1_want_read = 0;

void USART1_IRQHandler(void)
{

	if (LL_USART_IsActiveFlag_TC(USART1)) {

		LL_USART_ClearFlag_TC(USART1);


		LL_USART_ClearFlag_RXNE(USART1);
		LL_USART_EnableIT_RXNE(USART1);   // sending done, no mirroring anymore
		dummy = USART1->DR;				// clears ORE bit

		osSemaphoreRelease(UART1_DMA_TX_lock);

	} else if (LL_USART_IsActiveFlag_RXNE(USART1) ) {
		LL_USART_ClearFlag_RXNE(USART1);

		UART1_rxbuf[UART1_rxbufp_isr++] = USART1->DR;
		UART1_rxbufp_isr %= UART1_RXBUF_SIZE;

		if (UART1_want_read) {
			UART1_want_read--;

			if (UART1_want_read == 0) {
				osSemaphoreRelease(UART1_DMA_TX_lock);
			}
		}

	} else if (LL_USART_IsActiveFlag_ORE(USART1)) {
		dummy = USART1->DR;
	}
}
//
void UART1_user_init(void)
{
	osSemaphoreWait(UART1_DMA_TX_lock, osWaitForever);
	LL_DMA_SetPeriphAddress(DMA2, LL_DMA_STREAM_7, (uint32_t)&USART1->DR);

	HAL_NVIC_SetPriority(USART1_IRQn, 6, 0);
	HAL_NVIC_EnableIRQ(USART1_IRQn);
}
//
int UART1_read(char* data, unsigned int size, unsigned timeout)
{
	osThreadSuspendAll();
	LL_USART_DisableIT_RXNE(USART1);

	int diff = UART1_rxbufp_isr - UART1_rxbufp_thd;

	if (diff < 0) {
		// ISR index wrapped around
		diff += UART1_RXBUF_SIZE;
	}

	if (diff >= size) {
		LL_USART_EnableIT_RXNE(USART1);
		osThreadResumeAll();


	} else {
		UART1_want_read = size - diff;
		LL_USART_EnableIT_RXNE(USART1);
		osThreadResumeAll();

		int32_t sem_tok = osSemaphoreWait(UART1_DMA_TX_lock, timeout);

		if (sem_tok != osOK) {
			// overtime. Reset the semaphore and want read
			LL_USART_DisableIT_RXNE(USART1);

			osSemaphoreWait(UART1_DMA_TX_lock, 0);
			UART1_want_read = 0;

			LL_USART_EnableIT_RXNE(USART1);
			return -1;
		}
	}

	for (unsigned i = 0; i < size; i++) {
		data[i] = UART1_rxbuf[UART1_rxbufp_thd++];
		UART1_rxbufp_thd %= UART1_RXBUF_SIZE;
	}

	return 0;
}

int UART1_write(const char *data, unsigned int size)
{
	if (!size) return 0;

	while (LL_DMA_IsEnabledStream(DMA2, LL_DMA_STREAM_7));

	LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_7, (uint32_t)data);
	LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_7, size);

	LL_DMA_ClearFlag_HT7(DMA2);
	LL_DMA_ClearFlag_TC7(DMA2);

	LL_USART_DisableIT_RXNE(USART1);	// disable because of mirroring

	LL_USART_ClearFlag_TC(USART1);
	LL_USART_EnableIT_TC(USART1);
	LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_7);

	LL_USART_EnableDMAReq_TX(USART1);

	osSemaphoreWait(UART1_DMA_TX_lock, osWaitForever);

	return 0;
}

#define UART6_RXBUF_SIZE 128
static volatile char UART6_rxbuf[UART6_RXBUF_SIZE];

static volatile unsigned int UART6_rxbufp_isr = 0;
static volatile unsigned int UART6_rxbufp_thd = 0;
static volatile unsigned int USART6_newline_seen = 0;
static volatile unsigned int USART6_buf_cnt = 0;
static volatile int UART6_read_action; // 0 nothing, -1 readln,  n > 0 read n bytes

void USART6_IRQHandler(void)
{


	if (LL_USART_IsActiveFlag_TC(USART6)) {
		LL_USART_ClearFlag_TC(USART6);
		osSemaphoreRelease(UART6_TX_lock);
	}

	if (LL_USART_IsEnabledIT_RXNE(USART6) && LL_USART_IsActiveFlag_RXNE(USART6) ) {
		LL_USART_ClearFlag_RXNE(USART6);

		if (USART6_buf_cnt < UART6_RXBUF_SIZE) {
			USART6_buf_cnt++;
			UART6_rxbuf[UART6_rxbufp_isr] = USART6->DR;
			if (UART6_rxbuf[UART6_rxbufp_isr] == '\n') {
				USART6_newline_seen++;
			}

			UART6_rxbufp_isr++;
			UART6_rxbufp_isr %= UART6_RXBUF_SIZE;

			if (UART6_read_action > 0) {
				UART6_read_action--;

				if (UART6_read_action == 0) {
					osSemaphoreRelease(UART6_RX_lock);
				}

			} else if (UART6_read_action == -1) {
				if (USART6_newline_seen) {
					UART6_read_action = 0;

					osSemaphoreRelease(UART6_RX_lock);

				}
			}
		}

	}

	if (LL_USART_IsActiveFlag_ORE(USART6)) {
		dummy = USART6->DR;
	}
}

void UART6_user_init(void)
{
	osSemaphoreWait(UART6_RX_lock, 0);
	osSemaphoreWait(UART6_TX_lock, 0);
	LL_DMA_SetPeriphAddress(DMA2, LL_DMA_STREAM_6, (uint32_t)&USART6->DR);

	HAL_NVIC_SetPriority(USART6_IRQn, 6, 0);
	HAL_NVIC_EnableIRQ(USART6_IRQn);
}

int UART6_read(char* data, unsigned int size, unsigned timeout)
{
	osThreadSuspendAll();
	LL_USART_DisableIT_RXNE(USART6);

	int diff = USART6_buf_cnt;

	if (diff >= size) {
		LL_USART_EnableIT_RXNE(USART6);
		osThreadResumeAll();


	} else {
		UART6_read_action = size - diff;
		LL_USART_EnableIT_RXNE(USART6);
		osThreadResumeAll();

		int32_t sem_tok = osSemaphoreWait(UART6_RX_lock, timeout);
		if (sem_tok != osOK) {
			/*
			 * Make sure the semaphore is cleared.
			 */
			UART6_read_action = 0;
			osSemaphoreWait(UART6_RX_lock, 0);

			return -1; // overtime
		}
	}

	for (unsigned i = 0; i < size; i++) {
		data[i] = UART6_rxbuf[UART6_rxbufp_thd++];
		if (data[i] == '\n')
			USART6_newline_seen--;
		UART6_rxbufp_thd %= UART6_RXBUF_SIZE;
		USART6_buf_cnt--;
	}

	return 0;
}

int UART6_readln(char *data, unsigned int max_size, unsigned timeout)
{
	osThreadSuspendAll();
	LL_USART_DisableIT_RXNE(USART6);

	if (USART6_newline_seen) {

		LL_USART_EnableIT_RXNE(USART6);
		osThreadResumeAll();

	} else {
		UART6_read_action = -1;

		LL_USART_EnableIT_RXNE(USART6);
		osThreadResumeAll();

		if (osSemaphoreWait(UART6_RX_lock, timeout) != osOK) {
			/*
			 * Make sure the semaphore is cleared.
			 */
			UART6_read_action = 0;
			osSemaphoreWait(UART6_RX_lock, 0);

			data[max_size - 1] = 0;
			return -1; // overtime

		}
	}

	unsigned i = 0;
	while (i < max_size) {
		data[i] = UART6_rxbuf[UART6_rxbufp_thd++];
		UART6_rxbufp_thd %= UART6_RXBUF_SIZE;
		USART6_buf_cnt--;

		if (data[i] == '\n') {
			USART6_newline_seen--;
			data[i] = 0;
			return 0;
		}

		i++;
	}

	data[max_size - 1] = 0;
	return -2; // overflow
}

int UART6_write(const char *data, unsigned int size)
{
	if (!size) return 0;

	LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_6, (uint32_t)data);
	LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_6, size);

	LL_DMA_ClearFlag_HT6(DMA2);
	LL_DMA_ClearFlag_TC6(DMA2);

	LL_USART_ClearFlag_TC(USART6);
	LL_USART_EnableIT_TC(USART6);
	LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_6);

	LL_USART_EnableDMAReq_TX(USART6);
	osSemaphoreWait(UART6_TX_lock, osWaitForever);

	return 0;
}

void UART6_clr(void)
{
	while (LL_DMA_IsEnabledStream(DMA2, LL_DMA_STREAM_6));

	LL_USART_DisableIT_RXNE(USART6);

	UART6_read_action = 0;
	USART6_newline_seen = 0;
	USART6_buf_cnt = 0;
	UART6_rxbufp_isr = 0;
	UART6_rxbufp_thd = 0;

	LL_USART_EnableIT_RXNE(USART6);
}

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
