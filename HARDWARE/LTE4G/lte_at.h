#ifndef __LTE_AT_H__
#define __LTE_AT_H__

#include "main.h"
#include "lte_def.h"
#include <stdint.h>

typedef struct
{
    const char *name;
    uint32_t timeout;
} at_config;

typedef struct
{
    int32_t (*init)(at_config *config);
    int32_t (*cmd)(int8_t *cmd, int32_t len, const char *suffix, char *resp_buf, int *resp_len);
    int32_t (*sendBuf)(int8_t *buf, int32_t len, const char *suffix, char *resp_buf, int *resp_len);
    int32_t (*deinit)(void);
} at_task;

extern at_task at;

/* 等待某条 URC 出现（比如 conn/suback） */
int LTEAT_wait_urc(const char *needle, uint32_t timeout_ms);

/* 把接收缓冲里的内容打印一下（可选） */
void LTEAT_drain_print_once(void);

/* UART4 IDLE 收到一帧后会回调这个（用于兼容你 stm32f1xx_it.c 的调用） */
void LTE_UART4_OnRxChunk(const uint8_t *data, uint16_t len);

#endif
