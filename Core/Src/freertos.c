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
#include "stm32f4xx_hal.h"
#include "usart.h"
#include "ota_receive.h"
#include "stream_buffer.h"
#include "queue.h"
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

/* 结束帧解析出的固件信息，供 request_upgrade_task 写入 config */
static uint32_t g_firmware_size = 0;
static uint32_t g_firmware_crc  = 0;

/* USER CODE END Variables */
osThreadId ledHandle;
osThreadId oledmenuHandle;
osThreadId wifi_getHandle;
osThreadId ota_flashHandle;
osThreadId request_upgradeHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void ledTask(void const *argument);
void oledmenuTask(void const *argument);
void wifi_getTask(void const *argument);
void ota_flashTask(void const *argument);
void request_upgradeTask(void const *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize);

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize)
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
void MX_FREERTOS_Init(void)
{
	/* USER CODE BEGIN Init */
	/* 创建流缓冲区与队列：必须在任何任务使用它们之前调用 */
	ota_receive_init();
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

	/* USER CODE BEGIN RTOS_QUEUES */
	/* xFrameQueue / atQueue 由 ota_receive_init() 创建：元素分别是 fw_frame_t 和 AT_LINE_MAX 字节的行，
	   不能用 CMSIS 消息队列（只支持 1/2/4/8 字节元素） */
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
	/* ota_flashTask 通过这个句柄通知 request_upgradeTask，必须在任务启动前绑定 */
	xrequest_upgrade_taskHandle = request_upgradeHandle;
	/* USER CODE END RTOS_THREADS */
}

/* USER CODE BEGIN Header_ledTask */
/**
 * @brief  Function implementing the led thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_ledTask */
void ledTask(void const *argument)
{
	/* USER CODE BEGIN ledTask */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1,GPIO_PIN_RESET);
    osDelay(500);
	
	// char *str3 = "jump success!\r\n";
	// int len3 = strlen(str3);
	// HAL_UART_Transmit(&huart1, (uint8_t *)str3, len3, HAL_MAX_DELAY);

	/* Infinite loop */
	for (;;)
	{
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1);

		osDelay(500);
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
void oledmenuTask(void const *argument)
{
	/* USER CODE BEGIN oledmenuTask */
	/* Infinite loop */
	for (;;)
	{
		static const char msg[] = "here is oledmenuTask!\r\n";
		uart_send((const uint8_t *)msg, sizeof(msg) - 1);
		osDelay(1000);
	}
	/* USER CODE END oledmenuTask */
}

/* USER CODE BEGIN Header_wifi_getTask */
/**
 * @brief Function implementing the wifi_get thread.
 * @param argument: Not used
 * @retval None
 */
