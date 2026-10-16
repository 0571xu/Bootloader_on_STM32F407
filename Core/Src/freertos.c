/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId ledHandle;
osThreadId oledmenuHandle;
osThreadId wifi_getHandle;
osThreadId ota_flashHandle;
osThreadId request_upgradeHandle;
osMessageQId xFrameQueueHandle;
osMessageQId atQueueHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void ledTask(void const * argument);
void oledmenuTask(void const * argument);
void wifi_getTask(void const * argument);
void ota_flashTask(void const * argument);
void request_upgradeTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* definition and creation of xFrameQueue */
  osMessageQDef(xFrameQueue, 4, uint16_t);
  xFrameQueueHandle = osMessageCreate(osMessageQ(xFrameQueue), NULL);

  /* definition and creation of atQueue */
  osMessageQDef(atQueue, 8, uint16_t);
  atQueueHandle = osMessageCreate(osMessageQ(atQueue), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of led */
  osThreadDef(led, ledTask, osPriorityNormal, 0, 128);
  ledHandle = osThreadCreate(osThread(led), NULL);

  /* definition and creation of oledmenu */
  osThreadDef(oledmenu, oledmenuTask, osPriorityNormal, 0, 256);
  oledmenuHandle = osThreadCreate(osThread(oledmenu), NULL);

  /* definition and creation of wifi_get */
  osThreadDef(wifi_get, wifi_getTask, osPriorityHigh, 0, 384);
  wifi_getHandle = osThreadCreate(osThread(wifi_get), NULL);

  /* definition and creation of ota_flash */
  osThreadDef(ota_flash, ota_flashTask, osPriorityHigh, 0, 256);
  ota_flashHandle = osThreadCreate(osThread(ota_flash), NULL);

  /* definition and creation of request_upgrade */
  osThreadDef(request_upgrade, request_upgradeTask, osPriorityNormal, 0, 128);
  request_upgradeHandle = osThreadCreate(osThread(request_upgrade), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_ledTask */
/**
  * @brief  Function implementing the led thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_ledTask */
void ledTask(void const * argument)
{
  /* USER CODE BEGIN ledTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END ledTask */
}

/* USER CODE BEGIN Header_oledmenuTask */
/**
* @brief Function implementing the oledmenu thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_oledmenuTask */
void oledmenuTask(void const * argument)
{
  /* USER CODE BEGIN oledmenuTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END oledmenuTask */
}

/* USER CODE BEGIN Header_wifi_getTask */
/**
* @brief Function implementing the wifi_get thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_wifi_getTask */
void wifi_getTask(void const * argument)
{
  /* USER CODE BEGIN wifi_getTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END wifi_getTask */
}

/* USER CODE BEGIN Header_ota_flashTask */
/**
* @brief Function implementing the ota_flash thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_ota_flashTask */
void ota_flashTask(void const * argument)
{
  /* USER CODE BEGIN ota_flashTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END ota_flashTask */
}

/* USER CODE BEGIN Header_request_upgradeTask */
/**
* @brief Function implementing the request_upgrade thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_request_upgradeTask */
void request_upgradeTask(void const * argument)
{
  /* USER CODE BEGIN request_upgradeTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END request_upgradeTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
