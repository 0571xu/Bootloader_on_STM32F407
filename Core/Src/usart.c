/* USER CODE BEGIN Header */
/**
	******************************************************************************
	* @file    usart.c
	* @brief   This file provides code for the configuration
	*          of the USART instances.
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
/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */
#include "cmsis_os.h"

/* ==================== 全局变量 ==================== */

uint8_t CercleBuffer[CercleBufferSize];
volatile uint16_t read_pos = 0; // 用于跟踪从 DMA 缓冲区读取数据的位置

uint8_t send_buf[100]; //发送缓冲区

/* 发送互斥锁：多个任务共用 USART1，必须串行发送 */
osMutexDef(uartTxMutex);
osMutexId uartTxMutexId;

/* DMA 发送完成信号量：在 USART1 中断里释放，uart_send() 等它 */
osSemaphoreDef(uartTxDone);
osSemaphoreId uartTxDoneSem;

/* USER CODE END 0 */

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* 创建串口发送互斥锁（此时调度器尚未启动，创建是安全的） */
  uartTxMutexId = osMutexCreate(osMutex(uartTxMutex));

  /* CMSIS 封装里 count=1 表示创建“二值信号量”，且初始就有 1 个 token，这里取走使其为 0 */
  uartTxDoneSem = osSemaphoreCreate(osSemaphore(uartTxDone), 1);
  osSemaphoreWait(uartTxDoneSem, 0);

  /*
   * 接收 DMA 与空闲中断必须在这里启动，不能放在 HAL_UART_MspInit() 里：
   * MspInit 是在 HAL_UART_Init() 内部调用的，那时 RxState 还不是 READY，
   * HAL_UART_Receive_DMA() 会直接返回 HAL_BUSY，导致串口永远收不到数据。
   */
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
  if (HAL_UART_Receive_DMA(&huart1, CercleBuffer, CercleBufferSize) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE END USART1_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 DMA Init */
    /* USART1_RX Init */
    hdma_usart1_rx.Instance = DMA2_Stream2;
    hdma_usart1_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart1_rx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart1_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart1_rx);

    /* USART1_TX Init */
    hdma_usart1_tx.Instance = DMA2_Stream7;
    hdma_usart1_tx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_tx.Init.Mode = DMA_NORMAL;
    hdma_usart1_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart1_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart1_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmatx,hdma_usart1_tx);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspInit 1 */

	/* 空闲中断和 DMA 接收不在这里启动，见 MX_USART1_UART_Init() 末尾 */

  /* USER CODE END USART1_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

    /* USART1 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
    HAL_DMA_DeInit(uartHandle->hdmatx);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* 带互斥保护的串口发送：多个任务可以安全地共用 USART1
   （HAL_UART_Transmit 是轮询发送，若上一个任务还在发送中，
     第二个任务会直接拿到 HAL_BUSY 而一个字都发不出去） */
void uart_send(const uint8_t *data, uint16_t len)
{
  if (uartTxMutexId != NULL)
    osMutexWait(uartTxMutexId, osWaitForever);

  if (uartTxDoneSem != NULL)
    osSemaphoreWait(uartTxDoneSem, 0); /* 清掉上一次可能残留的信号 */

  /* 只有启动成功才等完成信号，否则会永远等不到（HAL_BUSY 时不会触发 TxCplt） */
  if (HAL_UART_Transmit_DMA(&huart1, data, len) == HAL_OK)
  {
    if (uartTxDoneSem != NULL)
      osSemaphoreWait(uartTxDoneSem, osWaitForever);
  }

  if (uartTxMutexId != NULL)
    osMutexRelease(uartTxMutexId);
}

/* USER CODE END 1 */