/* ==================== 逐字节解析帧状态机 + AT 行处理 ==================== */
/* USER CODE END Header_wifi_getTask */
void wifi_getTask(void const *argument)
{
	/* USER CODE BEGIN wifi_getTask */
	/* Infinite loop */

	uint8_t byte;
	uint8_t line[AT_LINE_MAX];
	size_t line_len = 0;
	parser_t parser;

	/* 初始化解析器 */
	memset(&parser, 0, sizeof(parser));
	parser.state = 0; /* ST_HEADER_1 */

	for (;;)
	{

		static const char msg[] = "here is wifi_getTask!\r\n";
		uart_send((const uint8_t *)msg, sizeof(msg) - 1);
		osDelay(1000);

		/* 一次读一个字节 */
		if (xStreamBufferReceive(uartStreamBuffer, &byte, 1, portMAX_DELAY) != 1)
			continue;

		/* ---------- 状态机 ---------- */
		switch (parser.state)
		{

		/* ===== 状态 0：等 0x55，同时处理 AT 行 ===== */
		case 0: /* ST_HEADER_1 */
			/* AT 行处理：遇到 \n 提交一行 */
			if (byte == '\n' && line_len > 0)
			{
				line[line_len] = '\0';
				xQueueSend(atQueue, line, 0);
				line_len = 0;
			}
			else if (byte != '\r')
			{
				if (line_len < AT_LINE_MAX - 1)
					line[line_len++] = byte;
			}

			/* 检测帧头起点 */
			if (byte == FRAME_HEADER_1)
			{
				parser.state = 1; /* ST_HEADER_2 */
			}
			break;

		/* ===== 状态 1：等 0x44 ===== */
		case 1: /* ST_HEADER_2 */
			if (byte == FRAME_HEADER_2)
			{
				/* 帧头确认，从帧池分配 slot */
				parser.slot = frame_pool_alloc();
				if (parser.slot == NULL)
				{
					parser.state = 0; /* 无空闲 slot，丢弃这一帧 */
					break;
				}
				parser.crc32 = 0;
				parser.crc_byte_idx = 0;
				parser.data_idx = 0;
				parser.state = 2; /* ST_TOTAL */
			}
			else if (byte == FRAME_HEADER_1)
			{
				/* 连续 0x55，保持在 ST_HEADER_2 */
				parser.state = 1;
			}
			else
			{
				/* 不是 0x44，回起点；当前字节按 AT 处理 */
				if (byte == '\n' && line_len > 0)
				{
					line[line_len] = '\0';
					xQueueSend(atQueue, line, 0);
					line_len = 0;
				}
				else if (byte != '\r')
				{
					if (line_len < AT_LINE_MAX - 1)
						line[line_len++] = byte;
				}
				parser.state = 0;
			}
			break;

		/* ===== 状态 2：读总包数 ===== */
		case 2: /* ST_TOTAL */
			parser.total_packets = byte;
			parser.state = 3; /* ST_CURRENT */
			break;

		/* ===== 状态 3：读当前包号 ===== */
		case 3: /* ST_CURRENT */
			parser.current_packet = byte;
			parser.data_len = 0;
			parser.len_byte_idx = 0;
			parser.state = 4; /* ST_LEN */
			break;

		/* ===== 状态 4：读实际长度（2 字节，小端） ===== */
		case 4: /* ST_LEN */
			parser.data_len |= (uint16_t)((uint16_t)byte << (parser.len_byte_idx * 8));
			parser.len_byte_idx++;
			if (parser.len_byte_idx >= 2)
			{
				if (parser.data_len == 0 || parser.data_len > DATA_FRAME_LEN)
				{
					/* 非法长度，回起点，释放 slot */
					frame_pool_free(parser.slot);
					parser.slot = NULL;
					parser.state = 0;
				}
				else
				{
					parser.crc_byte_idx = 0;
					parser.state = 5; /* ST_CRC */
				}
			}
			break;

		/* ===== 状态 5：读 4 字节 CRC32（小端） ===== */
		case 5: /* ST_CRC */
			parser.crc32 |= ((uint32_t)byte) << (parser.crc_byte_idx * 8);
			parser.crc_byte_idx++;
			if (parser.crc_byte_idx >= 4)
			{
				parser.data_idx = 0;
				parser.state = 6; /* ST_DATA */
			}
			break;

		/* ===== 状态 6：读数据（固定 DATA_FRAME_LEN 字节） ===== */
		case 6: /* ST_DATA */
			parser.slot->buf[parser.data_idx++] = byte;
			if (parser.data_idx >= DATA_FRAME_LEN)
			{
				parser.state = 7; /* ST_TAIL_1 */
			}
			break;

		/* ===== 状态 7：等 0x33 ===== */
		case 7: /* ST_TAIL_1 */
			if (byte == FRAME_TAIL_1)
			{
				parser.state = 8; /* ST_TAIL_2 */
			}
			else
			{
				frame_pool_free(parser.slot);
				parser.slot = NULL;
				parser.state = 0;
			}
			break;

		/* ===== 状态 8：等 0x22，帧完成 ===== */
		case 8: /* ST_TAIL_2 */
			if (byte == FRAME_TAIL_2)
			{
				/* 帧完成，校验 CRC */
				parser.crc_calc = calculate_crc32_ex(
					parser.total_packets,
					parser.current_packet,
					parser.data_len,
					parser.slot->buf);

				if (parser.crc_calc == parser.crc32)
				{
					/* CRC 通过，交给 Flash 写入 */
					fw_frame_t frame;
					frame.total_packets = parser.total_packets;
					frame.current_packet = parser.current_packet;
					frame.data_len = parser.data_len;
					frame.crc32 = parser.crc32;
					frame.slot = parser.slot;

					if (xQueueSend(xFrameQueue, &frame, 0) != pdTRUE)
					{
						/* 队列满，归还 slot */
						frame_pool_free(parser.slot);
					}
					/* 注意：slot 在这里不 free，由 Flash 任务 free */
				}
				else
				{
					/* CRC 失败，归还 slot */
					frame_pool_free(parser.slot);
				}
				parser.slot = NULL;
				parser.state = 0;
				line_len = 0;
			}
			else
			{
				frame_pool_free(parser.slot);
				parser.slot = NULL;
				parser.state = 0;
			}
			break;

		default:
			parser.state = 0;
			break;
		}
	}
	/* USER CODE END wifi_getTask */
}

/* USER CODE BEGIN Header_ota_flashTask */
/**
 * @brief Function implementing the ota_flash thread.
 * @param argument: Not used
 * @retval None
 */
