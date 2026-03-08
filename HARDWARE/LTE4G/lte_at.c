#include "lte_at.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>

/* 由 usart.c 提供 */
extern uint8_t  U4_RxBuf[];
extern volatile uint16_t U4_RxLen;
extern volatile uint8_t  U4_RxFlag;
extern UART_HandleTypeDef huart5;

/* ==================== 可调参数 ==================== */
#define LTEAT_ACCUM_SIZE   2048      // 累积缓冲大小（建议 >= 2*U4_RXMAX_SIZE）
#define LTEAT_PRINT_CHUNK  1         // 1=打印每个chunk，0=不打印
/* ================================================== */
#define AT_ACC_MAX 2048
static char at_acc_buf[AT_ACC_MAX];
static uint16_t at_acc_len;
static uint32_t s_at_timeout_ms = 5000;

static uint8_t accum[LTEAT_ACCUM_SIZE];
static uint16_t accum_len = 0;

static void accum_reset(void)
{
    accum_len = 0;
    memset(accum, 0, sizeof(accum));
}

static void accum_append(const uint8_t *data, uint16_t len)
{
    if (len == 0) return;

    /* 如果要溢出：保留尾部（简单策略），避免死 */
    if (accum_len + len >= LTEAT_ACCUM_SIZE)
    {
        uint16_t keep = LTEAT_ACCUM_SIZE / 2;
        if (keep > accum_len) keep = accum_len;

        memmove(accum, &accum[accum_len - keep], keep);
        accum_len = keep;
    }

    memcpy(&accum[accum_len], data, len);
    accum_len += len;
    accum[accum_len] = 0;
}

static int accum_has(const char *needle)
{
    if (!needle) return 0;
    if (accum_len == 0) return 0;
    return (strstr((char*)accum, needle) != NULL);
}

static int accum_has_error(void)
{
    if (accum_len == 0) return 0;
    if (strstr((char*)accum, "ERROR")) return 1;
    if (strstr((char*)accum, "+CME ERROR")) return 1;
    return 0;
}

static int32_t at_init(at_config *config)
{
    if (config && config->timeout >= 1000)
    {
        s_at_timeout_ms = config->timeout;
    }
    return 0;
}

/**
 * @brief 发送AT命令并等待ack（支持多段响应累积）
 * @retval 0=成功，1=超时，2=错误
 */
static uint8_t LTEAT_sendCmd(const char *cmd,
                             const char *ack,
                             uint32_t wait_ms,
                             uint8_t *out)
{
    char txbuf[512];
    int txlen;

    /* 清空累积缓冲 */
    at_acc_len = 0;
    memset(at_acc_buf, 0, sizeof(at_acc_buf));

    /* 清 UART4 接收标志 */
    U4_RxFlag = 0;
    U4_RxLen  = 0;

    txlen = snprintf(txbuf, sizeof(txbuf), "%s\r\n", cmd);
    if (txlen <= 0 || txlen >= (int)sizeof(txbuf))
    {
        printf("[LTEAT] TX overflow\r\n");
        return 2;
    }

    HAL_UART_Transmit(&huart5, (uint8_t*)txbuf, txlen, 0xFFFF);

    if (ack == NULL) return 0;

    uint32_t start = HAL_GetTick();

    while ((HAL_GetTick() - start) < wait_ms)
    {
        if (U4_RxFlag)
        {
            U4_RxFlag = 0;

            /* 防止越界 */
            if (at_acc_len + U4_RxLen < AT_ACC_MAX - 1)
            {
                memcpy(at_acc_buf + at_acc_len, U4_RxBuf, U4_RxLen);
                at_acc_len += U4_RxLen;
                at_acc_buf[at_acc_len] = 0;
            }

            /* 打印完整累积内容（调试非常有用） */
            // printf("[AT-ACC]\r\n%s\r\n", at_acc_buf);

            /* 先判 ERROR */
            if (strstr(at_acc_buf, "ERROR") ||
                strstr(at_acc_buf, "+CME ERROR"))
            {
                printf("[LTEAT] CMD ERROR=%s\r\n", cmd);
                printf("[LTEAT] RX=%s\r\n", at_acc_buf);
                return 2;
            }

            /* 再判 ACK */
            if (strstr(at_acc_buf, ack))
            {
                if (out)
                {
                    strncpy((char*)out, at_acc_buf, AT_DATA_LEN - 1);
                    out[AT_DATA_LEN - 1] = 0;
                }
                return 0;
            }
        }
    }

    printf("[LTEAT] TIMEOUT CMD=%s\r\n", cmd);
    printf("[LTEAT] LAST RX=%s\r\n", at_acc_buf);
    return 1;
}


static int32_t at_cmd(int8_t *cmd, int32_t len, const char *suffix, char *resp_buf, int *resp_len)
{
    (void)len;
    (void)resp_len;
    return (int32_t)LTEAT_sendCmd((const char*)cmd, suffix, s_at_timeout_ms, (uint8_t*)resp_buf);
}

static int32_t at_buf(int8_t *buf, int32_t len, const char *suffix, char *resp_buf, int *resp_len)
{
    (void)len;
    (void)resp_len;
    return (int32_t)LTEAT_sendCmd((const char*)buf, suffix, s_at_timeout_ms, (uint8_t*)resp_buf);
}

static int32_t at_deinit(void)
{
    return 0;
}

at_task at = {
    .init    = at_init,
    .cmd     = at_cmd,
    .sendBuf = at_buf,
    .deinit  = at_deinit
};

/**
 * @brief 等待URC出现（同样使用累积策略）
 */
int LTEAT_wait_urc(const char *needle, uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();
    accum_reset();

    while ((HAL_GetTick() - start) < timeout_ms)
    {
        if (U4_RxFlag)
        {
            U4_RxFlag = 0;

            if (U4_RxLen >= U4_RXMAX_SIZE) U4_RxLen = U4_RXMAX_SIZE - 1;
            ((char*)U4_RxBuf)[U4_RxLen] = 0;

#if LTEAT_PRINT_CHUNK
            printf("[URC-CHUNK] %s\r\n", U4_RxBuf);
#endif

            accum_append(U4_RxBuf, U4_RxLen);

            if (strstr((char*)accum, needle))
                return 0;
        }
    }
    return -1;
}

/* 把接收缓冲里的内容打印一下（可选） */
void LTEAT_drain_print_once(void)
{
    if (U4_RxFlag)
    {
        U4_RxFlag = 0;
        if (U4_RxLen >= U4_RXMAX_SIZE) U4_RxLen = U4_RXMAX_SIZE - 1;
        ((char*)U4_RxBuf)[U4_RxLen] = 0;
        printf("[URC] %s\r\n", U4_RxBuf);
    }
}

/* 兼容 stm32f1xx_it.c 里的调用：最小版不需要做任何事 */
void LTE_UART4_OnRxChunk(const uint8_t *data, uint16_t len)
{
    (void)data;
    (void)len;
}



