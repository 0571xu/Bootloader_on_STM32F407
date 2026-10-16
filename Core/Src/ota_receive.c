/**************************************
 * 文件名  ：ota_receive.c
 * 描述    ：OTA 帧接收、CRC 校验、Flash 写入
 * 说明    ：配合 uart.c 中串口接收的功能实现
 *********************************************************/
 
#include "main.h"
#include "ota_receive.h"
#include <string.h>

/* ==================== 全局变量 ==================== */

StreamBufferHandle_t uartStreamBuffer = NULL;
QueueHandle_t        xFrameQueue      = NULL;
QueueHandle_t        atQueue          = NULL;
TaskHandle_t xrequest_upgrade_taskHandle;
TaskHandle_t xWifisendTaskHandle      = NULL;

/* 帧池 */
static frame_slot_t frame_pool[FRAME_POOL_SIZE];

/* ==================== 帧池管理 ==================== */

frame_slot_t* frame_pool_alloc(void)
{
    for (int i = 0; i < FRAME_POOL_SIZE; i++) {
        if (!frame_pool[i].in_use) {
            frame_pool[i].in_use = 1;
            return &frame_pool[i];
        }
    }
    return NULL;   /* 无空闲 slot */
}

void frame_pool_free(frame_slot_t *slot)
{
    if (slot) {
        slot->in_use = 0;
    }
}

/* ==================== 初始化 ==================== */

void ota_receive_init(void)
{
    memset(frame_pool, 0, sizeof(frame_pool));

    uartStreamBuffer = xStreamBufferCreate(STREAM_BUF_SIZE, 1);   /* 触发水位=1 */
    xFrameQueue      = xQueueCreate(FRAME_QUEUE_DEPTH, sizeof(fw_frame_t));
    atQueue          = xQueueCreate(4, AT_LINE_MAX);

    configASSERT(uartStreamBuffer != NULL);
    configASSERT(xFrameQueue != NULL);
    configASSERT(atQueue != NULL);
}

/* ==================== CRC32 计算（含帧头字段） ==================== */

uint32_t calculate_crc32_ex(uint8_t total, uint8_t current, uint16_t len, uint8_t *data)
{
    uint32_t i;
    uint32_t words;
    uint32_t rest;
    uint32_t *p;
    uint8_t tmp[4];

    __HAL_RCC_CRC_CLK_ENABLE();   /* 使能 CRC 时钟（AHB1） */
    CRC->CR = CRC_CR_RESET;       /* 复位 CRC 数据寄存器 */

    /* 先算 4 字节头：total(1B) + current(1B) + len(2B,小端)，刚好拼成 1 个 32 位字 */
    uint32_t header = ((uint32_t)total)
                    | ((uint32_t)current << 8)
                    | ((uint32_t)len << 16);
    CRC->DR = header;

    /* 再算实际数据，只算 len 字节 */
    p     = (uint32_t *)data;
    words = len / 4;
    rest  = len % 4;

    for (i = 0; i < words; i++) {
        CRC->DR = p[i];
    }

    /* 剩余 1~3 字节，用 0xFF 补齐到 4 字节 */
    if (rest > 0) {
        tmp[0] = 0xFF;
        tmp[1] = 0xFF;
        tmp[2] = 0xFF;
        tmp[3] = 0xFF;
        for (i = 0; i < rest; i++) {
            tmp[i] = data[words * 4 + i];
        }
        CRC->DR = *(uint32_t *)tmp;
    }

    return CRC->DR;
}




// Flash 擦除
StatusTypeDef flash_erase(uint32_t start_addr, uint32_t size)
{
    FLASH_EraseInitTypeDef erase_init;
    uint32_t page_error = 0;
    uint32_t page_addr  = start_addr & ~(FLASH_PAGE_SIZE - 1);
    uint32_t end_addr   = start_addr + size;

    if (HAL_FLASH_Unlock() != HAL_OK)
        return Status_ERROR;

    while (page_addr < end_addr) {
        /* STM32F407 前 4 个扇区(0~3)每扇区 16KB，OTA 区与 config 区都在这一段 */
        erase_init.TypeErase    = FLASH_TYPEERASE_SECTORS;
        erase_init.VoltageRange = FLASH_VOLTAGE_RANGE_3;
        erase_init.Sector       = (page_addr - FLASH_BASE) / 0x4000U;
        erase_init.NbSectors    = 1;

        if (HAL_FLASHEx_Erase(&erase_init, &page_error) != HAL_OK) {
            HAL_FLASH_Lock();
            return Status_ERROR;
        }
        page_addr += FLASH_PAGE_SIZE;
    }

    HAL_FLASH_Lock();
    return Status_OK;
}

// Flash 写入
StatusTypeDef flash_write(uint32_t dest_addr, uint8_t *src, uint32_t size)
{
    uint32_t i;
    uint32_t word;

    if (HAL_FLASH_Unlock() != HAL_OK)
        return Status_ERROR;

    for (i = 0; i < size; i += 4) {
        // 最后一个不足 4 字节时用 0xFF 补齐（Flash 擦除后默认值）
        uint8_t b0 = (i + 0 < size) ? src[i + 0] : 0xFF;
        uint8_t b1 = (i + 1 < size) ? src[i + 1] : 0xFF;
        uint8_t b2 = (i + 2 < size) ? src[i + 2] : 0xFF;
        uint8_t b3 = (i + 3 < size) ? src[i + 3] : 0xFF;

        word = (uint32_t)b0
             | ((uint32_t)b1 << 8)
             | ((uint32_t)b2 << 16)
             | ((uint32_t)b3 << 24);

        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, dest_addr + i, word) != HAL_OK) {
            HAL_FLASH_Lock();
            return Status_ERROR;
        }
    }

    HAL_FLASH_Lock();
    return Status_OK;
}