/* ==================== Flash 写任务 ==================== */
/* USER CODE END Header_ota_flashTask */
void ota_flashTask(void const *argument)
{
	/* USER CODE BEGIN ota_flashTask */
	/* Infinite loop */

	fw_frame_t frame;
	uint32_t write_addr;
	uint32_t received_count = 0; /* 已成功写入的数据帧数，用于收齐校验 */
	int32_t erased_page = -1;	 /* 已擦除的 OTA 区页号；-1 表示本次尚未擦除 */

	for (;;)
	{

		static const char msg[] = "here is ota_flashTask!\r\n";
		uart_send((const uint8_t *)msg, sizeof(msg) - 1);
		osDelay(1000);

		if (xQueueReceive(xFrameQueue, &frame, portMAX_DELAY) != pdTRUE)
			continue;

		/* 判断是否是结束帧 */
		if (frame.current_packet == PACKET_END)
		{
			uint32_t fw_size = 0;
			uint32_t fw_crc = 0;

			/* 结束帧负载：data[0:4]=固件总长(小端), data[4:8]=整包 CRC32(小端) */
			if (frame.data_len >= 8)
			{
				fw_size = (uint32_t)frame.slot->buf[0] | ((uint32_t)frame.slot->buf[1] << 8) | ((uint32_t)frame.slot->buf[2] << 16) | ((uint32_t)frame.slot->buf[3] << 24);
				fw_crc = (uint32_t)frame.slot->buf[4] | ((uint32_t)frame.slot->buf[5] << 8) | ((uint32_t)frame.slot->buf[6] << 16) | ((uint32_t)frame.slot->buf[7] << 24);
			}
			frame_pool_free(frame.slot);

			/* 收齐校验：已收数据帧数必须等于总包数，否则丢弃本次升级 */
			if (fw_size != 0 && received_count == frame.total_packets)
			{
				g_firmware_size = fw_size;
				g_firmware_crc = fw_crc;
				xTaskNotify(xrequest_upgrade_taskHandle, 0x01, eSetValueWithOverwrite);
			}
			received_count = 0;
			erased_page = -1; /* 本次传输结束，下次重新擦除 */
			continue;		  /* 结束帧不写 Flash，直接继续 */
		}

		/* 新一轮传输开始，清零计数并强制重新擦除 */
		if (frame.current_packet == 0)
		{
			received_count = 0;
			erased_page = -1;
		}

		write_addr = OTA_BUFFER_BASE + ((uint32_t)frame.current_packet * DATA_FRAME_LEN);

		/* 边界保护：不能写到 config 区及以后 */
		if (write_addr + DATA_FRAME_LEN > CONFIG_BASE)
		{
			frame_pool_free(frame.slot);
			continue;
		}

		/* Flash 只能把 1 写成 0，写入前必须先擦除所在页（按页惰性擦除，每 4 帧一次） */
		{
			int32_t page = (int32_t)((write_addr - OTA_BUFFER_BASE) / FLASH_PAGE_SIZE);
			if (page != erased_page)
			{
				if (flash_erase(OTA_BUFFER_BASE + (uint32_t)page * FLASH_PAGE_SIZE,
								FLASH_PAGE_SIZE) != Status_OK)
				{
					frame_pool_free(frame.slot);
					continue;
				}
				erased_page = page;
			}
		}

		/* 写 Flash，只写实际长度，不写 padding */
		if (flash_write(write_addr, frame.slot->buf, frame.data_len) == Status_OK)
			received_count++; /* 写入成功，计入已收帧数 */
		else
		{
			/* 写失败，可以记录错误，忽略 */
		}

		/* 归还 slot */
		frame_pool_free(frame.slot);
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
void request_upgradeTask(void const *argument)
{
	/* USER CODE BEGIN request_upgradeTask */
	/* Infinite loop */

	(void)argument;

	for (;;)
	{

		static const char msg[] = "here is request_upgradeTask!\r\n";
		uart_send((const uint8_t *)msg, sizeof(msg) - 1);
		osDelay(1000);
		
		/* 等待 task_ota_flash 发来的升级通知 */
		xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);

		if (xWifisendTaskHandle != NULL)
			xTaskNotify(xWifisendTaskHandle, 0x02, eSetValueWithOverwrite);
		ota_config_t config;
		config.magic = OTA_MAGIC;
		config.update_flag = 1;
		config.version = CURRENT_VERSION;
		config.firmware_size = g_firmware_size;
		config.crc32 = g_firmware_crc;

		__disable_irq(); // 关闭全局中断，防止读写过程被打断

		// 擦除配置区所在页，写入 config
		flash_erase(CONFIG_BASE, sizeof(ota_config_t));
		flash_write(CONFIG_BASE, (uint8_t *)&config, sizeof(ota_config_t));

		// 软复位
		NVIC_SystemReset();
	}
	/* USER CODE END request_upgradeTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
